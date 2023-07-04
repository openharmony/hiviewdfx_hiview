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

#include "common_utils.h"
#include "event_source.h"
#include "file_util.h"
#include "parameter_ex.h"
#include "plugin_factory.h"
#include "string_util.h"
#include "sys_event.h"
#include "sys_event_dao.h"
#include "time_util.h"

#include "event_log_action.h"
#include "event_logger_config.h"
#include "freeze_common.h"
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
        sysEvent->jsonExtraInfo_.c_str());

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
    if (sysEvent->GetValue("eventLog_action").empty()) {
        UpdateDB(sysEvent, "nolog");
        PostEvent(sysEvent);
        return true;
    }

    auto task = [this, sysEvent]() {
        HIVIEW_LOGD("event time:%{public}llu jsonExtraInfo is %{public}s", TimeUtil::GetMilliseconds(),
            sysEvent->jsonExtraInfo_.c_str());

        long pid = sysEvent->GetEventIntValue("PID");
        pid = pid ? pid : sysEvent->GetPid();
        if (!CommonUtils::IsTheProcessExist(pid)) {
            HIVIEW_LOGW("event: eventName:%{public}s, pid:%{public}d is invalid, errno:%{public}d:%{public}s",
                sysEvent->eventName_.c_str(), pid, errno, strerror(errno));
            return;
        }
        this->StartLogCollect(sysEvent);
        this->PostEvent(sysEvent);
    };
    eventPool_->AddTask(task, "eventlogger");
    // finish pipeline
    onEvent->OnFinish();
    return true;
}

void EventLogger::StartLogCollect(std::shared_ptr<SysEvent> event)
{
    if (!JudgmentRateLimiting(event)) {
        return;
    }

    std::string idStr = event->eventName_.empty() ? std::to_string(event->eventId_) : event->eventName_;
    uint64_t logTime = event->happenTime_ / TimeUtil::SEC_TO_MILLISEC;
    int32_t pid = static_cast<int32_t>(event->GetEventIntValue("PID"));
    pid = pid ? pid : event->GetPid();
    std::string logFile = idStr + "-" + std::to_string(pid) + "-"
                          + TimeUtil::TimestampFormatToDate(logTime, "%Y%m%d%H%M%S") + ".log";
    if (FileUtil::FileExists(LOGGER_EVENT_LOG_PATH + "/" + logFile)) {
        HIVIEW_LOGW("filename: %{public}s is existed, direct use.", logFile.c_str());
        UpdateDB(event, logFile);
        return;
    }

    int fd = logStore_->CreateLogFile(logFile);
    if (fd < 0) {
        HIVIEW_LOGE("create log file %{public}s failed, %{public}d", logFile.c_str(), fd);
        return;
    }

    auto start = TimeUtil::GetMilliseconds();
    uint64_t startTime = start / TimeUtil::SEC_TO_MILLISEC;
    std::string startTimeStr = TimeUtil::TimestampFormatToDate(startTime, "%Y/%m/%d-%H:%M:%S");
    startTimeStr += ":" + std::to_string(start % TimeUtil::SEC_TO_MILLISEC);
    FileUtil::SaveStringToFd(fd, "start time: " + startTimeStr + "\n");
    WriteCommonHead(fd, event);
    auto eventLogAction = std::make_unique<EventLogAction>(fd, event);
    eventLogAction->Init();
    eventLogAction->CaptureAction();
    auto end = TimeUtil::GetMilliseconds();
    std::string totalTime = "\n\nCatcher log total time is " + std::to_string(end - start) + "ms\n";
    FileUtil::SaveStringToFd(fd, totalTime);
    close(fd);
    UpdateDB(event, logFile);
}

