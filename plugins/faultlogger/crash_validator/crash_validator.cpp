/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "crash_validator.h"


#include <csignal>
#include <iostream>

#include <fcntl.h>
#include <hisysevent.h>
#include <securec.h>

#include "plugin_factory.h"
#include "logger.h"
#include "sys_event.h"

namespace OHOS {
namespace HiviewDFX {
REGISTER(CrashValidator);
DEFINE_LOG_TAG("CrashValidator");
namespace {
constexpr char EVENT_CPP_CRASH[] = "CPP_CRASH";
constexpr char KEY_PROCESS_EXIT[] = "PROCESS_EXIT";
constexpr char KEY_NAME[] = "PROCESS_NAME";
constexpr char KEY_PID[] = "PID";
constexpr char KEY_UID[] = "UID";
constexpr char KEY_STATUS[] = "STATUS";
constexpr char KEY_LOG_PATH[] = "LOG_PATH";
constexpr char KEY_MODULE[] = "MODULE";
constexpr char INIT_LOG_PATTERN[] = "SigHandler, SIGCHLD received, ";
constexpr char KEY_NO_LOG_EVENT_NAME[] = "CPP_CRASH_NO_LOG";
constexpr char KEY_HAPPEN_TIME[] = "HAPPEN_TIME";
constexpr int32_t LOG_SIZE = 1024;
constexpr uint64_t MAX_LOG_GENERATE_TIME = 600; // 600 seconds
constexpr int32_t KMSG_SIZE = 2049;
}
CrashValidator::CrashValidator() : stopReadKmsg_(false), totalEventCount_(0),
    normalEventCount_(0), kmsgReaderThread_(nullptr)
{
    name_ = "CrashValidator";
}

CrashValidator::~CrashValidator()
{
    if (kmsgReaderThread_ != nullptr) {
        kmsgReaderThread_ = nullptr;
    }
}

std::string CrashValidator::GetListenerName()
{
    return name_;
}

bool CrashValidator::ReadyToLoad()
{
    return true;
}

bool CrashValidator::CanProcessEvent(std::shared_ptr<Event> event)
{
    return false;
}

void CrashValidator::PrintEvents(int fd, const std::vector<CrashEvent>& events)
{
    std::vector<CrashEvent>::const_iterator it = events.begin(); 
    while (it != events.end()) {
        dprintf(fd, "Module:%s Time:%" PRIu64 " Pid:%" PRIu64 " Uid:%" PRIu64 " HasLog:%d\n",
            it->name.c_str(),
            static_cast<uint64_t>(it->time),
            static_cast<uint64_t>(it->pid),
            static_cast<uint64_t>(it->uid),
            it->isCppCrash);
        it++;
    }
}

void CrashValidator::Dump(int fd, const std::vector<std::string>& cmds)
{
    dprintf(fd, "Summary:\n");
    dprintf(fd, "Total Signaled Process:%d\n", totalEventCount_);
    dprintf(fd, "Total CppCrash Count:%d\n", normalEventCount_);
    if (totalEventCount_ > 0) {
        dprintf(fd, "CppCrash detect rate:%d%%\n",
            (normalEventCount_ * 100) / totalEventCount_); // 100 : percent ratio
    }

    std::lock_guard<std::mutex> lock(lock_);
    if (!noLogEvents_.empty()) {
        dprintf(fd, "No CppCrash Log Events(%zu):\n", noLogEvents_.size());
        PrintEvents(fd, noLogEvents_);
    }

    if (!pendingEvents_.empty()) {
        dprintf(fd, "Pending CppCrash Log Events(%zu):\n", pendingEvents_.size());
        PrintEvents(fd, pendingEvents_);
    }

    if (!matchedEvents_.empty()) {
        dprintf(fd, "Matched Events(%zu):\n", matchedEvents_.size());
        PrintEvents(fd, matchedEvents_);
    }
}

bool CrashValidator::OnEvent(std::shared_ptr<Event>& event)
{
    OnUnorderedEvent(*(event.get()));
    return true;
}

void CrashValidator::OnUnorderedEvent(const Event &event)
{
    if (GetHiviewContext() == nullptr) {
        HIVIEW_LOGE("failed to get context.");
        return;
    }

    std::lock_guard<std::mutex> lock(lock_);
    Event& eventRef = const_cast<Event&>(event);
    SysEvent& sysEvent = static_cast<SysEvent&>(eventRef);
    if (eventRef.eventName_ == EVENT_CPP_CRASH) {
        HandleCppCrashEvent(sysEvent);
    } else if (eventRef.eventName_ == KEY_PROCESS_EXIT) {
        HandleProcessExitEvent(sysEvent);
    }
}

bool CrashValidator::RemoveSimilarEvent(const CrashEvent& event)
{
    std::vector<CrashEvent>::iterator it = pendingEvents_.begin(); 
    while (it != pendingEvents_.end()) {
        if (it->uid == event.uid && it->pid == event.pid) {
            it = pendingEvents_.erase(it);
            normalEventCount_++;
            return true;
        } else {
            ++it;
        }
    }
    return false;
}

void CrashValidator::HandleCppCrashEvent(SysEvent& sysEvent)
{
    CrashEvent crashEvent;
    crashEvent.isCppCrash = true;
    crashEvent.time = sysEvent.happenTime_;
    crashEvent.uid = sysEvent.GetEventIntValue(KEY_UID);
    crashEvent.pid = sysEvent.GetEventIntValue(KEY_PID);
    crashEvent.path = sysEvent.GetEventValue(KEY_LOG_PATH);
    crashEvent.name = sysEvent.GetEventValue(KEY_MODULE);
    HIVIEW_LOGI("CrashValidator Plugin is handling CPPCRASH event [PID : %{public}d]\n", crashEvent.pid);
    if (!RemoveSimilarEvent(crashEvent)) {
        pendingEvents_.push_back(crashEvent);
    }
}

void CrashValidator::HandleProcessExitEvent(SysEvent& sysEvent)
{
    CrashEvent crashEvent;
    crashEvent.isCppCrash = false;
    crashEvent.time = sysEvent.happenTime_;
    crashEvent.pid = sysEvent.GetEventIntValue(KEY_PID);
    crashEvent.uid = sysEvent.GetEventIntValue(KEY_UID);
    crashEvent.name = sysEvent.GetEventValue(KEY_NAME);
    int status = static_cast<int>(sysEvent.GetEventIntValue(KEY_STATUS));
    if (!WIFSIGNALED(status)) {
        return;
    }

    int interestedSignalList[] = {
        SIGABRT, SIGBUS, SIGFPE, SIGILL,
        SIGSEGV, SIGSTKFLT, SIGSYS, SIGTRAP };
    bool shouldGenerateCppCrash = false;
    for (size_t i = 0; i < sizeof(interestedSignalList) / sizeof(interestedSignalList[0]); i++) {
        if (interestedSignalList[i] == WTERMSIG(status)) {
            shouldGenerateCppCrash = true;
            break;
        }
    }

    if (!shouldGenerateCppCrash) {
        return;
    }

    HIVIEW_LOGI("Process Exit Name:%{public}s Time:%{public}llu Pid:%{public}d Uid:%{public}d status:%{public}d\n",
        crashEvent.name.c_str(),
        static_cast<unsigned long long>(crashEvent.time),
        crashEvent.pid,
        crashEvent.uid,
        status);
    totalEventCount_++;
    if (!RemoveSimilarEvent(crashEvent)) {
        pendingEvents_.push_back(crashEvent);
    }
}

void CrashValidator::CheckOutOfDateEvents()
{
    std::vector<CrashEvent>::iterator it = pendingEvents_.begin(); 
    while (it != pendingEvents_.end()) {
        uint64_t now = time(nullptr);
        uint64_t eventTime = it->time;
        if (eventTime > now) {
            eventTime = eventTime / 1000; // 1000 : convert to second
        }

        if (now > eventTime && now - eventTime < MAX_LOG_GENERATE_TIME) {
            it++;
            continue;
        }

        if (!it->isCppCrash) {
            HiSysEvent::Write("RELIABILITY", KEY_NO_LOG_EVENT_NAME, HiSysEvent::EventType::FAULT,
                KEY_NAME, it->name,
                KEY_PID, it->pid,
                KEY_UID, it->uid,
                KEY_HAPPEN_TIME, it->time);
            noLogEvents_.push_back(*it);
        } else {
            totalEventCount_++;
            normalEventCount_++;
        }
        it = pendingEvents_.erase(it);
    }
}

void CrashValidator::ReadServiceCrashStatus()
{
    char kmsg[KMSG_SIZE];
    int fd = open("/dev/kmsg", O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        HIVIEW_LOGE("Failed to open /dev/kmsg.");
        return;
    }
    lseek(fd, 0, 3); // 3 : SEEK_DATA
    while (true) {
        ssize_t len;
        if (-1 == (len = read(fd, kmsg, sizeof(kmsg))) && errno == EPIPE) {
            continue;
        }
        if (len == -1 && errno == EINVAL) {
            HIVIEW_LOGE("Failed to read kmsg");
            close(fd);
            return;
        }
        if (len < 1) {
            continue;
        }
        kmsg[len] = 0;
        if (stopReadKmsg_) {
            break;
        }
        std::string line = kmsg;
        auto pos = line.find(INIT_LOG_PATTERN);
        if (pos == std::string::npos) {
            continue;
        }
        std::string formatedLog = line.substr(pos + strlen(INIT_LOG_PATTERN));
        char name[LOG_SIZE] {0};
        int pid;
        int uid;
        int status;
        int ret = sscanf_s(formatedLog.c_str(), "Service:%s pid:%d uid:%d status:%d",
            name, sizeof(name), &pid, &uid, &status);
        if (ret <= 0) {
            HIVIEW_LOGI("Failed to parse kmsg:%{public}s.", formatedLog.c_str());
            continue;
        }

        HIVIEW_LOGI("report PROCESS_EXIT event[name : %{public}s  pid : %{public}d]", name, pid);

        HiSysEvent::Write(HiSysEvent::Domain::STARTUP, KEY_PROCESS_EXIT, HiSysEvent::EventType::BEHAVIOR,
            KEY_NAME, name, KEY_PID, pid, KEY_UID, uid, KEY_STATUS, status);
    }
    close(fd);
}

bool CrashValidator::ValidateLogContent(const CrashEvent& event)
{
    // check later
    return true;
}

void CrashValidator::OnLoad()
{
    kmsgReaderThread_ = std::make_unique<std::thread>(&CrashValidator::ReadServiceCrashStatus, this);
    kmsgReaderThread_->detach();
    AddListenerInfo(Event::MessageType::SYS_EVENT, EVENT_CPP_CRASH);
    AddListenerInfo(Event::MessageType::SYS_EVENT, KEY_PROCESS_EXIT);
    GetHiviewContext()->RegisterUnorderedEventListener(
        std::static_pointer_cast<CrashValidator>(shared_from_this()));
    GetHiviewContext()->AppendPluginToPipeline(GetName(), "SysEventPipeline");
}

void CrashValidator::OnUnload()
{
    stopReadKmsg_ = true;
    totalEventCount_ = 0;
    normalEventCount_ = 0;
    pendingEvents_.clear();
    noLogEvents_.clear();
    matchedEvents_.clear();
}
} // namespace HiviewDFX
} // namespace OHOS
