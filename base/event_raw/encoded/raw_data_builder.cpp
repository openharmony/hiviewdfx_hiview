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

#include "encoded/raw_data_builder.h"

#include <cinttypes>
#include <securec.h>
#include <sstream>
#include <vector>

#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventRaw {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D10, "HiView-RawDataBuilder" };
}

RawDataBuilder::RawDataBuilder(const std::string& domain, const std::string& name, const int eventType)
{
    (void)AppendDomain(domain);
    (void)AppendName(name);
    (void)AppendType(eventType);
}

bool RawDataBuilder::BuildHeader()
{
    if (!rawData_.Append(reinterpret_cast<uint8_t*>(&header_), sizeof(struct HiSysEventHeader))) {
        HiLog::Error(LABEL, "Event header copy failed.");
        return false;
    }
    // append trace info
    if (header_.isTraceOpened == 1 &&
        !rawData_.Append(reinterpret_cast<uint8_t*>(&traceInfo_), sizeof(struct TraceInfo))) {
        HiLog::Error(LABEL, "Trace info copy failed.");
        return false;
    }
    return true;
}

bool RawDataBuilder::BuildCustomizedParams()
{
    for (auto param : allParams_) {
        auto rawData = param->GetRawData();
        if (!rawData_.Append(rawData.GetData(), rawData.GetDataLength())) {
            return false;
        }
    }
    return true;
}

std::shared_ptr<RawData> RawDataBuilder::Build()
{
    // placehold block size
    int32_t blockSize = 0;
    rawData_.Reset();
    if (!rawData_.Append(reinterpret_cast<uint8_t*>(&blockSize), sizeof(int32_t))) {
        HiLog::Error(LABEL, "Block size copy failed.");
        return std::make_shared<RawData>(rawData_);
    }
    if (!BuildHeader()) {
        HiLog::Error(LABEL, "Header of sysevent build failed.");
        return std::make_shared<RawData>(rawData_);
    }
    // append parameter count
    int32_t paramCnt = static_cast<int32_t>(allParams_.size());
    if (!rawData_.Append(reinterpret_cast<uint8_t*>(&paramCnt), sizeof(int32_t))) {
        HiLog::Error(LABEL, "Parameter count copy failed.");
        return std::make_shared<RawData>(rawData_);
    }
    if (!BuildCustomizedParams()) {
        HiLog::Error(LABEL, "Customized paramters of sys event build failed.");
        return std::make_shared<RawData>(rawData_);
    }
    // update block size
    blockSize = static_cast<int32_t>(rawData_.GetDataLength());
    if (!rawData_.Update(reinterpret_cast<uint8_t*>(&blockSize), sizeof(int32_t), 0)) {
        HiLog::Error(LABEL, "Failed to update block size.");
    }
    return std::make_shared<RawData>(rawData_);
}

bool RawDataBuilder::IsBaseInfo(const std::string& key)
{
    std::vector<const std::string> allBaseInfoKeys = {
        BASE_INFO_KEY_DOMAIN, BASE_INFO_KEY_NAME, BASE_INFO_KEY_TYPE, BASE_INFO_KEY_TIME_STAMP,
        BASE_INFO_KEY_TIME_ZONE, BASE_INFO_KEY_ID, BASE_INFO_KEY_PID, BASE_INFO_KEY_TID, BASE_INFO_KEY_UID,
        BASE_INFO_KEY_TRACE_ID, BASE_INFO_KEY_SPAN_ID, BASE_INFO_KEY_PARENT_SPAN_ID, BASE_INFO_KEY_TRACE_FLAG
    };
    return find(allBaseInfoKeys.begin(), allBaseInfoKeys.end(), key) != allBaseInfoKeys.end();
}

