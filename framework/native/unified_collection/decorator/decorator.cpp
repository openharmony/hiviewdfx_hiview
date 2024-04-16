/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <fstream>

#include "decorator.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
void StatInfoWrapper::UpdateStatInfo(uint64_t startTime, uint64_t endTime, const std::string& funcName, bool isCallSucc)
{
    std::lock_guard<std::mutex> lock(mutex_);
    uint64_t latency = (endTime - startTime) > 0 ? (endTime - startTime) : 0;
    if (statInfos_.find(funcName) == statInfos_.end()) {
        StatInfo statInfo = {
            .name = funcName,
            .totalCall = 1,
            .failCall = isCallSucc ? 0 : 1,
            .avgLatency = latency,
            .maxLatency = latency,
            .totalTimeSpend = latency,
        };
        statInfos_.insert(std::make_pair(funcName, statInfo));
        return;
    }
    
    StatInfo& statInfo = statInfos_[funcName];
    statInfo.totalCall += 1;
    statInfo.failCall += isCallSucc ? 0 : 1;
    statInfo.totalTimeSpend += latency;
    statInfo.maxLatency = statInfo.maxLatency < latency ? latency : statInfo.maxLatency;
    if (statInfo.totalCall > 0) {
        statInfo.avgLatency = statInfo.totalTimeSpend / statInfo.totalCall;
    }
}

std::map<std::string, StatInfo> StatInfoWrapper::GetStatInfo()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return statInfos_;
}

void StatInfoWrapper::ResetStatInfo()
{
    std::lock_guard<std::mutex> lock(mutex_);
    statInfos_.clear();
}

void UCDecorator::WriteLinesToFile(const std::vector<std::string>& stats, bool addBlankLine)
{
    std::ofstream statFilesStream;
    statFilesStream.open(UC_STAT_LOG_PATH, std::ios::app);
    if (!statFilesStream.is_open()) {
        return;
    }
    for (const auto& record : stats) {
        statFilesStream << record << std::endl;
    }
    if (addBlankLine) {
        statFilesStream << std::endl; // write a blank line to separate content
    }
    statFilesStream.close();
}
} // namespace UCollectUtil
} // namespace HiviewDFX
} // namespace OHOS
