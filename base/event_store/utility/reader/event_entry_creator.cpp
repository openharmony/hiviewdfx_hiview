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
#include "event_entry_creator.h"

#include "base/raw_data_base_def.h"
#include "logger.h"
#include "securec.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
DEFINE_LOG_TAG("HiView-SysEventDocReader");
namespace {
template<typename T>
void AppendJsonValue(std::string& eventJson, const std::string& key, T val)
{
    std::string appendStr;
    appendStr.append(",\"").append(key).append("\":");
    if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
        appendStr.append("\"").append(val).append("\"");
    } else {
        appendStr.append(std::to_string(val));
    }
    eventJson.insert(eventJson.size() - 1, appendStr); // 1 for '}'
}
}
using EventRaw::HISYSEVENT_HEADER_SIZE;

EventEntryCreator::EventEntryCreator(CommonEventInfo commonEventInfo, DocQuery eventQuery)
    : comEventInfo_(commonEventInfo), eventQuery_(eventQuery)
{}

std::optional<Entry> EventEntryCreator::CreateEntry(uint8_t* content, uint32_t contentSize)
{
    auto eventJsonStr = BuildEventJson(content, contentSize);
    if (eventJsonStr.empty()) {
        return std::nullopt;
    }
    Entry eventEntry = {
        .id = eventHeader_.seq,
        .ts = eventHeader_.timestamp,
        .value = eventJsonStr,
    };
    return std::make_optional(eventEntry);
}

std::string EventEntryCreator::BuildEventJson(uint8_t* content, uint32_t contentSize)
{
    auto eventJsonStr = ParseEventJsonFromContent(content, contentSize);
    if (eventJsonStr.empty()) {
        HIVIEW_LOGE("the event json parsed from the content is empty");
        return "";
    }
    if (comEventInfo_.level.empty()) {
        HIVIEW_LOGE("the event level from the CommonEventInfo is empty");
        return "";
    }
    AppendJsonValue(eventJsonStr, EventCol::LEVEL, comEventInfo_.level);
    if (eventHeader_.seq < 0) {
        HIVIEW_LOGE("the event seq from DocEventHeader is invalid, seq=%{public}" PRId64, eventHeader_.seq);
        return "";
    }
    AppendJsonValue(eventJsonStr, EventCol::SEQ, eventHeader_.seq);
    if (!comEventInfo_.tag.empty()) {
        AppendJsonValue(eventJsonStr, EventCol::TAG, comEventInfo_.tag);
    }
    return eventJsonStr;
}

std::string EventEntryCreator::ParseEventJsonFromContent(uint8_t* content, uint32_t contentSize)
{
    if (!HasContainsInnerConds(content)) {
        HIVIEW_LOGD("the content does not match the inner query conditions");
        return "";
    }
    auto decodedEvent = BuildDecodedEvent(content, contentSize);
    if (decodedEvent == nullptr) {
        HIVIEW_LOGE("failed to build DecodedEvent");
        return "";
    }
    if (!HasContainsExtraConds(decodedEvent)) {
        HIVIEW_LOGD("the content does not match the extra query conditions");
        return "";
    }
    return decodedEvent->AsJsonStr();
}

bool EventEntryCreator::HasContainsInnerConds(uint8_t* content)
{
    return eventQuery_.IsContainInnerConds(content);
}

std::shared_ptr<EventRaw::DecodedEvent> EventEntryCreator::BuildDecodedEvent(uint8_t* content, uint32_t contentSize)
{
    uint8_t* rawEvent = BuildRawEvent(content, contentSize);
    if (rawEvent == nullptr) {
        return nullptr;
    }
    return std::make_shared<EventRaw::DecodedEvent>(rawEvent);
}

uint8_t* EventEntryCreator::BuildRawEvent(uint8_t* content, uint32_t contentSize)
{
    uint32_t eventSize = contentSize - DOC_EVENT_HEADER_SIZE + HISYSEVENT_HEADER_SIZE;
    uint8_t* event = new(std::nothrow) uint8_t[eventSize];
    if (event == nullptr) {
        HIVIEW_LOGE("failed to new memory for raw event, size=%{public}u", eventSize);
        return nullptr;
    }
    if (memcpy_s(event, eventSize, reinterpret_cast<char*>(&eventSize), BLOCK_SIZE) != EOK) {
        HIVIEW_LOGE("failed to copy block size to raw event");
        delete[] event;
        return nullptr;
    }
    uint32_t offset = BLOCK_SIZE;
    auto rawEventHeader = BuildRawEventHeader(content);
    if (!rawEventHeader.has_value()) {
        HIVIEW_LOGE("failed to build RawEventHeader");
        delete[] event;
        return nullptr;
    }
    if (memcpy_s(event + offset, eventSize - offset, &(rawEventHeader.value()), HISYSEVENT_HEADER_SIZE) != EOK) {
        HIVIEW_LOGE("failed to copy header to raw event");
        delete[] event;
        return nullptr;
    }
    offset += HISYSEVENT_HEADER_SIZE;
    if (memcpy_s(event + offset, eventSize - offset, content + BLOCK_SIZE + DOC_EVENT_HEADER_SIZE,
        eventSize - offset) != EOK) {
        HIVIEW_LOGE("failed to copy name to raw event");
        delete[] event;
        return nullptr;
    }
    return event;
}

std::optional<EventRaw::HiSysEventHeader> EventEntryCreator::BuildRawEventHeader(uint8_t* content)
{
    ParseEventHeaderFromContent(content);
    EventRaw::HiSysEventHeader rawEventHeader = {
        .domain = {0},
        .name = {0},
        .timestamp = eventHeader_.timestamp,
        .timeZone = eventHeader_.timeZone,
        .uid = eventHeader_.uid,
        .pid = eventHeader_.pid,
        .tid = eventHeader_.tid,
        .id = eventHeader_.id,
        .isTraceOpened = eventHeader_.isTraceOpened,
    };
    if (comEventInfo_.domain.empty()) {
        HIVIEW_LOGI("event domain=%{public}s is empty", comEventInfo_.domain.c_str());
        return std::nullopt;
    }
    if (strcpy_s(rawEventHeader.domain, EventRaw::MAX_DOMAIN_LENGTH + 1, comEventInfo_.domain.c_str()) != EOK) {
        HIVIEW_LOGI("failed to copy domain to the header");
        return std::nullopt;
    }
    if (comEventInfo_.name.empty()) {
        HIVIEW_LOGI("event name=%{public}s is empty", comEventInfo_.name.c_str());
        return std::nullopt;
    }
    if (strcpy_s(rawEventHeader.name, EventRaw::MAX_EVENT_NAME_LENGTH + 1, comEventInfo_.name.c_str()) != EOK) {
        HIVIEW_LOGI("failed to copy name to the header");
        return std::nullopt;
    }
    return std::make_optional(rawEventHeader);
}

void EventEntryCreator::ParseEventHeaderFromContent(uint8_t* content)
{
    eventHeader_ = *(reinterpret_cast<DocEventHeader*>(content + BLOCK_SIZE));
}

bool EventEntryCreator::HasContainsExtraConds(std::shared_ptr<EventRaw::DecodedEvent> decodedEvent)
{
    return eventQuery_.IsContainExtraConds(*decodedEvent);
}
} // EventStore
} // HiviewDFX
} // OHOS