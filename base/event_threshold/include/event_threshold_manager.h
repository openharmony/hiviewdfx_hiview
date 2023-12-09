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

#ifndef OHOS_HIVIEWDFX_EVENT_THRESHOLD_MGR_H
#define OHOS_HIVIEWDFX_EVENT_THRESHOLD_MGR_H

#include <vector>

#include "json/json.h"
#include "sys_event_user.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventThreshold {
class EventThresholdManager {
public:
    static EventThresholdManager& GetInstance();
    size_t GetQueryRuleLimit(const std::string& name, ProcessType type);
    size_t GetDefaultQueryRuleLimit();

private:
    EventThresholdManager();
    ~EventThresholdManager() = default;

private:
    bool ParseThresholdFile(const std::string& path);
    bool ParseSysEventUsers(Json::Value& thresholdsJson, std::vector<SysEventUser>& users);
    bool ParseUser(Json::Value& userJson, SysEventUser& user);
    bool ParseConfigs(Json::Value& configJson, Configs& config);

private:
    std::vector<SysEventUser> sysEventUsers_;
};
} // namespace EventThreshold
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_EVENT_THRESHOLD_MGR_H
