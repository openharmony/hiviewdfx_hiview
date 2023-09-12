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

struct SysCpuUsage {
    double totalUsage;
    std::vector<double> usages;
};

struct CpuFreq {
    int32_t cpuId;
    int32_t curFreq;
    int32_t minFreq;
    int32_t maxFreq;
};

struct CpuFreqStat {
    CpuFreq smallFreq;
    CpuFreq mediumFreq;
    CpuFreq bigFreq;
};

struct ProcessCpuUsage {
    int32_t processId;
    std::string processName;
    double cpuUsage;
};

struct ProcessCpuLoad {
    int32_t processId;
    std::string processName;
    double cpuLoad;
};

struct ProcessCpuStatInfo {
    uint64_t startTime = 0;
    uint64_t endTime = 0;
    int32_t pid = 0;
    std::string procName;
    double cpuLoad = 0;
    double cpuUsage = 0;
};
} // HiviewDFX
} // OHOS
#endif // INTERFACES_INNER_API_UNIFIED_COLLECTION_RESOURCE_CPU_H