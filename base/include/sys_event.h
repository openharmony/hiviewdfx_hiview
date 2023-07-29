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
namespace EventStore {
class EventCol {
public:
    static std::string DOMAIN;
    static std::string NAME;
    static std::string TYPE;
    static std::string TS;
    static std::string TZ;
    static std::string PID;
    static std::string TID;
    static std::string UID;
    static std::string INFO;
    static std::string LEVEL;
    static std::string SEQ;
    static std::string TAG;
};
}

class SysEventCreator;
class SysEvent : public PipelineEvent {
public:
    SysEvent(const std::string& sender, PipelineEventProducer* handler, std::shared_ptr<EventRaw::RawData> rawData);
    SysEvent(const std::string& sender, PipelineEventProducer* handler, SysEventCreator& sysEventCreator);
    SysEvent(const std::string& sender, PipelineEventProducer* handler, const std::string& jsonStr);
    ~SysEvent();

public:
    void SetTag(const std::string& tag);
    std::string GetTag() const;
    void SetLevel(const std::string& level);
    std::string GetLevel() const;
    int32_t GetPid() const;
    int32_t GetTid() const;
    int32_t GetUid() const;
    int16_t GetTz() const;
    void SetSeq(int64_t seq);
    int64_t GetSeq() const;
    void SetEventSeq(int64_t eventSeq);
    int64_t GetEventSeq() const;
    std::string GetEventValue(const std::string& key);
    int64_t GetEventIntValue(const std::string& key);
    int GetEventType();
    std::string AsJsonStr();
    uint8_t* AsRawData();

public:
    template<typename T>
    void SetEventValue(const std::string& key, T value, bool appendValue = false)
    {
        if (builder_ == nullptr) {
            return;
        }
        if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
            auto param = builder_->GetValue(key);
            std::string paramValue;
            if (appendValue && (param != nullptr) && param->AsString(paramValue)) {
                paramValue = UnescapeJsonStringValue(paramValue);
                paramValue.append(value);
                value = paramValue;
            }
            value = EscapeJsonStringValue(value);
        }
        builder_->AppendValue(key, value);
    }

    template<typename T,
        std::enable_if_t<std::is_same_v<std::decay_t<T>, uint64_t> ||
        std::is_same_v<std::decay_t<T>, std::string>>* = nullptr>
    void SetId(T id)
    {
        if (builder_ == nullptr) {
            return;
        }
        uint64_t eventHash = 0;
        if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
            auto idStream = std::stringstream(id);
            idStream >> eventHash;
        }
        if constexpr (std::is_same_v<std::decay_t<T>, uint64_t>) {
            eventHash = id;
        }
        builder_->AppendId(eventHash);
        rawData_ = builder_->Build(); // update
    }

public:
    int eventType_;
    bool preserve_;

public:
    static std::atomic<uint32_t> totalCount_;
    static std::atomic<int64_t> totalSize_;

private:
    void InitialMembers();
    std::shared_ptr<EventRaw::RawData> TansJsonStrToRawData(const std::string& jsonStr);
    std::string EscapeJsonStringValue(const std::string& src);
    std::string UnescapeJsonStringValue(const std::string& src);

private:
    int64_t seq_;
    int32_t pid_;
    int32_t tid_;
    int32_t uid_;
    int16_t tz_;
    int64_t eventSeq_;
    std::string tag_;
    std::string level_;
    std::shared_ptr<EventRaw::RawDataBuilder> builder_;
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
        if (builder_ == nullptr) {
            return;
        }
        if constexpr (std::is_same_v<std::decay_t<T>, std::vector<std::string>>) {
            std::vector<std::string> transVal;
            for (auto item : value) {
                transVal.emplace_back(EscapeJsonStringValue(item));
            }
            builder_->AppendValue(key, transVal);
            return;
        }
        if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
            value = EscapeJsonStringValue(value);
        }
        builder_->AppendValue(key, value);
    }

public:
    std::shared_ptr<EventRaw::RawData> GetRawData();

private:
    std::string EscapeJsonStringValue(const std::string& src);

private:
    std::shared_ptr<EventRaw::RawDataBuilder> builder_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_BASE_SYS_EVENT_H
