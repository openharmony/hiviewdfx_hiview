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
#ifndef HIVIEW_DISPATCH_CONFIG_H
#define HIVIEW_DISPATCH_CONFIG_H

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "defines.h"
#include "json/json.h"

namespace OHOS {
namespace HiviewDFX {
struct DllExport DomainRule
{
    enum FilterType {
        INCLUDE,
        EXCLUDE
    };
    uint8_t filterType;
    std::unordered_set<std::string> eventlist;
    bool FindEvent(const std::string& eventName) const;
};

struct DllExport DispatchRule {
    std::unordered_set<uint8_t> typeList;
    std::unordered_set<std::string> tagList;
    std::unordered_set<std::string> eventList;
    std::unordered_map<std::string, DomainRule> domainRuleMap;
    bool FindEvent(const std::string &domain, const std::string &eventName);
};

class DllExport HiviewRuleParser {
public:
    explicit HiviewRuleParser(const std::string & filePath);
    std::shared_ptr<DispatchRule> getRule();

private:
    std::shared_ptr<DispatchRule> dispatchRule_ = nullptr;
    void ParseEventTypes(const Json::Value& root);
    void ParseTagEvents(const Json::Value& root);
    void ParseEvents(const Json::Value& root);
    void ParseDomainRule(const Json::Value& root);
    void ParseDomains(const Json::Value& json, DomainRule &domainRule);
};
}
}
#endif