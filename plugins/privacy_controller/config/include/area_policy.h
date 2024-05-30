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
#ifndef HIVIEW_PLUGINS_PRIVACY_CONTROLLER_CONFIG_INCLUDE_AREA_POLICY_H
#define HIVIEW_PLUGINS_PRIVACY_CONTROLLER_CONFIG_INCLUDE_AREA_POLICY_H

#include "cjson_util.h"
#include "sys_event.h"

namespace OHOS {
namespace HiviewDFX {
struct AllowedInfo {
    uint8_t allowLevel = 0;
    uint8_t allowPrivacy = 0;
    bool allowSysUe = true;
    bool allowUe = true;
};

class AreaPolicy {
public:
    AreaPolicy(const std::string& configPath);
    ~AreaPolicy() = default;
    bool IsAllowed(std::shared_ptr<SysEvent> event) const;

private:
    void Parse(const std::string& configPath);
    bool ParseAllowedInfos(const cJSON* config);
    bool ParseAllowedInfo(const cJSON* config, int32_t regionCode);
    bool IsAllowedLevel(std::shared_ptr<SysEvent> event) const;
    bool IsAllowedPrivacy(std::shared_ptr<SysEvent> event) const;
    bool IsAllowedUe(std::shared_ptr<SysEvent> event) const;

private:
    AllowedInfo allowedInfo_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_PRIVACY_CONTROLLER_CONFIG_INCLUDE_AREA_POLICY_H
