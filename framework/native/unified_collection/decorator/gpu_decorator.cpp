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

#include "gpu_decorator.h"

#include "gpu_collector_impl.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
const std::string GPU_COLLECTOR_NAME = "GpuCollector";
StatInfoWrapper GpuDecorator::statInfoWrapper_;

std::shared_ptr<GpuCollector> GpuCollector::Create()
{
    std::shared_ptr<GpuDecorator> instance = std::make_shared<GpuDecorator>();
    return instance;
}

GpuDecorator::GpuDecorator()
{
    gpuCollector_ = std::make_shared<GpuCollectorImpl>();
}

CollectResult<GpuFreq> GpuDecorator::CollectGpuFrequency()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<GpuFreq> result = gpuCollector_->CollectGpuFrequency();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName = GPU_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}
CollectResult<SysGpuLoad> GpuDecorator::CollectSysGpuLoad()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<SysGpuLoad> result = gpuCollector_->CollectSysGpuLoad();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName = GPU_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

void GpuDecorator::SaveStatCommonInfo()
{
    std::map<std::string, StatInfo> statInfo = statInfoWrapper_.GetStatInfo();
    std::vector<std::string> formattedStatInfo;
    for (const auto& record : statInfo) {
        formattedStatInfo.push_back(record.second.ToString());
    }
    WriteLinesToFile(formattedStatInfo, false);
}

void GpuDecorator::ResetStatInfo()
{
    statInfoWrapper_.ResetStatInfo();
}
} // namespace UCollectUtil
} // namespace HiviewDFX
} // namespace OHOS
