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

#include "cpu_decorator.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
const std::string CPU_COLLECTOR_NAME = "CpuCollector";
StatInfoWrapper CpuDecorator::statInfoWrapper_;

CollectResult<SysCpuLoad> CpuDecorator::CollectSysCpuLoad()
{
    auto task = std::bind(&CpuCollector::CollectSysCpuLoad, cpuCollector_.get());
    return Invoke(task, statInfoWrapper_, CPU_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<SysCpuUsage>  CpuDecorator::CollectSysCpuUsage(bool isNeedUpdate)
{
    auto task = std::bind(&CpuCollector::CollectSysCpuUsage, cpuCollector_.get(), isNeedUpdate);
    return Invoke(task, statInfoWrapper_, CPU_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<double> CpuDecorator::GetSysCpuUsage()
{
    auto task = std::bind(&CpuCollector::GetSysCpuUsage, cpuCollector_.get());
    return Invoke(task, statInfoWrapper_, CPU_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<ProcessCpuStatInfo> CpuDecorator::CollectProcessCpuStatInfo(int32_t pid, bool isNeedUpdate)
{
    auto task = std::bind(&CpuCollector::CollectProcessCpuStatInfo, cpuCollector_.get(), pid, isNeedUpdate);
    return Invoke(task, statInfoWrapper_, CPU_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::vector<CpuFreq>> CpuDecorator::CollectCpuFrequency()
{
    auto task = std::bind(&CpuCollector::CollectCpuFrequency, cpuCollector_.get());
    return Invoke(task, statInfoWrapper_, CPU_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::vector<ProcessCpuStatInfo>> CpuDecorator::CollectProcessCpuStatInfos(bool isNeedUpdate)
{
    auto task = std::bind(&CpuCollector::CollectProcessCpuStatInfos, cpuCollector_.get(), isNeedUpdate);
    return Invoke(task, statInfoWrapper_, CPU_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

void CpuDecorator::SaveStatCommonInfo()
{
    std::map<std::string, StatInfo> statInfo = statInfoWrapper_.GetStatInfo();
    std::vector<std::string> formattedStatInfo;
    for (const auto& record : statInfo) {
        formattedStatInfo.push_back(record.second.ToString());
    }
    WriteLinesToFile(formattedStatInfo, false);
}

void CpuDecorator::ResetStatInfo()
{
    statInfoWrapper_.ResetStatInfo();
}
} // namespace UCollectUtil
} // namespace HiviewDFX
} // namespace OHOS
