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

#include "cpu_collector_impl.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
const std::string CPU_COLLECTOR_NAME = "CpuCollector";
StatInfoWrapper CpuDecorator::statInfoWrapper_;

std::shared_ptr<CpuCollector> CpuCollector::Create()
{
    static std::shared_ptr<CpuDecorator> instance_ = std::make_shared<CpuDecorator>();
    return instance_;
}

CpuDecorator::CpuDecorator()
{
    cpuCollector_ = std::make_shared<CpuCollectorImpl>();
}

CollectResult<SysCpuLoad> CpuDecorator::CollectSysCpuLoad()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<SysCpuLoad> result = cpuCollector_->CollectSysCpuLoad();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = CPU_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<SysCpuUsage>  CpuDecorator::CollectSysCpuUsage(bool isNeedUpdate)
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<SysCpuUsage> result = cpuCollector_->CollectSysCpuUsage(isNeedUpdate);
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = CPU_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<ProcessCpuStatInfo> CpuDecorator::CollectProcessCpuStatInfo(int32_t pid, bool isNeedUpdate)
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<ProcessCpuStatInfo> result = cpuCollector_->CollectProcessCpuStatInfo(pid, isNeedUpdate);
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = CPU_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::vector<CpuFreq>> CpuDecorator::CollectCpuFrequency()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::vector<CpuFreq>> result = cpuCollector_->CollectCpuFrequency();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = CPU_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::vector<ProcessCpuStatInfo>> CpuDecorator::CollectProcessCpuStatInfos(bool isNeedUpdate)
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::vector<ProcessCpuStatInfo>> result = cpuCollector_->CollectProcessCpuStatInfos(isNeedUpdate);
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = CPU_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
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
