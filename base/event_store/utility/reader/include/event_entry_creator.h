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

#ifndef HIVIEW_BASE_EVENT_STORE_UTILITY_EVENT_ENTRY_CREATOR_H
#define HIVIEW_BASE_EVENT_STORE_UTILITY_EVENT_ENTRY_CREATOR_H

#include <optional>
#include <string>

#include "base_def.h"
#include "decoded/decoded_event.h"
#include "sys_event_query.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
class EventEntryCreator {
public:
    EventEntryCreator(CommonEventInfo comEventInfo_, DocQuery eventQuery);
    ~EventEntryCreator() = default;
    std::optional<Entry> CreateEntry(uint8_t* content, uint32_t contentSize);

private:
    std::string BuildEventJson(uint8_t* content, uint32_t contentSize);
    std::string ParseEventJsonFromContent(uint8_t* content, uint32_t contentSize);
    bool HasContainsInnerConds(uint8_t* content);
    std::shared_ptr<EventRaw::DecodedEvent> BuildDecodedEvent(uint8_t* content, uint32_t contentSize);
    uint8_t* BuildRawEvent(uint8_t* content, uint32_t contentSize);
    std::optional<EventRaw::HiSysEventHeader> BuildRawEventHeader(uint8_t* content);
    void ParseEventHeaderFromContent(uint8_t* content);
    bool HasContainsExtraConds(std::shared_ptr<EventRaw::DecodedEvent> decodedEvent);

private:
    CommonEventInfo comEventInfo_;
    DocEventHeader eventHeader_;
    DocQuery eventQuery_;
}; // EventEntryCreator
} // EventStore
} // HiviewDFX
} // OHOS
#endif // HIVIEW_BASE_EVENT_STORE_UTILITY_EVENT_ENTRY_CREATOR_H
