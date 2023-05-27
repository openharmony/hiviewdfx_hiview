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
#include "sys_event.h"

#include <chrono>
#include <functional>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <sys/time.h>

#include "encoded/raw_data_builder_json_parser.h"
#include "string_util.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
std::string EventCol::DOMAIN = "domain_";
std::string EventCol::NAME = "name_";
std::string EventCol::TYPE = "type_";
std::string EventCol::TS = "time_";
std::string EventCol::TZ = "tz_";
std::string EventCol::PID = "pid_";
std::string EventCol::TID = "tid_";
std::string EventCol::UID = "uid_";
std::string EventCol::INFO = "info_";
std::string EventCol::LEVEL = "level_";
std::string EventCol::SEQ = "seq_";
std::string EventCol::TAG = "tag_";
}
namespace {
template<typename T>
void AppendJsonValue(std::string& eventJson, const std::string& key, T val)
{
    if (eventJson.empty()) {
        return;
    }
    std::string findKey = "\"" + key + "\":";
    if (eventJson.find(findKey) != std::string::npos) {
        return;
    }
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
using EventRaw::UnsignedVarintEncodedParam;
using EventRaw::SignedVarintEncodedParam;
using EventRaw::FloatingNumberEncodedParam;
using EventRaw::StringEncodedParam;
using EventRaw::UnsignedVarintEncodedArrayParam;
using EventRaw::SignedVarintEncodedArrayParam;
using EventRaw::FloatingNumberEncodedArrayParam;
using EventRaw::StringEncodedArrayParam;
std::atomic<uint32_t> SysEvent::totalCount_(0);
std::atomic<int64_t> SysEvent::totalSize_(0);

SysEvent::SysEvent(const std::string& sender, PipelineEventProducer* handler,
    std::shared_ptr<EventRaw::RawData> rawData)
    : PipelineEvent(sender, handler), eventType_(0), preserve_(true), seq_(0), pid_(0),
    tid_(0), uid_(0), tz_(0), eventSeq_(-1)
{
    messageType_ = Event::MessageType::SYS_EVENT;
    if (rawData == nullptr) {
        return;
    }
    rawData_ = rawData;
    InitEventBuilder(rawData_, rawDataBuilder_);
    totalCount_.fetch_add(1);
    totalSize_.fetch_add(AsJsonStr().length());
    InitialMember();
}

SysEvent::SysEvent(const std::string& sender, PipelineEventProducer* handler, SysEventCreator& sysEventCreator)
    : SysEvent(sender, handler, sysEventCreator.GetRawData())
{}

SysEvent::SysEvent(const std::string& sender, PipelineEventProducer* handler, const std::string& jsonStr)
    : SysEvent(sender, handler, TansJsonStrToRawData(jsonStr))
{}

SysEvent::~SysEvent()
{
    if (totalCount_ > 0) {
        totalCount_.fetch_sub(1);
    }

    totalSize_.fetch_sub(AsJsonStr().length());
    if (totalSize_ < 0) {
        totalSize_.store(0);
    }
}

void SysEvent::InitialMember()
{
    domain_ = rawDataBuilder_.GetDomain();
    eventName_ = rawDataBuilder_.GetName();
    auto header = rawDataBuilder_.GetHeader();
    eventType_ = rawDataBuilder_.GetEventType();
    what_ = static_cast<uint16_t>(eventType_);
    happenTime_ = header.timestamp;
    if (happenTime_ == 0) {
        auto currentTimeStamp = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
        rawDataBuilder_.AppendTimeStamp(currentTimeStamp);
        happenTime_ = currentTimeStamp;
    }
    auto seqParam = rawDataBuilder_.GetValue("seq_");
    if (seqParam != nullptr) {
        seqParam->AsInt64(eventSeq_);
    }
    tz_ = static_cast<int16_t>(header.timeZone);
    pid_ = static_cast<int32_t>(header.pid);
    tid_ = static_cast<int32_t>(header.tid);
    uid_ = static_cast<int32_t>(header.uid);
    if (header.isTraceOpened == 1) {
        auto traceInfo = rawDataBuilder_.GetTraceInfo();
        traceId_ = StringUtil::ToString(traceInfo.traceId);
        spanId_ =  StringUtil::ToString(traceInfo.spanId);
        parentSpanId_ =  StringUtil::ToString(traceInfo.pSpanId);
        traceFlag_ =  StringUtil::ToString(traceInfo.traceFlag);
    }
}

void SysEvent::InitEventBuilder(std::shared_ptr<EventRaw::RawData> rawData,
    EventRaw::RawDataBuilder& builder)
{
    if (rawData == nullptr) {
        return;
    }
    EventRaw::DecodedEvent event(rawData->GetData());
    auto header = event.GetHeader();
    auto traceInfo = event.GetTraceInfo();
    builder.AppendDomain(header.domain).AppendName(header.name).AppendType(static_cast<int>(header.type) + 1).
        AppendTimeStamp(header.timestamp).AppendTimeZone(header.timeZone).
        AppendUid(header.uid).AppendPid(header.pid).AppendTid(header.tid).AppendId(header.id);
    if (header.isTraceOpened == 1) {
        builder.AppendTraceInfo(traceInfo.traceId, traceInfo.spanId, traceInfo.pSpanId, traceInfo.traceFlag);
    }
    InitEventBuilderValueParams(event.GetAllCustomizedValues(), builder);
}

void SysEvent::InitEventBuilderValueParams(std::vector<std::shared_ptr<EventRaw::DecodedParam>> params,
    EventRaw::RawDataBuilder& builder)
{
    std::unordered_map<EventRaw::DataCodedType,
        std::function<void(std::shared_ptr<EventRaw::DecodedParam>)>> paramFuncs = {
        {EventRaw::DataCodedType::UNSIGNED_VARINT, std::bind(
            [&builder] (std::shared_ptr<EventRaw::DecodedParam> param) {
                if (uint64_t val = 0; param->AsUint64(val)) {
                    builder.AppendValue(std::make_shared<UnsignedVarintEncodedParam<uint64_t>>(param->GetKey(),
                        val));
                }
            }, std::placeholders::_1)},
        {EventRaw::DataCodedType::SIGNED_VARINT, std::bind(
            [&builder] (std::shared_ptr<EventRaw::DecodedParam> param) {
                if (int64_t val = 0; param->AsInt64(val)) {
                    builder.AppendValue(std::make_shared<SignedVarintEncodedParam<int64_t>>(param->GetKey(),
                        val));
                }
            }, std::placeholders::_1)},
        {EventRaw::DataCodedType::FLOATING, std::bind(
            [&builder] (std::shared_ptr<EventRaw::DecodedParam> param) {
                if (double val = 0.0; param->AsDouble(val)) {
                    builder.AppendValue(std::make_shared<FloatingNumberEncodedParam<double>>(param->GetKey(),
                        val));
                }
            }, std::placeholders::_1)},
        {EventRaw::DataCodedType::DSTRING, std::bind(
            [&builder] (std::shared_ptr<EventRaw::DecodedParam> param) {
                if (std::string val; param->AsString(val)) {
                    builder.AppendValue(std::make_shared<StringEncodedParam>(param->GetKey(),
                        val));
                }
            }, std::placeholders::_1)},
    };
    auto iter = paramFuncs.begin();
    for (auto param : params) {
        if (param == nullptr) {
            continue;
        }
        iter = paramFuncs.find(param->GetDataCodedType());
        if (iter == paramFuncs.end()) {
            continue;
        }
        iter->second(param);
    }
    InitEventBuilderArrayValueParams(params, builder);
}

void SysEvent::InitEventBuilderArrayValueParams(std::vector<std::shared_ptr<EventRaw::DecodedParam>> params,
    EventRaw::RawDataBuilder& builder)
{
    std::unordered_map<EventRaw::DataCodedType,
        std::function<void(std::shared_ptr<EventRaw::DecodedParam>)>> paramFuncs = {
        {EventRaw::DataCodedType::UNSIGNED_VARINT_ARRAY, std::bind(
            [&builder] (std::shared_ptr<EventRaw::DecodedParam> param) {
                if (std::vector<uint64_t> vals; param->AsUint64Vec(vals)) {
                    builder.AppendValue(std::make_shared<UnsignedVarintEncodedArrayParam<uint64_t>>(param->GetKey(),
                        vals));
                }
            }, std::placeholders::_1)},
        {EventRaw::DataCodedType::SIGNED_VARINT_ARRAY, std::bind(
            [&builder] (std::shared_ptr<EventRaw::DecodedParam> param) {
                if (std::vector<int64_t> vals; param->AsInt64Vec(vals)) {
                    builder.AppendValue(std::make_shared<SignedVarintEncodedArrayParam<int64_t>>(param->GetKey(),
                        vals));
                }
            }, std::placeholders::_1)},
        {EventRaw::DataCodedType::FLOATING_ARRAY, std::bind(
            [&builder] (std::shared_ptr<EventRaw::DecodedParam> param) {
                if (std::vector<double> vals; param->AsDoubleVec(vals)) {
                    builder.AppendValue(std::make_shared<FloatingNumberEncodedArrayParam<double>>(param->GetKey(),
                        vals));
                }
            }, std::placeholders::_1)},
        {EventRaw::DataCodedType::DSTRING_ARRAY, std::bind(
            [&builder] (std::shared_ptr<EventRaw::DecodedParam> param) {
                if (std::vector<std::string> vals; param->AsStringVec(vals)) {
                    builder.AppendValue(std::make_shared<StringEncodedArrayParam>(param->GetKey(),
                        vals));
                }
            }, std::placeholders::_1)},
    };
    auto iter = paramFuncs.begin();
    for (auto param : params) {
        if (param == nullptr) {
            continue;
        }
        iter = paramFuncs.find(param->GetDataCodedType());
        if (iter == paramFuncs.end()) {
            continue;
        }
        iter->second(param);
    }
}

std::shared_ptr<EventRaw::RawData> SysEvent::TansJsonStrToRawData(const std::string& jsonStr)
{
    auto parser = std::make_unique<EventRaw::RawDataBuilderJsonParser>(jsonStr);
    if (parser == nullptr) {
        return nullptr;
    }
    auto rawDataBuilder = parser->Parse();
    if (rawDataBuilder == nullptr) {
        return nullptr;
    }
    return rawDataBuilder->Build();
}

void SysEvent::SetTag(const std::string& tag)
{
    tag_ = tag;
}

std::string SysEvent::GetTag() const
{
    return tag_;
}

void SysEvent::SetLevel(const std::string& level)
{
    level_ = level;
}

std::string SysEvent::GetLevel() const
{
    return level_;
}

void SysEvent::SetSeq(int64_t seq)
{
    seq_ = seq;
}

int64_t SysEvent::GetSeq() const
{
    return seq_;
}

void SysEvent::SetEventSeq(int64_t eventSeq)
{
    eventSeq_ = eventSeq;
}

int64_t SysEvent::GetEventSeq() const
{
    return eventSeq_;
}

int32_t SysEvent::GetPid() const
{
    return pid_;
}

int32_t SysEvent::GetTid() const
{
    return tid_;
}

int32_t SysEvent::GetUid() const
{
    return uid_;
}

int16_t SysEvent::GetTz() const
{
    return tz_;
}

std::string SysEvent::GetEventValue(const std::string& key)
{
    std::string dest;
    rawDataBuilder_.ParseValueByKey(key, dest);
    return dest;
}

uint64_t SysEvent::GetEventIntValue(const std::string& key)
{
    uint64_t dest;
    rawDataBuilder_.ParseValueByKey(key, dest);
    return dest;
}

int SysEvent::GetEventType()
{
    return eventType_;
}

std::string SysEvent::AsJsonStr()
{
    auto rawData = rawDataBuilder_.Build(); // update
    if (rawData == nullptr) {
        return "";
    }
    rawData_ = rawData;
    EventRaw::DecodedEvent event(rawData_->GetData());

    std::string jsonStr = event.AsJsonStr();
    if (!tag_.empty()) {
        AppendJsonValue(jsonStr, EventStore::EventCol::TAG, tag_);
    }
    if (!level_.empty()) {
        AppendJsonValue(jsonStr, EventStore::EventCol::LEVEL, level_);
    }
    if (eventSeq_ >= 0) {
        AppendJsonValue(jsonStr, EventStore::EventCol::SEQ, eventSeq_);
    }
    return jsonStr;
}

uint8_t* SysEvent::AsRawData()
{
    auto rawData = rawDataBuilder_.Build();
    if (rawData != nullptr) {
        rawData_ = rawData;
        return rawData_->GetData();
    }
    return nullptr;
}

std::string SysEvent::EscapeJsonStringValue(const std::string& src)
{
    return StringUtil::EscapeJsonStringValue(src);
}

std::string SysEvent::UnescapeJsonStringValue(const std::string& src)
{
    return StringUtil::UnescapeJsonStringValue(src);
}

SysEventCreator::SysEventCreator(const std::string& domain, const std::string& eventName,
    SysEventCreator::EventType type)
{
    rawDataBuilder_.AppendDomain(domain).AppendName(eventName).AppendType(static_cast<int>(type)).
        AppendTimeStamp(TimeUtil::GetMilliseconds()).AppendTimeZone(TimeUtil::GetTimeZone()).
        AppendPid(getpid()).AppendTid(gettid()).AppendUid(getuid());
}

std::shared_ptr<EventRaw::RawData> SysEventCreator::GetRawData()
{
    return rawDataBuilder_.Build();
}

std::string SysEventCreator::EscapeJsonStringValue(const std::string& src)
{
    return StringUtil::EscapeJsonStringValue(src);
}
} // namespace HiviewDFX
} // namespace OHOS
