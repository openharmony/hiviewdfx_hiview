/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "freeze_common.h"

#include "logger.h"
namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("FreezeDetector");
FreezeCommon::FreezeCommon()
{
    freezeRuleCluster_ = nullptr;
}

FreezeCommon::~FreezeCommon()
{
    freezeRuleCluster_ = nullptr;
}

bool FreezeCommon::Init()
{
    freezeRuleCluster_ = std::make_shared<FreezeRuleCluster>();
    return freezeRuleCluster_->Init();
}

bool FreezeCommon::IsFreezeEvent(const std::string& domain, const std::string& stringId) const
{
    return IsApplicationEvent(domain, stringId) || IsSystemEvent(domain, stringId);
}

bool FreezeCommon::IsApplicationEvent(const std::string& domain, const std::string& stringId) const
{
    if (freezeRuleCluster_ == nullptr) {
        HIVIEW_LOGW("freezeRuleCluster_ == nullptr.");
        return false;
    }
    auto applicationPairs = freezeRuleCluster_->GetApplicationPairs();
    for (auto const &pair : applicationPairs) {
        if (stringId == pair.first && domain == pair.second.first) {
            return true;
        }
    }
    return false;
}

bool FreezeCommon::IsSystemEvent(const std::string& domain, const std::string& stringId) const
{
    if (freezeRuleCluster_ == nullptr) {
        HIVIEW_LOGW("freezeRuleCluster_ == nullptr.");
        return false;
    }
    auto systemPairs = freezeRuleCluster_->GetSystemPairs();
    for (auto const &pair : systemPairs) {
        if (stringId == pair.first && domain == pair.second.first) {
            return true;
        }
    }
    return false;
}

bool FreezeCommon::IsSystemResult(const FreezeResult& result) const
{
    return result.GetId() == SYSTEM_RESULT_ID;
}

bool FreezeCommon::IsApplicationResult(const FreezeResult& result) const
{
    return result.GetId() == APPLICATION_RESULT_ID;
}

bool FreezeCommon::IsBetaVersion() const
{
    return true;
}

std::set<std::string> FreezeCommon::GetPrincipalStringIds() const
{
    std::set<std::string> set;
    if (freezeRuleCluster_ == nullptr) {
        HIVIEW_LOGW("freezeRuleCluster_ == nullptr.");
        return set;
    }
    auto applicationPairs = freezeRuleCluster_->GetApplicationPairs();
    auto systemPairs = freezeRuleCluster_->GetSystemPairs();
    for (auto const &pair : applicationPairs) {
        if (pair.second.second) {
            set.insert(pair.first);
        }
    }
    for (auto const &pair : systemPairs) {
        if (pair.second.second) {
            set.insert(pair.first);
        }
    }

    return set;
}
}  // namespace HiviewDFX
}  // namespace OHOS