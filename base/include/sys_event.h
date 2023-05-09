/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef HIVIEW_BASE_SYS_EVENT_H
#define HIVIEW_BASE_SYS_EVENT_H

#include <atomic>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "encoded/encoded_param.h"
#include "decoded/decoded_event.h"
#include "pipeline.h"
#include "encoded/raw_data_builder.h"
#include "base/raw_data.h"

namespace OHOS {
namespace HiviewDFX {
class SysEventCreator;
class SysEvent : public PipelineEvent {
public:
    SysEvent(const std::string& sender, PipelineEventProducer* handler, std::shared_ptr<EventRaw::RawData> rawData);
    SysEvent(const std::string& sender, PipelineEventProducer* handler, SysEventCreator& sysEventCreator);
    SysEvent(const std::string& sender, PipelineEventProducer* handler, const std::string& jsonStr);
    ~SysEvent();

public:
    int32_t GetPid() const;
    int32_t GetTid() const;
    int32_t GetUid() const;
    int16_t GetTz() const;
    void SetSeq(int64_t seq);
    int64_t GetSeq() const;
    int64_t GetEventSeq() const;
    std::string GetEventValue(const std::string& key);
    uint64_t GetEventIntValue(const std::string& key);
    int GetEventType();
    std::string AsJsonStr();
    uint8_t* AsRawData();

public:
    template<typename T>
    void SetEventValue(const std::string& key, T value, bool appendValue = false)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
            auto param = rawDataBuilder_.GetValue(key);
            std::string paramValue;
            if (appendValue && (param != nullptr) && param->AsString(paramValue)) {
                paramValue = UnescapeJsonStringValue(paramValue);
                paramValue.append(value);
                value = paramValue;
            }
            value = EscapeJsonStringValue(value);
        }
        rawDataBuilder_.AppendValue(key, value);
    }

    template<typename T,
        std::enable_if_t<std::is_same_v<std::decay_t<T>, uint64_t> ||
        std::is_same_v<std::decay_t<T>, std::string>>* = nullptr>
    void SetId(T id)
    {
        uint64_t eventHash = 0;
        if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
            auto idStream = std::stringstream(id);
            idStream >> eventHash;
        }
        if constexpr (std::is_same_v<std::decay_t<T>, uint64_t>) {
            eventHash = id;
        }
        rawDataBuilder_.AppendId(eventHash);
    }

public:
    std::string tag_;
    int eventType_;
    bool preserve_;

public:
    static std::atomic<uint32_t> totalCount_;
    static std::atomic<int64_t> totalSize_;

private:
    void InitialMember();
    void InitEventBuilder(std::shared_ptr<EventRaw::RawData> rawData, EventRaw::RawDataBuilder& builder);
    void InitEventBuilderValueParams(std::vector<std::shared_ptr<EventRaw::DecodedParam>> params,
        EventRaw::RawDataBuilder& builder);
    void InitEventBuilderArrayValueParams(std::vector<std::shared_ptr<EventRaw::DecodedParam>> params,
        EventRaw::RawDataBuilder& builder);
    std::shared_ptr<EventRaw::RawData> TansJsonStrToRawData(const std::string& jsonStr);
    std::string EscapeJsonStringValue(const std::string& src);
    std::string UnescapeJsonStringValue(const std::string& src);

private:
    int64_t seq_;
    int32_t pid_;
    int32_t tid_;
    int32_t uid_;
    int16_t tz_;
    int64_t eventSeq_ = 0;
    EventRaw::RawDataBuilder rawDataBuilder_;
};

class SysEventCreator {
public:
    enum EventType {
        FAULT     = 1,    // system fault event
        STATISTIC = 2,    // system statistic event
        SECURITY  = 3,    // system security event
        BEHAVIOR  = 4     // system behavior event
    };

public:
    SysEventCreator(const std::string &domain, const std::string &eventName, EventType type);

public:
    template<typename T>
    void SetKeyValue(const std::string& key, T value)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, std::vector<std::string>>) {
            std::vector<std::string> transVal;
            for (auto item : value) {
                transVal.emplace_back(EscapeJsonStringValue(item));
            }
            rawDataBuilder_.AppendValue(key, transVal);
            return;
        }
        if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
            value = EscapeJsonStringValue(value);
        }
        rawDataBuilder_.AppendValue(key, value);
    }

public:
    std::shared_ptr<EventRaw::RawData> GetRawData();

private:
    std::string EscapeJsonStringValue(const std::string& src);

private:
    EventRaw::RawDataBuilder rawDataBuilder_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_BASE_SYS_EVENT_H