RawDataBuilder& RawDataBuilder::AppendDomain(const std::string& domain)
{
    auto ret = memcpy_s(header_.domain, MAX_DOMAIN_LENGTH, domain.c_str(), domain.length());
    if (ret != EOK) {
        HiLog::Error(LABEL, "Failed to copy event domain, ret is %{public}d.", ret);
    }
    auto resetPos = std::min(domain.length(), static_cast<size_t>(MAX_DOMAIN_LENGTH));
    header_.domain[resetPos] = '\0';
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendName(const std::string& name)
{
    auto ret = memcpy_s(header_.name, MAX_EVENT_NAME_LENGTH, name.c_str(), name.length());
    if (ret != EOK) {
        HiLog::Error(LABEL, "Failed to copy event name, ret is %{public}d.", ret);
    }
    auto resetPos = std::min(name.length(), static_cast<size_t>(MAX_EVENT_NAME_LENGTH));
    header_.name[resetPos] = '\0';
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendType(const int eventType)
{
    header_.type = static_cast<uint8_t>(eventType - 1); // header_.type is only 2 bits which must be
                                                       // subtracted 1 in order to avoid data overrflow.
    HiLog::Debug(LABEL, "Encode event type is %{public}d.", eventType);
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendTimeStamp(const uint64_t timestamp)
{
    header_.timestamp = timestamp;
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendTimeZone(const std::string& timeZone)
{
    header_.timeZone = static_cast<uint8_t>(ParseTimeZone(timeZone));
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendTimeZone(const uint8_t timeZone)
{
    header_.timeZone = timeZone;
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendUid(const uint32_t uid)
{
    header_.uid = uid;
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendPid(const uint32_t pid)
{
    header_.pid = pid;
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendTid(const uint32_t tid)
{
    header_.tid = tid;
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendId(const uint64_t id)
{
    header_.id = id;
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendId(const std::string& id)
{
    uint64_t u64Id = 0;
    std::stringstream ss(id);
    ss >> u64Id;
    AppendId(u64Id);
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendTraceId(const uint64_t traceId)
{
    header_.isTraceOpened = 1;
    traceInfo_.traceId = traceId;
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendSpanId(const uint32_t spanId)
{
    header_.isTraceOpened = 1;
    traceInfo_.spanId = spanId;
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendPSpanId(const uint32_t pSpanId)
{
    header_.isTraceOpened = 1;
    traceInfo_.pSpanId = pSpanId;
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendTraceFlag(const uint8_t traceFlag)
{
    header_.isTraceOpened = 1;
    traceInfo_.traceFlag = traceFlag;
    return *this;
}

RawDataBuilder& RawDataBuilder::AppendTraceInfo(const uint64_t traceId, const uint32_t spanId,
    const uint32_t pSpanId, const uint8_t traceFlag)
{
    header_.isTraceOpened = 1; // 1: include trace info, 0: exclude trace info.

    traceInfo_.traceId = traceId;
    traceInfo_.spanId = spanId;
    traceInfo_.pSpanId = pSpanId;
    traceInfo_.traceFlag = traceFlag;

    return *this;
}

RawDataBuilder& RawDataBuilder::AppendValue(std::shared_ptr<EncodedParam> param)
{
    if (param == nullptr || !param->Encode()) {
        return *this;
    }
    auto paramKey = param->GetKey();
    for (auto iter = allParams_.begin(); iter < allParams_.end(); iter++) {
        if ((*iter) == nullptr) {
            continue;
        }
        if ((*iter)->GetKey() == paramKey) {
            allParams_.erase(iter);
            break;
        }
    }
    allParams_.emplace_back(param);
    return *this;
}

std::shared_ptr<EncodedParam> RawDataBuilder::GetValue(const std::string& key)
{
    for (auto iter = allParams_.begin(); iter < allParams_.end(); iter++) {
        if ((*iter) == nullptr) {
            continue;
        }
        if ((*iter)->GetKey() == key) {
            return *iter;
        }
    }
    return nullptr;
}

std::string RawDataBuilder::GetDomain()
{
    return std::string(header_.domain);
}

std::string RawDataBuilder::GetName()
{
    return std::string(header_.name);
}

int RawDataBuilder::GetEventType()
{
    return static_cast<int>(header_.type) + 1; // only 2 bits
}

size_t RawDataBuilder::GetParamCnt()
{
    return allParams_.size();
}

struct HiSysEventHeader& RawDataBuilder::GetHeader()
{
    return header_;
}

struct TraceInfo& RawDataBuilder::GetTraceInfo()
{
    return traceInfo_;
}
} // namespace EventRaw
} // namespace HiviewDFX
} // namespace OHOS