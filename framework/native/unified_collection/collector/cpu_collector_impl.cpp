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
#include "cpu_collector.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
class CpuCollectorImpl: public CpuCollector {
public:
    CpuCollectorImpl() = default;
    virtual ~CpuCollectorImpl() = default;

public:
    virtual CollectResult<SysCpuLoad> CollectSysCpuLoad() override;
    virtual CollectResult<SysCpuUsage> CollectSysCpuUsage() override;
    virtual CollectResult<ProcessCpuUsage> CollectProcessCpuUsage(int32_t pid) override;
    virtual CollectResult<ProcessCpuLoad> CollectProcessCpuLoad(int32_t pid) override;
    virtual CollectResult<CpuFreqStat> CollectCpuFreqStat() override;
    virtual CollectResult<std::vector<CpuFreq>> CollectCpuFrequency() override;
};

std::shared_ptr<CpuCollector> CpuCollector::Create()
{
    return std::make_shared<CpuCollectorImpl>();
}

CollectResult<SysCpuLoad> CpuCollectorImpl::CollectSysCpuLoad()
{
    CollectResult<SysCpuLoad> result;
    result.retCode = UcError::SUCCESS;
    return result;
}

CollectResult<SysCpuUsage> CpuCollectorImpl::CollectSysCpuUsage()
{
    CollectResult<SysCpuUsage> result;
    result.retCode = UcError::SUCCESS;
    return result;
}

CollectResult<ProcessCpuUsage> CpuCollectorImpl::CollectProcessCpuUsage(int32_t pid)
{
    CollectResult<ProcessCpuUsage> result;
    return result;
}

CollectResult<ProcessCpuLoad> CpuCollectorImpl::CollectProcessCpuLoad(int32_t pid)
{
    CollectResult<ProcessCpuLoad> result;
    return result;
}

CollectResult<CpuFreqStat> CpuCollectorImpl::CollectCpuFreqStat()
{
    CollectResult<CpuFreqStat> result;
    return result;
}

CollectResult<std::vector<CpuFreq>> CpuCollectorImpl::CollectCpuFrequency()
{
    CollectResult<std::vector<CpuFreq>> result;
    return result;
}
} // UCollectUtil
} // HiViewDFX
} // OHOS