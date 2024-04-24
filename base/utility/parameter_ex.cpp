/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "parameter_ex.h"
#include <cstdint>

#include "parameter.h"
#include "parameters.h"

namespace OHOS {
namespace HiviewDFX {
namespace Parameter {
std::string GetString(const std::string& key, const std::string& defaultValue)
{
    return OHOS::system::GetParameter(key, defaultValue);
}

int64_t GetInteger(const std::string& key, const int64_t defaultValue)
{
    return OHOS::system::GetIntParameter(key, defaultValue);
}

uint64_t GetUnsignedInteger(const std::string& key, const uint64_t defaultValue)
{
    return OHOS::system::GetUintParameter(key, defaultValue);
}

bool GetBoolean(const std::string& key, const bool defaultValue)
{
    return OHOS::system::GetBoolParameter(key, defaultValue);
}

bool SetProperty(const std::string& key, const std::string& defaultValue)
{
    return OHOS::system::SetParameter(key, defaultValue);
}

int WaitParamSync(const char *key, const char *value, int timeout)
{
    return WaitParameter(key, value, timeout);
}

int WatchParamChange(const char *keyPrefix, ParameterChgPtr callback, void *context)
{
    return WatchParameter(keyPrefix, callback, context);
}

int RemoveParamWatcher(const char *keyPrefix, ParameterChgPtr callback, void *context)
{
    return RemoveParameterWatcher(keyPrefix, callback, context);
}

std::string GetDisplayVersionStr()
{
    static std::string displayedVersionStr = std::string(GetDisplayVersion());
    return displayedVersionStr;
}

bool IsBetaVersion()
{
    auto versionType = GetString(KEY_HIVIEW_VERSION_TYPE, "unknown");
    return (versionType.find("beta") != std::string::npos);
}

bool IsDeveloperMode()
{
    return GetBoolean(KEY_DEVELOPER_MODE_STATE, false);
}

bool IsUCollectionSwitchOn()
{
    std::string ucollectionState = GetString(HIVIEW_UCOLLECTION_STATE, "false");
    return ucollectionState == "true";
}

bool IsTraceCollectionSwitchOn()
{
    std::string traceCollectionState = GetString(DEVELOP_HIVIEW_TRACE_RECORDER, "false");
    return traceCollectionState == "true";
}

DeviceType GetDeviceType()
{
    std::string deviceType = GetString(KEY_BUILD_CHARACTER, "unknown");
    if (deviceType == "phone" || deviceType == "default") {
        return DeviceType::PHONE;
    } else if (deviceType == "watch") {
        return DeviceType::WATCH;
    } else if (deviceType == "tv") {
        return DeviceType::TV;
    } else if (deviceType == "car") {
        return DeviceType::IVI;
    } else {
        return DeviceType::UNKNOWN;
    }
}
} // namespace Parameter
} // namespace HiviewDFX
} // namespace OHOS
