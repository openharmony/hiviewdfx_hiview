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
#include "event_log_task.h"

#include <unistd.h>

#include "securec.h"

#include "common_utils.h"
#include "logger.h"
#include "string_util.h"

#include "binder_catcher.h"
#include "open_stacktrace_catcher.h"
#include "peer_binder_catcher.h"
#include "command_catcher.h"
namespace OHOS {
namespace HiviewDFX {
namespace {
const std::string SYSTEM_STACK[] = {
    "foundation",
};
}
DEFINE_LOG_LABEL(0xD002D01, "EventLogger-EventLogTask");
EventLogTask::EventLogTask(int fd, std::shared_ptr<SysEvent> event)
    : targetFd_(fd),
      event_(event),
      maxLogSize_(DEFAULT_LOG_SIZE),
      taskLogSize_(0),
      status_(Status::TASK_RUNNABLE)
{
    cmdCatcher_ = nullptr;
    captureList_.insert(std::pair<std::string, capture>("s", std::bind(&EventLogTask::AppStackCapture, this)));
    captureList_.insert(std::pair<std::string, capture>("S", std::bind(&EventLogTask::SystemStackCapture, this)));
    captureList_.insert(std::pair<std::string, capture>("b", std::bind(&EventLogTask::BinderLogCapture, this)));
    captureList_.insert(std::pair<std::string, capture>("cmd:c", std::bind(&EventLogTask::CpuUsageCapture, this)));
    captureList_.insert(std::pair<std::string, capture>("cmd:m", std::bind(&EventLogTask::MemoryUsageCapture, this)));
    captureList_.insert(std::pair<std::string, capture>("cmd:w", std::bind(&EventLogTask::WMSUsageCapture, this)));
    captureList_.insert(std::pair<std::string, capture>("cmd:a", std::bind(&EventLogTask::AMSUsageCapture, this)));
    captureList_.insert(std::pair<std::string, capture>("cmd:p", std::bind(&EventLogTask::PMSUsageCapture, this)));
}

void EventLogTask::AddLog(const std::string &cmd)
{
    if (tasks_.size() == 0) {
        status_ = Status::TASK_RUNNABLE;
    }

    if (captureList_.find(cmd) != captureList_.end()) {
        captureList_[cmd]();
        return;
    }
    PeerBinderCapture(cmd);
}

EventLogTask::Status EventLogTask::StartCompose()
{
    // nothing to do, return success
    if (status_ != Status::TASK_RUNNABLE) {
        return status_;
    }
    status_ = Status::TASK_RUNNING;
    // nothing to do, return success
    if (tasks_.size() == 0) {
        return Status::TASK_SUCCESS;
    }

    auto dupedFd = dup(targetFd_);
    uint32_t catcherIndex = 0;
    for (auto& catcher : tasks_) {
        catcherIndex++;
        if (dupedFd < 0) {
            status_ = Status::TASK_FAIL;
            AddStopReason(targetFd_, catcher, "Fail to dup file descriptor, exit!");
            return TASK_FAIL;
        }

        AddSeparator(dupedFd, catcher);
        int curLogSize = catcher->Catch(dupedFd);
        if (ShouldStopLogTask(dupedFd, catcherIndex, curLogSize, catcher)) {
            break;
        }
    }
    close(dupedFd);

    if (status_ == Status::TASK_RUNNING) {
        status_ = Status::TASK_SUCCESS;
    }
    return status_;
}

bool EventLogTask::ShouldStopLogTask(int fd, uint32_t curTaskIndex, int curLogSize,
    std::shared_ptr<EventLogCatcher> catcher)
{
    if (status_ == Status::TASK_TIMEOUT) {
        HIVIEW_LOGE("Break Log task, parent has timeout.");
        return true;
    }

    bool encounterErr = (curLogSize < 0);
    bool hasFinished = (curTaskIndex == tasks_.size());
    if (!encounterErr) {
        taskLogSize_ += curLogSize;
    }

    if (taskLogSize_ > maxLogSize_ && !hasFinished) {
        AddStopReason(fd, catcher, "Exceed max log size");
        status_ = Status::TASK_EXCEED_SIZE;
        return true;
    }

    if (encounterErr) {
        AddStopReason(fd, catcher, "Log catcher not successful");
        HIVIEW_LOGE("catcher %{public}s, Log catcher not successful", catcher->GetDescription().c_str());
    }
    return false;
}

void EventLogTask::AddStopReason(int fd, std::shared_ptr<EventLogCatcher> catcher, const std::string& reason)
{
    char buf[BUF_SIZE_512] = {0};
    int ret = -1;
    if (catcher != nullptr) {
        catcher->Stop();
        // sleep 1s for syncing log to the fd, then we could append failure reason ?
        sleep(1);
        std::string summary = catcher->GetDescription();
        ret = snprintf_s(buf, BUF_SIZE_512, BUF_SIZE_512 - 1, "\nTask stopped when running catcher:%s, Reason:%s \n",
                         summary.c_str(), reason.c_str());
    } else {
        ret = snprintf_s(buf, BUF_SIZE_512, BUF_SIZE_512 - 1, "\nTask stopped, Reason:%s \n", reason.c_str());
    }

    if (ret > 0) {
        write(fd, buf, strnlen(buf, BUF_SIZE_512));
        fsync(fd);
    }
}

void EventLogTask::AddSeparator(int fd, std::shared_ptr<EventLogCatcher> catcher) const
{
    char buf[BUF_SIZE_512] = {0};
    std::string summary = catcher->GetDescription();
    if (summary.empty()) {
        HIVIEW_LOGE("summary.empty() catcher is %{public}s", catcher->GetName().c_str());
        return;
    }

    int ret = snprintf_s(buf, BUF_SIZE_512, BUF_SIZE_512 - 1, "\n%s:\n", summary.c_str());
    if (ret > 0) {
        write(fd, buf, strnlen(buf, BUF_SIZE_512));
        fsync(fd);
    }
}

EventLogTask::Status EventLogTask::GetTaskStatus() const
{
    return status_;
}

long EventLogTask::GetLogSize() const
{
    return taskLogSize_;
}

std::shared_ptr<CommandCatcher> EventLogTask::GetCmdCatcher()
{
    if (cmdCatcher_ != nullptr) {
        return cmdCatcher_;
    }

    auto capture = std::make_shared<CommandCatcher>();
    int pid = event_->GetEventIntValue("PID");
    pid = pid ? pid : event_->GetPid();
    capture->Initialize(event_->GetEventValue("PACKAGE_NAME"), pid, 0);
    tasks_.push_back(capture);
    cmdCatcher_ = capture;
    return cmdCatcher_;
}

void EventLogTask::AppStackCapture()
{
    auto capture = std::make_shared<OpenStacktraceCatcher>();
    int pid = event_->GetEventIntValue("PID");
    pid = pid ? pid : event_->GetPid();
    capture->Initialize(event_->GetEventValue("PACKAGE_NAME"), pid, 0);
    tasks_.push_back(capture);
}

void EventLogTask::SystemStackCapture()
{
    for (auto packageName : SYSTEM_STACK) {
        auto capture = std::make_shared<OpenStacktraceCatcher>();
        capture->Initialize(packageName, 0, 0);
        tasks_.push_back(capture);
    }
}

void EventLogTask::BinderLogCapture()
{
    auto capture = std::make_shared<BinderCatcher>();
    capture->Initialize("", 0, 0);
    tasks_.push_back(capture);
}

bool EventLogTask::PeerBinderCapture(const std::string &cmd)
{
    auto find = cmd.find("pb");
    if (find == cmd.npos) {
        return false;
    }

    std::vector<std::string> cmdList;
    StringUtil::SplitStr(cmd, ":", cmdList, true);
    if (cmdList.front() != "pb") {
        return false;
    }

    int layer = 0;
    int pid = event_->GetEventIntValue("PID");
    pid = pid ? pid : event_->GetPid();
    StringUtil::StrToInt(cmdList.back(), layer);
    HIVIEW_LOGI("pid:%{public}d, value:%{public}d ", pid, layer);

    auto capture = std::make_shared<PeerBinderCatcher>();
    capture->Initialize("", pid, layer);
    capture->Init(event_, "");
    tasks_.push_back(capture);

    return true;
}

void EventLogTask::WMSUsageCapture()
{
    auto cmdCatcher = GetCmdCatcher();
    std::vector<std::string> cmd = {"hidumper", "-s", "WindowManagerService", "-a", "-a"};
    cmdCatcher->AddCmd(cmd);
}

void EventLogTask::AMSUsageCapture()
{
    auto cmdCatcher = GetCmdCatcher();
    std::vector<std::string> cmd = {"hidumper", "-s", "AbilityManagerService", "-a", "-a"};
    cmdCatcher->AddCmd(cmd);
}

void EventLogTask::CpuUsageCapture()
{
    auto cmdCatcher = GetCmdCatcher();
    std::vector<std::string> cmd = {"hidumper", "--cpuusage"};
    cmdCatcher->AddCmd(cmd);
}

void EventLogTask::MemoryUsageCapture()
{
    auto cmdCatcher = GetCmdCatcher();
    std::vector<std::string> cmd = {"hidumper", "--mem", std::to_string(cmdCatcher->GetPid())};
    cmdCatcher->AddCmd(cmd);
}

void EventLogTask::PMSUsageCapture()
{
    auto cmdCatcher = GetCmdCatcher();
    std::vector<std::string> cmd = {"hidumper", "-s", "PowerManagerService", "-a", "-s"};
    cmdCatcher->AddCmd(cmd);
    std::vector<std::string> cmd1 = {"hidumper", "-s", "DisplayPowerManagerService"};
    cmdCatcher->AddCmd(cmd1);
}
} // namespace HiviewDFX
} // namespace OHOS