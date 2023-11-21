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
#ifndef INTERFACES_INNER_API_UNIFIED_COLLECTION_RESOURCE_CPU_H
#define INTERFACES_INNER_API_UNIFIED_COLLECTION_RESOURCE_CPU_H
#include <cinttypes>
#include <string>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
struct SysCpuLoad {
    double avgLoad1;
    double avgLoad5;
    double avgLoad15;
};

struct CpuUsageInfo {
    std::string cpuId;
    uint32_t userTime = 0;
    uint32_t niceTime = 0;
    uint32_t systemTime = 0;
    uint32_t idleTime = 0;
    uint32_t ioWaitTime = 0;
    uint32_t irqTime = 0;
    uint32_t softIrqTime = 0;
};

struct SysCpuUsage {
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    std::vector<CpuUsageInfo> cpuInfos;
};

struct CpuFreq {
    uint32_t cpuId = 0;
    uint32_t curFreq = 0;
    uint32_t minFreq = 0;
    uint32_t maxFreq = 0;
};

struct ProcessCpuStatInfo {
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    int32_t pid = 0;
    uint32_t minFlt = 0;
    uint32_t majFlt = 0;
    double cpuLoad = 0;
    double uCpuUsage = 0;
    double sCpuUsage = 0;
    double cpuUsage = 0;
    std::string procName;
};
} // HiviewDFX
} // OHOS
#endif // INTERFACES_INNER_API_UNIFIED_COLLECTION_RESOURCE_CPU_H