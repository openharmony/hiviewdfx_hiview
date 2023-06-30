/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#ifndef HIVIEW_BASE_DEFINES_H
#define HIVIEW_BASE_DEFINES_H

#include <unordered_set>
#include <unordered_map>

#ifndef __UNUSED

#if defined(_MSC_VER)
#define __UNUSED // Note: actually gcc seems to also supports this syntax.
#else
#if defined(__GNUC__)
#define __UNUSED __attribute__ ((__unused__))
#endif
#endif

#endif
#ifdef _WIN32
#define DllExport __declspec (dllexport)
#else
#define DllExport
#endif // _WIN32

namespace OHOS {
namespace HiviewDFX {
struct DllExport DomainRule {
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
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIVIEW_BASE_DEFINES_H