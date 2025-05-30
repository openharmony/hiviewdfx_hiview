/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "type/base_types.h"

namespace OHOS {
namespace HiviewDFX {
class SysEventQueryRule : public Parcelable {
public:
    SysEventQueryRule() {};
    SysEventQueryRule(const std::string& domain, const std::vector<std::string>& events,
        uint32_t ruleType = RuleType::WHOLE_WORD, uint32_t eventType = 0, const std::string& cond = "")
        : domain(domain), eventList(events), ruleType(ruleType), eventType(eventType), condition(cond) {};
    ~SysEventQueryRule() {}

    bool Marshalling(Parcel& parcel) const override;
    static SysEventQueryRule* Unmarshalling(Parcel& parcel);

    std::string domain;
    std::vector<std::string> eventList;
    uint32_t ruleType = RuleType::WHOLE_WORD;
    uint32_t eventType = 0;
    std::string condition;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_SYS_EVENT_QUERY_RULE_H