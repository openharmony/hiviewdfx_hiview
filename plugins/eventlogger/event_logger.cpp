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

#include <cinttypes>
#include <list>
#include <map>
#include <mutex>
#include <regex>
#include <sstream>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "parameter.h"

#include "common_utils.h"
#include "dfx_json_formatter.h"
#include "event_source.h"
#include "file_util.h"
#include "freeze_json_util.h"
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
    HIVIEW_LOGD("event time:%{public}" PRIu64 " jsonExtraInfo is %{public}s", TimeUtil::GetMilliseconds(),
        sysEvent->AsJsonStr().c_str());

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

    long pid = sysEvent->GetEventIntValue("PID");
    pid = pid ? pid : sysEvent->GetPid();
    HIVIEW_LOGI("event: domain=%{public}s, eventName=%{public}s, pid=%{public}ld", sysEvent->domain_.c_str(),
        sysEvent->eventName_.c_str(), pid);

    if (sysEvent->eventName_ == "THREAD_BLOCK_6S" || sysEvent->eventName_ == "APP_INPUT_BLOCK") {
        long lastPid = lastPid_;
        std::string lastEventName = lastEventName_;
        lastPid_ = pid;
        lastEventName_ = sysEvent->eventName_;
        if (lastPid == pid) {
            HIVIEW_LOGI("eventName=%{public}s, pid=%{public}ld has happened", lastEventName.c_str(), pid);
            return true;
        }
    }

    if (sysEvent->GetValue("eventLog_action").empty()) {
        HIVIEW_LOGI("eventName=%{public}s, pid=%{public}ld, eventLog_action is empty.",
            sysEvent->eventName_.c_str(), pid);
        UpdateDB(sysEvent, "nolog");
        return true;
    }

    sysEvent->OnPending();

    auto task = [this, sysEvent]() {
        HIVIEW_LOGD("event time:%{public}" PRIu64 " jsonExtraInfo is %{public}s", TimeUtil::GetMilliseconds(),
            sysEvent->AsJsonStr().c_str());
        if (!JudgmentRateLimiting(sysEvent)) {
            return;
        }
        this->StartLogCollect(sysEvent);
    };
    HIVIEW_LOGI("before add task to eventpool, eventName=%{public}s, pid=%{public}ld",
        sysEvent->eventName_.c_str(), pid);
    eventPool_->AddTask(task, "eventlogger");
    HIVIEW_LOGI("after add task to eventpool, eventName=%{public}s, pid=%{public}ld",
        sysEvent->eventName_.c_str(), pid);
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

    int jsonFd = -1;
    if (FreezeJsonUtil::IsAppFreeze(event->eventName_)) {
        std::string jsonFilePath = FreezeJsonUtil::GetFilePath(event->GetEventIntValue("PID"),
            event->GetEventIntValue("UID"), event->happenTime_);
        jsonFd = FreezeJsonUtil::GetFd(jsonFilePath);
    }

    std::unique_ptr<EventLogTask> logTask = std::make_unique<EventLogTask>(fd, jsonFd, event);
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
    WriteFreezeJsonInfo(fd, jsonFd, event);
    auto ret = logTask->StartCompose();
    if (ret != EventLogTask::TASK_SUCCESS) {
        HIVIEW_LOGE("capture fail %{public}d", ret);
    }
    auto end = TimeUtil::GetMilliseconds();
    std::string totalTime = "\n\nCatcher log total time is " + std::to_string(end - start) + "ms\n";
    FileUtil::SaveStringToFd(fd, totalTime);
    close(fd);
    if (jsonFd >= 0) {
        close(jsonFd);
    }
    UpdateDB(event, logFile);

    constexpr int waitTime = 1;
    auto CheckFinishFun = std::bind(&EventLogger::CheckEventOnContinue, this, event);
    threadLoop_->AddTimerEvent(nullptr, nullptr, CheckFinishFun, waitTime, false);
    HIVIEW_LOGI("Collect on finish, name: %{public}s", logFile.c_str());
}

