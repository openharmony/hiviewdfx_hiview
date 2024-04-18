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
namespace RemoteService {
namespace {
DEFINE_LOG_TAG("HiViewRemoteService");
sptr<IRemoteObject> g_hiviewServiceAbilityProxy = nullptr;
sptr<IRemoteObject::DeathRecipient> g_deathRecipient = nullptr;
std::mutex g_proxyMutex;
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
    std::lock_guard<std::mutex> proxyGuard(g_proxyMutex);
    if (g_hiviewServiceAbilityProxy != nullptr) {
        return g_hiviewServiceAbilityProxy;
    }
    HIVIEW_LOGI("refresh remote service instance.");
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        return nullptr;
    }
    g_hiviewServiceAbilityProxy = abilityManager->CheckSystemAbility(DFX_SYS_HIVIEW_ABILITY_ID);
    if (g_hiviewServiceAbilityProxy == nullptr) {
        HIVIEW_LOGE("get hiview ability failed.");
        return nullptr;
    }
    g_deathRecipient = sptr<IRemoteObject::DeathRecipient>(new HiviewServiceDeathRecipient());
    if (g_deathRecipient == nullptr) {
        HIVIEW_LOGE("create service deathrecipient failed.");
        g_hiviewServiceAbilityProxy = nullptr;
        return nullptr;
    }
    g_hiviewServiceAbilityProxy->AddDeathRecipient(g_deathRecipient);
    return g_hiviewServiceAbilityProxy;
}
} // namespace RemoteService
} // namespace HiviewDFX
} // namespace OHOS