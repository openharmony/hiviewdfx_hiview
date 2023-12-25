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

#ifndef OHOS_HIVIEWDFX_SYS_EVENT_USER_H
#define OHOS_HIVIEWDFX_SYS_EVENT_USER_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
namespace EventThreshold {
enum ProcessType {
    HAP = 1,
};

struct Configs {
    size_t queryRuleLimit;
};

class SysEventUser {
public:
    SysEventUser(const std::string& name, ProcessType type) : name_(name), type_(type), configs_({0}) {}
    SysEventUser() : name_(""), type_(ProcessType::HAP), configs_({0}) {}

public:
    void SetName(std::string& name);
    void GetName(std::string& name);
    void SetProcessType(ProcessType type);
    ProcessType GetProcessType();
    void SetConfigs(Configs& configs);
    void GetConfigs(Configs& configs);

private:
    std::string name_;
    ProcessType type_;
    Configs configs_;
};
} // namespace EventThreshold
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_SYS_EVENT_USER_H