bool ParseMsgForMessageAndEventHandler(const std::string& msg, std::string& message, std::string& eventHandlerStr)
{
    std::vector<std::string> lines;
    StringUtil::SplitStr(msg, "\n", lines, false, true);
    bool isGetMessage = false;
    std::string messageStartFlag = "Fault time:";
    std::string messageEndFlag = "mainHandler dump is:";
    std::string eventFlag = "Event {";
    bool isGetEvent = false;
    std::regex eventStartFlag(".*((Immediate)|(High)|(Low)) priority event queue information:.*");
    std::regex eventEndFlag(".*Total size of ((Immediate)|(High)|(Low)) events :.*");
    std::list<std::string> eventHandlerList;
    for (auto line = lines.begin(); line != lines.end(); line++) {
        if ((*line).find(messageStartFlag) != std::string::npos) {
            isGetMessage = true;
            continue;
        }
        if (isGetMessage) {
            if ((*line).find(messageEndFlag) != std::string::npos) {
                isGetMessage = false;
                HIVIEW_LOGI("Get FreezeJson message jsonStr: %{public}s", message.c_str());
                continue;
            }
            message += StringUtil::TrimStr(*line);
            continue;
        }
        if (regex_match(*line, eventStartFlag)) {
            isGetEvent = true;
            continue;
        }
        if (isGetEvent) {
            if (regex_match(*line, eventEndFlag)) {
                isGetEvent = false;
                continue;
            }
            std::string::size_type pos = (*line).find(eventFlag);
            if (pos == std::string::npos) {
                continue;
            }
            std::string handlerStr = StringUtil::TrimStr(*line).substr(pos);
            HIVIEW_LOGI("Get EventHandler str: %{public}s.", handlerStr.c_str());
            eventHandlerList.push_back(handlerStr);
        }
    }
    eventHandlerStr = FreezeJsonUtil::GetStrByList(eventHandlerList);
    return true;
}

void ParsePeerBinder(const std::string& binderInfo, std::string& binderInfoJsonStr)
{
    std::vector<std::string> lines;
    StringUtil::SplitStr(binderInfo, "\\n", lines, false, true);
    std::list<std::string> infoList;
    std::map<std::string, std::string> processNameMap;

    for (auto lineIt = lines.begin(); lineIt != lines.end(); lineIt++) {
        std::string line = *lineIt;
        if (line.empty() || line.find("async") != std::string::npos) {
            continue;
        }

        if (line.find("context") != line.npos) {
            break;
        }

        std::istringstream lineStream(line);
        std::vector<std::string> strList;
        std::string tmpstr;
        while (lineStream >> tmpstr) {
            strList.push_back(tmpstr);
        }
        if (strList.size() < 7) { // less than 7: valid array size
            continue;
        }
        // 2: binder peer id
        std::string pidStr = strList[2].substr(0, strList[2].find(":"));
        if (pidStr == "") {
            continue;
        }
        if (processNameMap.find(pidStr) == processNameMap.end()) {
            std::ifstream cmdLineFile("/proc/" + pidStr + "/cmdline");
            std::string processName;
            if (cmdLineFile) {
                std::getline(cmdLineFile, processName);
                cmdLineFile.close();
                processName = StringUtil::FormatCmdLine(processName);
                processNameMap[pidStr] = processName;
            } else {
                HIVIEW_LOGE("Fail to open /proc/%{public}s/cmdline", pidStr.c_str());
            }
        }
        std::string lineStr = line + "    " + pidStr + FreezeJsonUtil::WrapByParenthesis(processNameMap[pidStr]);
        infoList.push_back(lineStr);
    }
    binderInfoJsonStr = FreezeJsonUtil::GetStrByList(infoList);
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
    if (event->GetEventValue("MODULE_NAME") != "") {
        headerStream << "MODULE_NAME = " << event->GetEventValue("MODULE_NAME") << std::endl;
    } else {
        headerStream << "PACKAGE_NAME = " << event->GetEventValue("PACKAGE_NAME") << std::endl;
    }
    headerStream << "PROCESS_NAME = " << event->GetEventValue("PROCESS_NAME") << std::endl;
    headerStream << "eventLog_action = " << event->GetValue("eventLog_action") << std::endl;
    headerStream << "eventLog_interval = " << event->GetValue("eventLog_interval") << std::endl;

    FileUtil::SaveStringToFd(fd, headerStream.str());
    return true;
}

