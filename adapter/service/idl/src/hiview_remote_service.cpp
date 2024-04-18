/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "hiview_remote_service.h"

#include "logger.h"

#include "system_ability_definition.h"
#include "iservice_registry.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiViewRemoteService");
namespace RemoteService {
namespace {
sptr<IRemoteObject> hiviewServiceAbilityProxy_;
sptr<IRemoteObject::DeathRecipient> deathRecipient_;
std::mutex proxyMutex_;
}

class HiviewServiceDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    HiviewServiceDeathRecipient() {};
    ~HiviewServiceDeathRecipient() = default;
    DISALLOW_COPY_AND_MOVE(HiviewServiceDeathRecipient);

    void OnRemoteDied(const wptr<IRemoteObject>& remote)
    {
    }
};

sptr<IRemoteObject> GetHiViewRemoteService()
{
    std::lock_guard<std::mutex> proxyGuard(proxyMutex_);
    if (hiviewServiceAbilityProxy_ != nullptr) {
        return hiviewServiceAbilityProxy_;
    }
    HIVIEW_LOGI("refresh remote service instance.");
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        return nullptr;
    }
    hiviewServiceAbilityProxy_ = abilityManager->CheckSystemAbility(DFX_SYS_HIVIEW_ABILITY_ID);
    if (hiviewServiceAbilityProxy_ == nullptr) {
        HIVIEW_LOGE("get hiview ability failed.");
        return nullptr;
    }
    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new HiviewServiceDeathRecipient());
    if (deathRecipient_ == nullptr) {
        HIVIEW_LOGE("create service deathrecipient failed.");
        hiviewServiceAbilityProxy_ = nullptr;
        return nullptr;
    }
    hiviewServiceAbilityProxy_->AddDeathRecipient(deathRecipient_);
    return hiviewServiceAbilityProxy_;
}
} // namespace RemoteService
} // namespace HiviewDFX
} // namespace OHOS