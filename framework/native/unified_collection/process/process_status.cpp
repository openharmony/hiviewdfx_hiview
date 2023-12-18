/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include "process_status.h"

#include <unordered_map>

#include "file_util.h"
#include "logger.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
DEFINE_LOG_TAG("UCollectUtil-ProcessStatus");
namespace {
constexpr uint64_t INVALID_LAST_FOREGROUND_TIME = 0;

bool IsValidProcessId(int32_t pid)
{
    std::string procDir = "/proc/" + std::to_string(pid);
    return FileUtil::IsDirectory(procDir);
}

std::string GetProcessNameFromProcCmdline(int32_t pid)
{
    std::string procCmdlinePath = "/proc/" + std::to_string(pid) + "/cmdline";
    std::string procCmdlineContent = FileUtil::GetFirstLine(procCmdlinePath);
    if (procCmdlineContent.empty()) {
        return "";
    }

    size_t procNameStartPos = 0;
    size_t procNameEndPos = procCmdlineContent.size();
    for (size_t i = 0; i < procCmdlineContent.size(); i++) {
        if (procCmdlineContent[i] == '/') {
            // for the format '/system/bin/hiview' of the cmdline file
            procNameStartPos = i + 1; // 1 for next char
        } else if (procCmdlineContent[i] == '\0') {
            // for the format 'hiview \0 3 \0 hiview' of the cmdline file
            procNameEndPos = i;
            break;
        }
    }
    return procCmdlineContent.substr(procNameStartPos, procNameEndPos - procNameStartPos);
}

std::string GetProcessNameFromProcStat(int32_t pid)
{
    std::string procStatFilePath = "/proc/" + std::to_string(pid) + "/stat";
    std::string procStatFileContent = FileUtil::GetFirstLine(procStatFilePath);
    if (procStatFileContent.empty()) {
        return "";
    }
    // for the format '40 (hiview) I ...'
    auto procNameStartPos = procStatFileContent.find('(') + 1; // 1: for '(' next char
    if (procNameStartPos == std::string::npos) {
        return "";
    }
    auto procNameEndPos = procStatFileContent.find(')');
    if (procNameEndPos == std::string::npos) {
        return "";
    }
    if (procNameEndPos <= procNameStartPos) {
        return "";
    }
    return procStatFileContent.substr(procNameStartPos, procNameEndPos - procNameStartPos);
}
}

std::string ProcessStatus::GetProcessName(int32_t pid)
{
    std::unique_lock<std::mutex> lock(mutex_);
    // the cleanup judgment is triggered each time
    if (NeedClearProcessInfos()) {
        ClearProcessInfos();
    }

    if (processInfos_.find(pid) != processInfos_.end() && !processInfos_[pid].name.empty()) {
        return processInfos_[pid].name;
    }
    std::string procName = GetProcessNameFromProcCmdline(pid);
    if (UpdateProcessName(pid, procName)) {
        return procName;
    }
    procName = GetProcessNameFromProcStat(pid);
    if (UpdateProcessName(pid, procName)) {
        return procName;
    }
    HIVIEW_LOGW("failed to get proc name from pid=%{public}d", pid);
    return "";
}

bool ProcessStatus::NeedClearProcessInfos()
{
    constexpr size_t maxSizeOfProcessNames = 1000;
    return processInfos_.size() > maxSizeOfProcessNames;
}

void ProcessStatus::ClearProcessInfos()
{
    HIVIEW_LOGI("start to clear process cache, size=%{public}zu", processInfos_.size());
    for (auto it = processInfos_.begin(); it != processInfos_.end();) {
        if (!IsValidProcessId(it->first)) {
            it = processInfos_.erase(it);
        } else {
            it++;
        }
    }
    HIVIEW_LOGI("end to clear process cache, size=%{public}zu", processInfos_.size());
}

bool ProcessStatus::UpdateProcessName(int32_t pid, const std::string& procName)
{
    if (procName.empty()) {
        return false;
    }

    if (processInfos_.find(pid) != processInfos_.end()) {
        processInfos_[pid].name = procName;
        return true;
    }
    processInfos_[pid] = {
        .name = procName,
        .state = BACKGROUND,
        .lastForegroundTime = INVALID_LAST_FOREGROUND_TIME,
    };
    return true;
}

ProcessState ProcessStatus::GetProcessState(int32_t pid)
{
    std::unique_lock<std::mutex> lock(mutex_);
    return (processInfos_.find(pid) != processInfos_.end())
        ? processInfos_[pid].state
        : BACKGROUND;
}

uint64_t ProcessStatus::GetProcessLastForegroundTime(int32_t pid)
{
    std::unique_lock<std::mutex> lock(mutex_);
    return (processInfos_.find(pid) != processInfos_.end())
        ? processInfos_[pid].lastForegroundTime
        : INVALID_LAST_FOREGROUND_TIME;
}

void ProcessStatus::NotifyProcessState(int32_t pid, ProcessState procState)
{
    std::unique_lock<std::mutex> lock(mutex_);
    UpdateProcessState(pid, procState);
}

void ProcessStatus::UpdateProcessState(int32_t pid, ProcessState procState)
{
    HIVIEW_LOGI("update process=%{public}d state=%{public}d", pid, procState);
    switch (procState) {
        case FOREGROUND:
            UpdateProcessForegroundState(pid);
            break;
        case BACKGROUND:
            UpdateProcessBackgroundState(pid);
            break;
        default:
            HIVIEW_LOGW("invalid process=%{public}d state=%{public}d", pid, procState);
    }
}

void ProcessStatus::UpdateProcessForegroundState(int32_t pid)
{
    uint64_t nowTime = TimeUtil::GetMilliseconds();
    if (processInfos_.find(pid) != processInfos_.end()) {
        processInfos_[pid].state = FOREGROUND;
        processInfos_[pid].lastForegroundTime = nowTime;
        return;
    }
    processInfos_[pid] = {
        .name = "",
        .state = FOREGROUND,
        .lastForegroundTime = nowTime,
    };
}

void ProcessStatus::UpdateProcessBackgroundState(int32_t pid)
{
    if (processInfos_.find(pid) != processInfos_.end()) {
        // last foreground time needs to be updated when the foreground status is switched to the background
        if (processInfos_[pid].state == FOREGROUND) {
            processInfos_[pid].lastForegroundTime = TimeUtil::GetMilliseconds();
        }
        processInfos_[pid].state = BACKGROUND;
        return;
    }
    processInfos_[pid] = {
        .name = "",
        .state = BACKGROUND,
        .lastForegroundTime = INVALID_LAST_FOREGROUND_TIME,
    };
}
} // UCollectUtil
}  // namespace HiviewDFX
}  // namespace OHOS