bool EventLogger::WriteFreezeJsonInfo(int fd, int jsonFd, std::shared_ptr<SysEvent> event)
{
    std::string msg = StringUtil::ReplaceStr(event->GetEventValue("MSG"), "\\n", "\n");
    std::string stack;
    std::string binderInfo = event -> GetEventValue("BINDER_INFO");
    if (FreezeJsonUtil::IsAppFreeze(event -> eventName_)) {
        std::string message;
        std::string eventHandlerStr;
        ParseMsgForMessageAndEventHandler(msg, message, eventHandlerStr);
        long runtimeId = event->GetEventIntValue("RUNTIME_ID");

        std::string jsonStack = event->GetEventValue("STACK");
        if (!jsonStack.empty() && jsonStack[0] == '[') { // json stack info should start with '['
            jsonStack = StringUtil::UnescapeJsonStringValue(jsonStack);
            if (!DfxJsonFormatter::FormatJsonStack(jsonStack, stack)) {
                stack = jsonStack;
            }
        } else {
            stack = jsonStack;
        }

        if (jsonFd >= 0) {
            HIVIEW_LOGI("success to open FreezeJsonFile! jsonFd: %{public}d", jsonFd);
            FreezeJsonUtil::WriteKeyValue(jsonFd, "message", message);
            FreezeJsonUtil::WriteKeyValue(jsonFd, "event_handler", eventHandlerStr);
            FreezeJsonUtil::WriteKeyValue(jsonFd, "runtimeId", runtimeId);
            FreezeJsonUtil::WriteKeyValue(jsonFd, "stack", jsonStack);
        } else {
            HIVIEW_LOGE("fail to open FreezeJsonFile! jsonFd: %{public}d", jsonFd);
        }

        if (!binderInfo.empty() && jsonFd >= 0) {
            std::string binderInfoJsonStr;
            ParsePeerBinder(binderInfo, binderInfoJsonStr);
            FreezeJsonUtil::WriteKeyValue(jsonFd, "peer_binder", binderInfoJsonStr);
        }
    } else {
        stack = event->GetEventValue("STACK");
    }

    std::ostringstream oss;
    oss << "MSG = " << msg << std::endl;
    if (!stack.empty()) {
        oss << StringUtil::UnescapeJsonStringValue(stack) << std::endl;
    }
    if (!binderInfo.empty()) {
        oss << StringUtil::UnescapeJsonStringValue(binderInfo) << std::endl;
    }
    FileUtil::SaveStringToFd(fd, oss.str());
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
                interval:%{public}" PRId32 " There's not enough interval",
                event->eventId_, eventName.c_str(), eventPid.c_str(), interval);
            return false;
        }
    }
    eventTagTime_[tagTimeName] = now;
    HIVIEW_LOGI("event: id:0x%{public}d, eventName:%{public}s pid:%{public}s. \
        interval:%{public}" PRId32 " normal interval",
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

void EventLogger::CheckEventOnContinue(std::shared_ptr<SysEvent> event)
{
    event->ResetPendingStatus();
    event->OnContinue();
}

void EventLogger::OnLoad()
{
    HIVIEW_LOGI("EventLogger OnLoad.");
    SetName("EventLogger");
    SetVersion("1.0");
    logStore_->SetMaxSize(MAX_FOLDER_SIZE);
    logStore_->SetMinKeepingFileNumber(MAX_FILE_NUM);
    LogStoreEx::LogFileComparator comparator = [this](const LogFile &lhs, const LogFile &rhs) {
        return rhs < lhs;
    };
    logStore_->SetLogFileComparator(comparator);
    logStore_->Init();
    threadLoop_ = GetWorkLoop();

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

        auto ptr = std::static_pointer_cast<EventLogger>(shared_from_this());
        context->RegisterUnorderedEventListener(ptr);
        AddListenerInfo(Event::MessageType::PLUGIN_MAINTENANCE);
    }

    GetCmdlineContent();
    GetRebootReasonConfig();
}

void EventLogger::OnUnload()
{
    HIVIEW_LOGD("called");
    eventPool_->Stop();
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

    if (event == nullptr) {
        HIVIEW_LOGW("event is null.");
        return;
    }

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
    if (context != nullptr) {
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
            if (std::any_of(rebootReasons_.begin(), rebootReasons_.end(), [&reboot, &reset](auto& reason) {
                return (reason == reboot || reason == reset);
            })) {
                HIVIEW_LOGI("get reboot reason: LONG_PRESS.");
                return LONG_PRESS;
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
