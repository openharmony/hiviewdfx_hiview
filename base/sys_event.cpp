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
    : PipelineEvent(sender, handler), seq_(0), pid_(0), tid_(0), uid_(0), tz_(0)
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
    what_ = static_cast<uint16_t>(rawDataBuilder_.GetEventType());
    happenTime_ = header.timestamp;
    auto seqParam = rawDataBuilder_.GetValue("seq_");
    if (seqParam != nullptr) {
        seqParam->AsInt64(eventSeq_);
    }
    tz_ = static_cast<int16_t>(header.timeZone);
    pid_ = header.pid;
    tid_ = header.tid;
    uid_ = header.uid;
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
    EventRaw::RawDataBuilderJsonParser parser(jsonStr);
    auto rawDataBuilder = parser.Parse();
    if (rawDataBuilder == nullptr) {
        return nullptr;
    }
    return rawDataBuilder->Build();
}

void SysEvent::SetSeq(int64_t seq)
{
    seq_ = seq;
}

int64_t SysEvent::GetSeq() const
{
    return seq_;
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
    return rawDataBuilder_.GetEventType();
}

std::string SysEvent::AsJsonStr()
{
    auto rawData = rawDataBuilder_.Build(); // update
    if (rawData == nullptr) {
        return "";
    }
    rawData_ = rawData;
    EventRaw::DecodedEvent event(rawData_->GetData());
    return event.AsJsonStr();
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
} // namespace HiviewDFX
} // namespace OHOS
