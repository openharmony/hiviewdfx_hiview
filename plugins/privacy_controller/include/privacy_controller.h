/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef HIVIEW_PLUGINS_PRIVACY_CONTROLLER_INCLUDE_PRIVACY_CONTROLLER_H
#define HIVIEW_PLUGINS_PRIVACY_CONTROLLER_INCLUDE_PRIVACY_CONTROLLER_H

#include <string>

#include "plugin.h"
#include "sys_event.h"

namespace OHOS {
namespace HiviewDFX {
class PrivacyController : public Plugin {
public:
    bool OnEvent(std::shared_ptr<Event>& event) override;
    void OnLoad() override;

private:
    bool IsBundleNameAllow(const std::string& bundleName, const std::string& allowListFile);
    bool IsValidParam(std::shared_ptr<SysEvent>& sysEvent,
        const std::string& paramName, const std::string& allowListFile);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_PRIVACY_CONTROLLER_INCLUDE_PRIVACY_CONTROLLER_H
