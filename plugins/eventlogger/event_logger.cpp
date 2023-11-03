/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "event_logger.h"

#include "securec.h"

#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <mutex>

#include "parameter.h"

#include "common_utils.h"
#include "event_source.h"
#include "file_util.h"
#include "parameter_ex.h"
#include "plugin_factory.h"
#include "string_util.h"
#include "sys_event.h"
#include "sys_event_dao.h"
#include "time_util.h"

#include "event_log_task.h"
#include "event_logger_config.h"
#include "freeze_common.h"
#include "utility/trace_collector.h"
namespace OHOS {
namespace HiviewDFX {
REGISTER(EventLogger);
DEFINE_LOG_LABEL(0xD002D01, "EventLogger");
bool EventLogger::IsInterestedPipelineEvent(std::shared_ptr<Event> event)
{
    if (event == nullptr) {
        return false;
    }
    if (event->eventId_ > EVENT_MAX_ID) {
        return false;
    }

    auto sysEvent = Event::DownCastTo<SysEvent>(event);
    if (eventLoggerConfig_.find(sysEvent->eventName_) == eventLoggerConfig_.end()) {
        return false;
    }
    HIVIEW_LOGD("event time:%{public}llu jsonExtraInfo is %{public}s", TimeUtil::GetMilliseconds(),
        sysEvent->AsJsonStr().c_str());

    long pid = sysEvent->GetEventIntValue("PID");
    pid = pid ? pid : sysEvent->GetPid();

    if (!CommonUtils::IsTheProcessExist(pid)) {
        HIVIEW_LOGW("event: eventName:%{public}s, pid:%{public}d is invalid, errno:%{public}d:%{public}s",
            sysEvent->eventName_.c_str(), pid, errno, strerror(errno));
        return false;
    }

    EventLoggerConfig::EventLoggerConfigData& configOut = eventLoggerConfig_[sysEvent->eventName_];
    sysEvent->eventName_ = configOut.name;
    sysEvent->SetValue("eventLog_action", configOut.action);
    sysEvent->SetValue("eventLog_interval", configOut.interval);
    return true;
}

bool EventLogger::OnEvent(std::shared_ptr<Event> &onEvent)
{
    if (onEvent == nullptr) {
        HIVIEW_LOGE("event == nullptr");
        return false;
    }

    auto sysEvent = Event::DownCastTo<SysEvent>(onEvent);
    if (!IsHandleAppfreeze(sysEvent)) {
        return true;
    }

    if (sysEvent->GetValue("eventLog_action").empty()) {
        UpdateDB(sysEvent, "nolog");
        return true;
    }

    sysEvent->OnPending();
    sysEvent->SetEventValue("Finish", "0");

    std::unique_lock<std::mutex> lck(finishMutex_);
    sysEventSet_.insert(sysEvent);
    if (sysEventSet_.size() == 1) {
        constexpr int waitTime = 5;
        auto CheckFinishFun = std::bind(&EventLogger::CheckEventOnContinue, this);
        threadLoop_->AddTimerEvent(nullptr, nullptr, CheckFinishFun, waitTime, false);
    }

    auto task = [this, sysEvent]() {
        HIVIEW_LOGD("event time:%{public}llu jsonExtraInfo is %{public}s", TimeUtil::GetMilliseconds(),
            sysEvent->AsJsonStr().c_str());

        long pid = sysEvent->GetEventIntValue("PID");
        pid = pid ? pid : sysEvent->GetPid();
        if (!CommonUtils::IsTheProcessExist(pid)) {
            HIVIEW_LOGW("event: eventName:%{public}s, pid:%{public}d is invalid, errno:%{public}d:%{public}s",
                sysEvent->eventName_.c_str(), pid, errno, strerror(errno));
            return;
        }
        if (!JudgmentRateLimiting(sysEvent)) {
            return;
        }
        this->StartLogCollect(sysEvent);
    };
    eventPool_->AddTask(task, "eventlogger");
    return true;
}

int EventLogger::Getfile(std::shared_ptr<SysEvent> event, std::string& logFile)
{
    std::string idStr = event->eventName_.empty() ? std::to_string(event->eventId_) : event->eventName_;
    uint64_t logTime = event->happenTime_ / TimeUtil::SEC_TO_MILLISEC;
    int32_t pid = static_cast<int32_t>(event->GetEventIntValue("PID"));
    pid = pid ? pid : event->GetPid();
    logFile = idStr + "-" + std::to_string(pid) + "-"
                          + TimeUtil::TimestampFormatToDate(logTime, "%Y%m%d%H%M%S") + ".log";
    if (FileUtil::FileExists(LOGGER_EVENT_LOG_PATH + "/" + logFile)) {
        HIVIEW_LOGW("filename: %{public}s is existed, direct use.", logFile.c_str());
        UpdateDB(event, logFile);
        return -1;
    }

    return logStore_->CreateLogFile(logFile);
}

void EventLogger::StartLogCollect(std::shared_ptr<SysEvent> event)
{
    std::string logFile;
    int fd = Getfile(event, logFile);
    if (fd < 0) {
        HIVIEW_LOGE("create log file %{public}s failed, %{public}d", logFile.c_str(), fd);
        return;
    }

    std::unique_ptr<EventLogTask> logTask = std::make_unique<EventLogTask>(fd, event);
    std::string cmdStr = event->GetValue("eventLog_action");
    std::vector<std::string> cmdList;
    StringUtil::SplitStr(cmdStr, ",", cmdList);
    for (const std::string& cmd : cmdList) {
        logTask->AddLog(cmd);
    }

    const uint32_t placeholder = 3;
    auto start = TimeUtil::GetMilliseconds();
    uint64_t startTime = start / TimeUtil::SEC_TO_MILLISEC;
    std::ostringstream startTimeStr;
    startTimeStr << "start time: " << TimeUtil::TimestampFormatToDate(startTime, "%Y/%m/%d-%H:%M:%S");
    startTimeStr << ":" << std::setw(placeholder) << std::setfill('0') <<
        std::to_string(start % TimeUtil::SEC_TO_MILLISEC);
    startTimeStr << std::endl;
    FileUtil::SaveStringToFd(fd, startTimeStr.str());
    WriteCommonHead(fd, event);
    auto ret = logTask->StartCompose();
    if (ret != EventLogTask::TASK_SUCCESS) {
        HIVIEW_LOGE("capture fail %{public}d", ret);
    }
    auto end = TimeUtil::GetMilliseconds();
    std::string totalTime = "\n\nCatcher log total time is " + std::to_string(end - start) + "ms\n";
    FileUtil::SaveStringToFd(fd, totalTime);
    close(fd);
    UpdateDB(event, logFile);
    event->SetEventValue("Finish", "1");
    HIVIEW_LOGI("Collect on finish, name: %{public}s", logFile.c_str());
}

bool EventLogger::WriteCommonHead(int fd, std::shared_ptr<SysEvent> event)
{
    std::ostringstream headerStream;

    headerStream << "DOMAIN = " << event->domain_ << std::endl;
    headerStream << "EVENTNAME = " << event->eventName_ << std::endl;
    uint64_t logTime = event->happenTime_ / TimeUtil::SEC_TO_MILLISEC;
    uint64_t logTimeMs = event->happenTime_ % TimeUtil::SEC_TO_MILLISEC;
    std::string happenTime = TimeUtil::TimestampFormatToDate(logTime, "%Y/%m/%d-%H:%M:%S");
    headerStream << "TIMESTAMP = " << happenTime << ":" << logTimeMs << std::endl;
    long pid = event->GetEventIntValue("PID");
    pid = pid ? pid : event->GetPid();
    headerStream << "PID = " << pid << std::endl;
    long uid = event->GetEventIntValue("UID");
    uid = uid ? uid : event->GetUid();
    headerStream << "UID = " << uid << std::endl;
    long tid = event->GetTid();
    headerStream << "TID = " << tid << std::endl;
    if (event->GetEventValue("MODULE_NAME") != "") {
        headerStream << "MODULE_NAME = " << event->GetEventValue("MODULE_NAME") << std::endl;
    } else {
        headerStream << "PACKAGE_NAME = " << event->GetEventValue("PACKAGE_NAME") << std::endl;
    }
    headerStream << "PROCESS_NAME = " << event->GetEventValue("PROCESS_NAME") << std::endl;
    headerStream << "eventLog_action = " << event->GetValue("eventLog_action") << std::endl;
    headerStream << "eventLog_interval = " << event->GetValue("eventLog_interval") << std::endl;
    std::string msg = StringUtil::ReplaceStr(event->GetEventValue("MSG"), "\\n", "\n");
    headerStream << "MSG = " << msg << std::endl;

    std::string stack = event->GetEventValue("STACK");
    if (!stack.empty()) {
        headerStream << StringUtil::UnescapeJsonStringValue(stack) << std::endl;
    }

    FileUtil::SaveStringToFd(fd, headerStream.str());
    return true;
}

bool EventLogger::JudgmentRateLimiting(std::shared_ptr<SysEvent> event)
{
    int32_t interval = event->GetIntValue("eventLog_interval");
    if (interval == 0) {
        return true;
    }

    int64_t pid = event->GetEventIntValue("PID");
    pid = pid ? pid : event->GetPid();
    std::string eventName = event->eventName_;
    std::string eventPid = std::to_string(pid);

    std::unique_lock<std::mutex> lck(intervalMutex_);
    std::time_t now = std::time(0);
    for (auto it = eventTagTime_.begin(); it != eventTagTime_.end();) {
        if (it->first.find(eventName) != it->first.npos) {
            if ((now - it->second) >= interval) {
                it = eventTagTime_.erase(it);
                continue;
            }
        }
        ++it;
    }

    std::string tagTimeName = eventName + eventPid;
    auto it = eventTagTime_.find(tagTimeName);
    if (it != eventTagTime_.end()) {
        if ((now - it->second) < interval) {
            HIVIEW_LOGE("event: id:0x%{public}d, eventName:%{public}s pid:%{public}s. \
                interval:%{public}ld There's not enough interval",
                event->eventId_, eventName.c_str(), eventPid.c_str(), interval);
            return false;
        }
    }
    eventTagTime_[tagTimeName] = now;
    HIVIEW_LOGI("event: id:0x%{public}d, eventName:%{public}s pid:%{public}s. \
        interval:%{public}ld normal interval",
        event->eventId_, eventName.c_str(), eventPid.c_str(), interval);
    return true;
}

bool EventLogger::UpdateDB(std::shared_ptr<SysEvent> event, std::string logFile)
{
    if (logFile == "nolog") {
        HIVIEW_LOGI("set info_ with nolog into db.");
        event->SetEventValue(EventStore::EventCol::INFO, "nolog", false);
    } else {
        auto logPath = R"~(logPath:)~" + LOGGER_EVENT_LOG_PATH  + "/" + logFile;
        event->SetEventValue(EventStore::EventCol::INFO, logPath, true);
    }
    return true;
}

bool EventLogger::IsHandleAppfreeze(std::shared_ptr<SysEvent> event)
{
    std::string bundleName = event->GetEventValue("PACKAGE_NAME");
    if (bundleName.empty()) {
        bundleName = event->GetEventValue("MODULE_NAME");
    }
    if (bundleName.empty()) {
        return true;
    }

    const int buffSize = 128;
    char paramOutBuff[buffSize] = {0};
    GetParameter("hiviewdfx.appfreeze.filter_bundle_name", "", paramOutBuff, buffSize - 1);

    std::string str(paramOutBuff);
    if (str.find(bundleName) != std::string::npos) {
        HIVIEW_LOGW("appfreeze filtration %{public}s.", bundleName.c_str());
        return false;
    }
    return true;
}

void EventLogger::CheckEventOnContinue()
{
    HIVIEW_LOGI("Check Event can Continue");
    std::unique_lock<std::mutex> lck(finishMutex_);
    for (auto eventIter = sysEventSet_.begin(); eventIter != sysEventSet_.end();) {
        std::shared_ptr<SysEvent> event = *eventIter;
        if (event->GetEventValue("Finish") == "1") {
            event->ResetPendingStatus();
            event->OnContinue();
            eventIter = sysEventSet_.erase(eventIter);
            HIVIEW_LOGI("event onContinue");
        } else {
            ++eventIter;
        }
    }
    HIVIEW_LOGI("event size:%{public}d", sysEventSet_.size());
    if (sysEventSet_.size() > 0) {
        constexpr int waitTime = 5;
        auto CheckFinishFun = std::bind(&EventLogger::CheckEventOnContinue, this);
        threadLoop_->AddTimerEvent(nullptr, nullptr, CheckFinishFun, waitTime, false);
    }
}

void EventLogger::OnLoad()
{
    HIVIEW_LOGI("EventLogger OnLoad.");
    SetName("EventLogger");
    SetVersion("1.0");
    logStore_->SetMaxSize(MAX_FOLDER_SIZE);
    logStore_->SetMinKeepingFileNumber(MAX_FILE_NUM);
    logStore_->Init();
    threadLoop_ = GetWorkLoop();
    threadLoop_->AddFileDescriptorEventCallback("EventLoggerFd",
        std::static_pointer_cast<EventLogger>(shared_from_this()));

    EventLoggerConfig logConfig;
    eventLoggerConfig_ = logConfig.GetConfig();

    eventPool_ = std::make_shared<EventThreadPool>(maxEventPoolCount, "EventLog");
    eventPool_->Start();

    activeKeyEvent_ = std::make_unique<ActiveKeyEvent>();
    activeKeyEvent_ ->Init(eventPool_, logStore_);
    FreezeCommon freezeCommon;
    if (!freezeCommon.Init()) {
        HIVIEW_LOGE("FreezeCommon filed.");
        return;
    }

    std::set<std::string> freezeeventNames = freezeCommon.GetPrincipalStringIds();
    std::unordered_set<std::string> eventNames;
    for (auto& i : freezeeventNames) {
        eventNames.insert(i);
    }
    auto context = GetHiviewContext();
    if (context != nullptr) {
        auto plugin = context->GetPluginByName("FreezeDetectorPlugin");
        HIVIEW_LOGE("plugin plugin %{public}s.", plugin->GetName().c_str());
        context->AddDispatchInfo(plugin, {}, eventNames, {}, {});
    }

    GetCmdlineContent();
    GetRebootReasonConfig();

    auto ptr = std::static_pointer_cast<EventLogger>(shared_from_this());
    context->RegisterUnorderedEventListener(ptr);
    AddListenerInfo(Event::MessageType::PLUGIN_MAINTENANCE);
}

void EventLogger::OnUnload()
{
    HIVIEW_LOGD("called");
    eventPool_->Stop();
}

void EventLogger::CreateAndPublishEvent(std::string& dirPath, std::string& fileName)
{
    uint8_t count = 0;
    for (auto& i : MONITOR_STACK_FLIE_NAME) {
        if (fileName.find(i) != fileName.npos) {
            ++count;
            break;
        }
    }

    if (count == 0) {
        return;
    }
    std::string logPath = dirPath + "/" + fileName;
    if (!FileUtil::FileExists(logPath)) {
        HIVIEW_LOGW("file %{public}s not exist. exit!", logPath.c_str());
        return;
    }
    std::vector<std::string> values;
    StringUtil::SplitStr(fileName, "-", values, false, false);
    if (values.size() != 3) { // 3: type-pid-timestamp
        HIVIEW_LOGE("failed to split stack file name %{public}s.", fileName.c_str());
        return;
    }
    int32_t pid = 0;
    StringUtil::ConvertStringTo<int32_t>(values[1], pid);  // 1: pid

    auto jsonStr = "{\"domain_\":\"RELIABILITY\"}";
    auto sysEvent = std::make_shared<SysEvent>("EventLogger", nullptr, jsonStr);
    sysEvent->SetEventValue("name_", "STACK");
    sysEvent->SetEventValue("type_", 1);
    sysEvent->SetEventValue("time_", TimeUtil::GetMilliseconds());
    sysEvent->SetEventValue("pid_", getpid());
    sysEvent->SetEventValue("tid_", gettid());
    sysEvent->SetEventValue("uid_", getuid());
    sysEvent->SetEventValue("tz_", TimeUtil::GetTimeZone());
    sysEvent->SetEventValue("PID", pid);
    sysEvent->SetEventValue("MSG", "application stack");
    std::string tmpStr = R"~(logPath:)~" + logPath;
    sysEvent->SetEventValue(EventStore::EventCol::INFO, tmpStr);
    auto context = GetHiviewContext();
    if (context != nullptr) {
        auto seq = context->GetPipelineSequenceByName("SysEventPipeline");
        sysEvent->SetPipelineInfo("SysEventPipeline", seq);
        sysEvent->OnContinue();
    }
}

bool EventLogger::OnFileDescriptorEvent(int fd, int type)
{
    HIVIEW_LOGD("fd:%{public}d, type:%{public}d, inotifyFd_:%{public}d.\n", fd, type, inotifyFd_);
    const int bufSize = 2048;
    char buffer[bufSize] = {0};
    char *offset = nullptr;
    struct inotify_event *event = nullptr;
    if (inotifyFd_ < 0) {
        HIVIEW_LOGE("Invalid inotify fd:%{public}d", inotifyFd_);
        return false;
    }
    int len = read(inotifyFd_, buffer, bufSize);
    if (len <= 0) {
        HIVIEW_LOGE("failed to read event");
        return false;
    }

    offset = buffer;
    event = (struct inotify_event *)buffer;
    while ((reinterpret_cast<char *>(event) - buffer + sizeof(struct inotify_event)) <
            static_cast<uintptr_t>(len)) {
        if ((reinterpret_cast<char *>(event) - buffer + sizeof(struct inotify_event) + event->len) >
            static_cast<uintptr_t>(len)) {
            break;
        }

        if (event->len != 0) {
            const auto& it = fileMap_.find(event->wd);
            if (it == fileMap_.end()) {
                continue;
            }

            std::string fileName = std::string(event->name);
            HIVIEW_LOGI("fileName: %{public}s event->mask: 0x%{public}x, event->len: %{public}d",
                fileName.c_str(), event->mask, event->len);
                
            if (it->second != MONITOR_STACK_LOG_PATH) {
                return false;
            }
            CreateAndPublishEvent(it->second, fileName);
        }

        int tmpLen = sizeof(struct inotify_event) + event->len;
        event = (struct inotify_event *)(offset + tmpLen);
        offset += tmpLen;
    }
    return true;
}

int32_t EventLogger::GetPollFd()
{
    HIVIEW_LOGD("call");
    if (inotifyFd_ > 0) {
        return inotifyFd_;
    }

    inotifyFd_ = inotify_init();
    if (inotifyFd_ == -1) {
        HIVIEW_LOGE("failed to init inotify: %s.\n", strerror(errno));
        return -1;
    }

    for (const std::string& i : MONITOR_LOG_PATH) {
        int wd = inotify_add_watch(inotifyFd_, i.c_str(), IN_CLOSE_WRITE | IN_MOVED_TO);
        if (wd < 0) {
            HIVIEW_LOGE("failed to add watch entry : %s(%s).\n", strerror(errno), i.c_str());
            continue;
        }
        fileMap_[wd] = i;
    }

    if (fileMap_.empty()) {
        close(inotifyFd_);
        inotifyFd_ = -1;
    }
    return inotifyFd_;
}

int32_t EventLogger::GetPollType()
{
    return EPOLLIN;
}

std::string EventLogger::GetListenerName()
{
    return "EventLogger";
}

void EventLogger::OnUnorderedEvent(const Event& msg)
{
    if (CanProcessRebootEvent(msg)) {
        auto task = std::bind(&EventLogger::ProcessRebootEvent, this);
        threadLoop_->AddEvent(nullptr, nullptr, task);
    }
}

bool EventLogger::CanProcessRebootEvent(const Event& event)
{
    return (event.messageType_ == Event::MessageType::PLUGIN_MAINTENANCE) &&
        (event.eventId_ == Event::EventId::PLUGIN_LOADED);
}

void EventLogger::ProcessRebootEvent()
{
    if (GetRebootReason() != LONG_PRESS) {
        return;
    }

    auto event = std::make_shared<SysEvent>("EventLogger", nullptr, "");
    event->domain_ = DOMAIN_LONGPRESS;
    event->eventName_ = STRINGID_LONGPRESS;
    event->happenTime_ = TimeUtil::GetMilliseconds();
    event->messageType_ = Event::MessageType::SYS_EVENT;
    event->SetEventValue(EventStore::EventCol::DOMAIN, DOMAIN_LONGPRESS);
    event->SetEventValue(EventStore::EventCol::NAME, STRINGID_LONGPRESS);
    event->SetEventValue(EventStore::EventCol::TYPE, 1);
    event->SetEventValue(EventStore::EventCol::TS, TimeUtil::GetMilliseconds());
    event->SetEventValue(EventStore::EventCol::TZ, TimeUtil::GetTimeZone());
    event->SetEventValue("PID", 0);
    event->SetEventValue("UID", 0);
    event->SetEventValue("PACKAGE_NAME", STRINGID_LONGPRESS);
    event->SetEventValue("PROCESS_NAME", STRINGID_LONGPRESS);
    event->SetEventValue("MSG", STRINGID_LONGPRESS);

    auto context = GetHiviewContext();
    if (event != nullptr && context != nullptr) {
        auto seq = context->GetPipelineSequenceByName("EventloggerPipeline");
        event->SetPipelineInfo("EventloggerPipeline", seq);
        event->OnContinue();
    }
}

std::string EventLogger::GetRebootReason() const
{
    std::string reboot = "";
    std::string reset = "";
    if (GetMatchString(cmdlineContent_, reboot, REBOOT_REASON + PATTERN_WITHOUT_SPACE) &&
        GetMatchString(cmdlineContent_, reset, NORMAL_RESET_TYPE + PATTERN_WITHOUT_SPACE)) {
            for (auto reason : rebootReasons_) {
                if (reason == reboot || reason == reset) {
                    HIVIEW_LOGI("get reboot reason: LONG_PRESS.");
                    return LONG_PRESS;
                }
            }
        }
    return "";
}

void EventLogger::GetCmdlineContent()
{
    if (FileUtil::LoadStringFromFile(cmdlinePath_, cmdlineContent_) == false) {
        HIVIEW_LOGE("failed to read cmdline:%{public}s.", cmdlinePath_.c_str());
    }
}

void EventLogger::GetRebootReasonConfig()
{
    rebootReasons_.clear();
    if (rebootReasons_.size() == 0) {
        rebootReasons_.push_back(AP_S_PRESS6S);
    }
}

bool EventLogger::GetMatchString(const std::string& src, std::string& dst, const std::string& pattern) const
{
    std::regex reg(pattern);
    std::smatch result;
    if (std::regex_search(src, result, reg)) {
        dst = StringUtil::TrimStr(result[1], '\n');
        return true;
    }
    return false;
}
} // namesapce HiviewDFX
} // namespace OHOS
