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
#include "area_policy.h"

#include <unordered_map>

#include "hiview_logger.h"
#include "parameter_ex.h"
#include "string_util.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("AreaPolicy");
namespace {
const std::unordered_map<std::string, uint8_t> EVENT_LEVELS = {
    {"DEBUG", 1}, {"INFO", 2}, {"MINOR", 3}, {"MAJOR", 4}, {"CRITICAL", 5}
};
}

AreaPolicy::AreaPolicy(const std::string& configPath)
{
    Parse(configPath);
}

void AreaPolicy::Parse(const std::string& configPath)
{
    auto root = CJsonUtil::ParseJsonRoot(configPath);
    if (root == nullptr) {
        HIVIEW_LOGW("failed to parse config file=%{public}s", configPath.c_str());
        return;
    }
    std::string version = Parameter::GetVersionStr();
    auto config = CJsonUtil::GetArrayValue(root, version);
    if (config == nullptr) {
        HIVIEW_LOGW("failed to parse config file=%{public}s, version=%{public}s",
            configPath.c_str(), version.c_str());
        cJSON_Delete(root);
        return;
    }
    if (!ParseAllowedInfos(config)) {
        HIVIEW_LOGW("failed to parse allowed info, file=%{public}s, version=%{public}s",
            configPath.c_str(), version.c_str());
    }
    cJSON_Delete(root);
}

bool AreaPolicy::ParseAllowedInfos(const cJSON* config)
{
    cJSON* areaConfig = nullptr;
    int32_t regionCode = Parameter::GetRegionCode();
    cJSON_ArrayForEach(areaConfig, config) {
        if (!cJSON_IsObject(areaConfig)) {
            HIVIEW_LOGW("area config is not object");
            return false;
        }

        if (ParseAllowedInfo(areaConfig, regionCode)) {
            HIVIEW_LOGI("succ to parse, code=%{public}d, allowLevel=%{public}d, allowPrivacy=%{public}d, "
                "allowSysUe=%{public}d, allowUe=%{public}d", regionCode, allowedInfo_.allowLevel,
                allowedInfo_.allowPrivacy, allowedInfo_.allowSysUe, allowedInfo_.allowUe);
            return true;
        }
    }
    return false;
}

bool AreaPolicy::ParseAllowedInfo(const cJSON* config, int32_t regionCode)
{
    int32_t codeValue = CJsonUtil::GetIntValue(config, "code");
    if (codeValue != regionCode) {
        return false;
    }

    const std::string allowLevelKey = "allowLevel";
    int32_t allowLevelValue = CJsonUtil::GetIntValue(config, allowLevelKey);
    if (allowLevelValue <= 0) {
        HIVIEW_LOGW("failed to parse key=%{public}s, value=%{public}d",
            allowLevelKey.c_str(), allowLevelValue);
        return false;
    }

    const std::string allowPrivacyKey = "allowPrivacy";
    int32_t allowPrivacyValue = CJsonUtil::GetIntValue(config, allowPrivacyKey);
    if (allowPrivacyValue <= 0) {
        HIVIEW_LOGW("failed to parse key=%{public}s, value=%{public}d",
            allowPrivacyKey.c_str(), allowPrivacyValue);
        return false;
    }

    const std::string allowSysUeKey = "allowSysUe";
    bool allowSysUeValue = true;
    if (!CJsonUtil::GetBoolValue(config, allowSysUeKey, allowSysUeValue)) {
        HIVIEW_LOGW("failed to parse key=%{public}s, value=%{public}d",
            allowSysUeKey.c_str(), allowSysUeValue);
        return false;
    }

    const std::string allowUeKey = "allowUe";
    bool allowUeValue = true;
    if (!CJsonUtil::GetBoolValue(config, allowUeKey, allowUeValue)) {
        HIVIEW_LOGW("failed to parse key=%{public}s, value=%{public}d",
            allowUeKey.c_str(), allowUeValue);
        return false;
    }

    allowedInfo_.allowLevel = static_cast<uint8_t>(allowLevelValue);
    allowedInfo_.allowPrivacy = static_cast<uint8_t>(allowPrivacyValue);
    allowedInfo_.allowSysUe = allowSysUeValue;
    allowedInfo_.allowUe = allowUeValue;
    return true;
}

bool AreaPolicy::IsAllowed(std::shared_ptr<SysEvent> event) const
{
    return IsAllowedLevel(event) && IsAllowedPrivacy(event) && IsAllowedUe(event);
}

bool AreaPolicy::IsAllowedLevel(std::shared_ptr<SysEvent> event) const
{
    std::string levelStr = event->GetLevel();
    if (EVENT_LEVELS.find(levelStr) == EVENT_LEVELS.end()) {
        HIVIEW_LOGD("event level=%{public}s is invalid", levelStr.c_str());
        return false;
    }
    uint8_t level = EVENT_LEVELS.at(levelStr);
    return level >= allowedInfo_.allowLevel;
}

bool AreaPolicy::IsAllowedPrivacy(std::shared_ptr<SysEvent> event) const
{
    return event->GetPrivacy() >= allowedInfo_.allowPrivacy;
}

bool AreaPolicy::IsAllowedUe(std::shared_ptr<SysEvent> event) const
{
    if (StringUtil::EndWith(event->domain_, "_UE")) {
        return allowedInfo_.allowUe;
    } else if (event->eventType_ == 4) { // 4: behavior event
        return allowedInfo_.allowSysUe;
    } else {
        return true;
    }
}
} // namespace HiviewDFX
} // namespace OHOS