bool EventLogger::PostEvent(std::shared_ptr<SysEvent> event)
{
    auto eventPtr = std::make_shared<SysEvent>(*(event.get()));
    GetHiviewContext()->PostUnorderedEvent(shared_from_this(), eventPtr);
    return true;
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
    event->SetEventValue("MSG", StringUtil::ReplaceStr(event->GetEventValue("MSG"), "\\n", "\n"));
    std::string msg = event->GetEventValue("MSG");
    headerStream << "MSG = " << msg << std::endl;

    FileUtil::SaveStringToFd(fd, headerStream.str());
    return true;
}

bool EventLogger::JudgmentRateLimiting(std::shared_ptr<SysEvent> event)
{
    auto interval = event->GetIntValue("eventLog_interval");
    if (interval == 0) {
        return true;
    }

    long pid = event->GetEventIntValue("PID");
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
    auto eventQuery = EventStore::SysEventDao::BuildQuery(event->what_);
    std::vector<std::string> selections { EventStore::EventCol::TS };
    EventStore::ResultSet set = (*eventQuery).Select(selections)
        .Where(EventStore::EventCol::TS, EventStore::Op::EQ, static_cast<int64_t>(event->happenTime_))
        .And(EventStore::EventCol::DOMAIN, EventStore::Op::EQ, event->domain_)
        .And(EventStore::EventCol::NAME, EventStore::Op::EQ, event->eventName_)
        .Execute();
    if (set.GetErrCode() != 0) {
        HIVIEW_LOGE("failed to get db, error:%{public}d.", set.GetErrCode());
        return false;
    }
    if (set.HasNext()) {
        auto record = set.Next();
        if (record->GetSeq() == event->GetSeq()) {
            HIVIEW_LOGI("Seq match success.");
            if (logFile == "nolog") {
                HIVIEW_LOGI("set info_ with nolog into db.");
                event->SetEventValue(EventStore::EventCol::INFO, "nolog", false);
            } else {
                auto logPath = R"~(logPath:)~" + LOGGER_EVENT_LOG_PATH  + "/" + logFile;
                event->SetEventValue(EventStore::EventCol::INFO, logPath, true);
            }

            auto retCode = EventStore::SysEventDao::Update(event, false);
            if (retCode == 0) {
                return true;
            }
        }
    }
    HIVIEW_LOGE("eventLog LogPath update to DB failed!");
    return false;
}

void EventLogger::OnLoad()
{
    HIVIEW_LOGI("EventLogger OnLoad.");
    SetName("EventLogger");
    SetVersion("1.0");
    logStore_->SetMaxSize(MAX_FOLDER_SIZE);
    logStore_->SetMinKeepingFileNumber(MAX_FILE_NUM);
    logStore_->Init();
    std::shared_ptr<EventLoop> tmp = GetWorkLoop();
    tmp->AddFileDescriptorEventCallback("EventLoggerFd",
        std::static_pointer_cast<EventLogger>(shared_from_this()));

    EventLoggerConfig logConfig;
    eventLoggerConfig_ = logConfig.GetConfig();

    eventPool_ = std::make_unique<EventThreadPool>(maxEventPoolCount, "EventLog");
    eventPool_->Start();

    FreezeCommon freezeCommon;
    if (!freezeCommon.Init()) {
        HIVIEW_LOGE("FreezeCommon filed.");
        return;
    }
    std::set<EventListener::EventIdRange> listenerInfo;
    std::set<std::string> eventNames = freezeCommon.GetPrincipalStringIds();
    auto context = GetHiviewContext();
    if (context != nullptr) {
        auto plugin = context->GetPluginByName("FreezeDetectorPlugin");
        context->AddListenerInfo(Event::MessageType::SYS_EVENT, plugin, eventNames, listenerInfo);
        context->RegisterDynamicListenerInfo(plugin);
    }
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
    if (sysEvent->ParseJson() < 0) {
        HIVIEW_LOGW("Failed to parse EventLogger from queryResult file name %{public}s.", fileName.c_str());
        return;
    }
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
} // namesapce HiviewDFX
} // namespace OHOS
