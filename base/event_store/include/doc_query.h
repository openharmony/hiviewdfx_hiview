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
#ifndef HIVIEW_BASE_EVENT_STORE_INCLUDE_DOC_QUERY_H
#define HIVIEW_BASE_EVENT_STORE_INCLUDE_DOC_QUERY_H

#include <string>

#include "base_def.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventRaw {
class DecodedEvent;
} // EventRaw
namespace EventStore {
class Cond;
class FieldValue;

class DocQuery {
public:
    std::pair<std::string, bool> GetOrderField();
    void And(const Cond& cond);
    bool IsContainExtraConds(EventRaw::DecodedEvent& decodedEvent) const;
    bool IsContainInnerConds(uint8_t* content) const;
    std::string ToString() const;

private:
    bool IsContainInnerCond(const DocEventHeader& eventHeader, const Cond& cond) const;
    bool IsContainCond(const Cond& cond, const FieldValue& value) const;
    bool IsInnerCond(const Cond& cond) const;

    std::vector<Cond> innerConds_;
    std::vector<Cond> extraConds_;
}; // DocQuery
} // EventStore
} // HiviewDFX
} // OHOS
#endif // HIVIEW_BASE_EVENT_STORE_INCLUDE_DOC_QUERY_H
