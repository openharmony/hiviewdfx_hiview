/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_HIVIEWDFX_SYS_EVENT_QUERY_RULE_H
#define OHOS_HIVIEWDFX_SYS_EVENT_QUERY_RULE_H

#include <vector>

#include "parcel.h"

namespace OHOS {
namespace HiviewDFX {
class SysEventQueryRule : public Parcelable {
public:
    SysEventQueryRule() {};
    SysEventQueryRule(const uint32_t rule, const std::string& domainIn, const std::vector<std::string>& events)
        : ruleType(rule), domain(domainIn), eventList(events) {};
    ~SysEventQueryRule() {};

    bool Marshalling(Parcel& parcel) const override;
    static SysEventQueryRule* Unmarshalling(Parcel& parcel);

    uint32_t ruleType {0};
    std::string domain;
    std::vector<std::string> eventList;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_SYS_EVENT_QUERY_RULE_H