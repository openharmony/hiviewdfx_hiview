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
#ifndef INTERFACES_INNER_API_UNIFIED_COLLECTION_CLIENT_CPU_COLLECTOR_H
#define INTERFACES_INNER_API_UNIFIED_COLLECTION_CLIENT_CPU_COLLECTOR_H
#include <memory>
#include <vector>

#include "collect_result.h"
#include "resource/cpu.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectClient {
class CpuCollector {
public:
    CpuCollector() = default;
    virtual ~CpuCollector() = default;

public:
    virtual CollectResult<SysCpuLoad> CollectSysCpuLoad() = 0;
    virtual CollectResult<SysCpuUsage> CollectSysCpuUsage() = 0;
    virtual CollectResult<ProcessCpuUsage> CollectProcessCpuUsage(int32_t pid) = 0;
    virtual CollectResult<ProcessCpuLoad> CollectProcessCpuLoad(int32_t pid) = 0;
    virtual CollectResult<CpuFreqStat> CollectCpuFreqStat() = 0;
    virtual CollectResult<std::vector<CpuFreq>> CollectCpuFrequency() = 0;
    static std::shared_ptr<CpuCollector> Create();
}; // CpuCollector
} // UCollectClient
} // HiviewDFX
} // OHOS
#endif // INTERFACES_INNER_API_UNIFIED_COLLECTION_CLIENT_CPU_COLLECTOR_H