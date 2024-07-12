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

#include "privacy_controller.h"

#include "hiview_logger.h"
#include "plugin_factory.h"

namespace OHOS {
namespace HiviewDFX {
REGISTER(PrivacyController);
DEFINE_LOG_TAG("PrivacyController");

void PrivacyController::OnLoad()
{
    InitAreaPolicy();
}

void PrivacyController::InitAreaPolicy()
{
    auto context = GetHiviewContext();
    if (context == nullptr) {
        HIVIEW_LOGW("context is null");
        return;
    }

    std::string configPath = context->GetHiViewDirectory(HiviewContext::DirectoryType::CONFIG_DIRECTORY);
    const std::string configFileName = "area_policy.json";
    areaPolicy_ = std::make_unique<AreaPolicy>(configPath.append(configFileName));
}

bool PrivacyController::OnEvent(std::shared_ptr<Event>& event)
{
    if (event == nullptr) {
        return false;
    }

    if (areaPolicy_ == nullptr) {
        return true;
    }

    auto sysEvent = std::static_pointer_cast<SysEvent>(event);
    if (!areaPolicy_->IsAllowed(sysEvent)) {
        HIVIEW_LOGD("event[%{public}s|%{public}s] is not allowed",
            sysEvent->domain_.c_str(), sysEvent->eventName_.c_str());
        return sysEvent->OnFinish();
    }
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
