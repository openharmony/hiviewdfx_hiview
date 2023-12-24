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

#include "network_decorator.h"

#include "network_collector_impl.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
const std::string NET_COLLECTOR_NAME = "NetworkCollector";
StatInfoWrapper NetworkDecorator::statInfoWrapper_;

std::shared_ptr<NetworkCollector> NetworkCollector::Create()
{
    std::shared_ptr<NetworkDecorator> instance = std::make_shared<NetworkDecorator>();
    return instance;
}

NetworkDecorator::NetworkDecorator()
{
    networkCollector_ = std::make_shared<NetworkCollectorImpl>();
}

CollectResult<NetworkRate> NetworkDecorator::CollectRate()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<NetworkRate> result = networkCollector_->CollectRate();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = NET_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<NetworkPackets> NetworkDecorator::CollectSysPackets()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<NetworkPackets> result = networkCollector_->CollectSysPackets();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = NET_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

void NetworkDecorator::SaveStatCommonInfo()
{
    std::map<std::string, StatInfo> statInfo = statInfoWrapper_.GetStatInfo();
    std::vector<std::string> formattedStatInfo;
    for (const auto& record : statInfo) {
        formattedStatInfo.push_back(record.second.ToString());
    }
    WriteLinesToFile(formattedStatInfo, false);
}

void NetworkDecorator::ResetStatInfo()
{
    statInfoWrapper_.ResetStatInfo();
}
} // namespace UCollectUtil
} // namespace HiviewDFX
} // namespace OHOS
