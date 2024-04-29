/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "uc_system_ability_listener.h"

#include "hiview_logger.h"
#include "system_ability_definition.h"
#include "uc_observer_mgr.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-UnifiedCollector");

void UcSystemAbilityListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    if (systemAbilityId != APP_MGR_SERVICE_ID) {
        return;
    }
    HIVIEW_LOGI("APP_MGR_SERVICE has added");
    UcObserverManager::GetInstance().RegisterAppObserver();
    UcObserverManager::GetInstance().RegisterRenderObserver();
}

void UcSystemAbilityListener::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    if (systemAbilityId != APP_MGR_SERVICE_ID) {
        return;
    }
    // app mgr service has been removed and does not need to unregister.
    HIVIEW_LOGI("APP_MGR_SERVICE has removed");
}
}  // namespace HiviewDFX
}  // namespace OHOS
