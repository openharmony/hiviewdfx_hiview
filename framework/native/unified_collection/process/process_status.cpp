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

#include "file_util.h"
#include "logger.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
DEFINE_LOG_TAG("UCollectUtil-ProcessStatus");
namespace {
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
    if (NeedClearProcessNames()) {
        ClearProcessNames();
    }

    if (processNames_.find(pid) != processNames_.end()) {
        return processNames_[pid];
    }
    std::string procName = GetProcessNameFromProcCmdline(pid);
    if (!procName.empty()) {
        processNames_[pid] = procName;
        return procName;
    }
    procName = GetProcessNameFromProcStat(pid);
    if (!procName.empty()) {
        processNames_[pid] = procName;
        return procName;
    }
    HIVIEW_LOGW("failed to get proc name from pid=%{public}d", pid);
    return "";
}

bool ProcessStatus::NeedClearProcessNames()
{
    constexpr size_t maxSizeOfProcessNames = 1000;
    return processNames_.size() > maxSizeOfProcessNames;
}

void ProcessStatus::ClearProcessNames()
{
    HIVIEW_LOGI("start to clear process name, size=%{public}zu", processNames_.size());
    for (auto it = processNames_.begin(); it != processNames_.end();) {
        if (!IsValidProcessId(it->first)) {
            processNames_.erase(it++);
        } else {
            it++;
        }
    }
    HIVIEW_LOGI("end to clear process name, size=%{public}zu", processNames_.size());
}
} // UCollectUtil
}  // namespace HiviewDFX
}  // namespace OHOS
