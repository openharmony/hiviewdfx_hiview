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
#include <fstream>
#include <iostream>

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
}
CrashValidator::CrashValidator() : stopReadKmsg_(false), totalEventCount_(0),
    normalEventCount_(0), kmsgReaderThread_(nullptr)
{
}

CrashValidator::~CrashValidator()
{
    if (kmsgReaderThread_ != nullptr) {
        kmsgReaderThread_ = nullptr;
    }
}

bool CrashValidator::ReadyToLoad()
{
    return true;
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

    if (!logContentMissingEvents_.empty()) {
        dprintf(fd, "Uncompleted Events(%zu):\n", logContentMissingEvents_.size());
        PrintEvents(fd, logContentMissingEvents_);
    }

    if (!pendingEvents_.empty()) {
        dprintf(fd, "Unmatched CppCrash Log Events(%zu):\n", pendingEvents_.size());
        PrintEvents(fd, pendingEvents_);
    }
}

bool CrashValidator::OnEvent(std::shared_ptr<Event>& event)
{
    if (event == nullptr || event->jsonExtraInfo_.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(lock_);
    if (event->eventName_ == EVENT_CPP_CRASH) {
        HandleCppCrashEvent(event);
    } else if (event->eventName_ == KEY_PROCESS_EXIT) {
        HandleProcessExitEvent(event);
    }
    return true;
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

void CrashValidator::HandleCppCrashEvent(std::shared_ptr<Event>& event)
{
    auto sysEvent = std::static_pointer_cast<SysEvent>(event);
    CrashEvent crashEvent;
    crashEvent.isCppCrash = true;
    crashEvent.time = sysEvent->happenTime_;
    crashEvent.uid = static_cast<uint64_t>(sysEvent->GetUid());
    crashEvent.pid = static_cast<uint64_t>(sysEvent->GetPid());
    crashEvent.path = sysEvent->GetEventValue(KEY_LOG_PATH);
    crashEvent.name = sysEvent->GetEventValue(KEY_MODULE);
    if (!RemoveSimilarEvent(crashEvent)) {
        pendingEvents_.push_back(crashEvent);
    }
    CheckOutOfDateEvents();
}

void CrashValidator::HandleProcessExitEvent(std::shared_ptr<Event>& event)
{
    auto sysEvent = std::static_pointer_cast<SysEvent>(event);
    CrashEvent crashEvent;
    crashEvent.isCppCrash = false;
    crashEvent.time = sysEvent->happenTime_;
    crashEvent.pid = sysEvent->GetEventIntValue(KEY_PID);
    crashEvent.uid = sysEvent->GetEventIntValue(KEY_UID);
    crashEvent.name = sysEvent->GetEventValue(KEY_NAME);
    int status = static_cast<int>(sysEvent->GetEventIntValue(KEY_STATUS));
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
    CheckOutOfDateEvents();
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
    std::ifstream kmsgStream;
    std::string line;
    kmsgStream.open("/dev/kmsg");
    while (kmsgStream.good()) {
        std::getline(kmsgStream, line);
        auto pos = line.find(INIT_LOG_PATTERN);
        if (pos == std::string::npos) {
            continue;
        }

        if (stopReadKmsg_) {
            break;
        }

        std::string formatedLog = line.substr(pos + strlen(INIT_LOG_PATTERN));
        char name[LOG_SIZE] {0};
        int pid;
        int uid;
        int status;
        int  ret = sscanf_s(formatedLog.c_str(), "Service:%s pid:%d uid:%d status:%d",
            name, sizeof(name), &pid, &uid, &status);
        if (ret <= 0) {
            HIVIEW_LOGI("Failed to parse kmsg:%{public}s.", formatedLog.c_str());
            continue;
        }

        HiSysEvent::Write(HiSysEvent::Domain::STARTUP, KEY_PROCESS_EXIT, HiSysEvent::EventType::BEHAVIOR,
            KEY_NAME, name, KEY_PID, pid, KEY_UID, uid, KEY_STATUS, status);
    }
    kmsgStream.close();
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
}

void CrashValidator::OnUnload()
{
    stopReadKmsg_ = true;
    totalEventCount_ = 0;
    normalEventCount_ = 0;
    pendingEvents_.clear();
    noLogEvents_.clear();
    logContentMissingEvents_.clear();
}
} // namespace HiviewDFX
} // namespace OHOS
