/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_DECORATOR_H
#define HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_DECORATOR_H

#include <map>
#include <mutex>
#include <string>
#include <unordered_map>

#include "collect_result.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
const std::string UC_STAT_LOG_PATH = "data/log/hiview/unified_collection/ucollection_stat_detail.log";
const std::string UC_SEPARATOR = "::";

struct StatInfo {
    std::string name;
    uint32_t totalCall = 0;
    uint32_t failCall = 0;
    uint64_t avgLatency = 0;
    uint64_t maxLatency = 0;
    uint64_t totalTimeSpend = 0;

    std::string ToString() const
    {
        std::string str;
        str.append(name).append(" ")
        .append(std::to_string(totalCall)).append(" ")
        .append(std::to_string(failCall)).append(" ")
        .append(std::to_string(avgLatency)).append(" ")
        .append(std::to_string(maxLatency)).append(" ")
        .append(std::to_string(totalTimeSpend));
        return str;
    }
};

class StatInfoWrapper {
public:
    void UpdateStatInfo(uint64_t startTime, uint64_t endTime, const std::string& funcName, bool isCallSucc);
    std::map<std::string, StatInfo> GetStatInfo();
    void ResetStatInfo();

private:
    std::map<std::string, StatInfo> statInfos_;
    std::mutex mutex_;
};

class UCDecorator {
public:
    UCDecorator() {};
    virtual ~UCDecorator() = default;

public:
    static void WriteLinesToFile(const std::vector<std::string>& stats, bool addBlankLine);
};
} // namespace UCollectUtil
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_DECORATOR_H
