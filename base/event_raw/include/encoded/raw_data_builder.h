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

#ifndef BASE_EVENT_RAW_ENCODE_INCLUDE_RAW_DATA_BUILDER_H
#define BASE_EVENT_RAW_ENCODE_INCLUDE_RAW_DATA_BUILDER_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <functional>

#include "encoded/encoded_param.h"
#include "base/raw_data_base_def.h"
#include "base/raw_data.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventRaw {
class RawDataBuilder {
public:
    RawDataBuilder() {};
    RawDataBuilder(const std::string& domain, const std::string& name, const int eventType);
    ~RawDataBuilder() = default;

public:
    std::shared_ptr<RawData> Build();
    std::shared_ptr<EncodedParam> GetValue(const std::string& key);
    struct HiSysEventHeader& GetHeader();
    struct TraceInfo& GetTraceInfo();
    std::string GetDomain();
    std::string GetName();
    int GetEventType();
    size_t GetParamCnt();
    bool IsBaseInfo(const std::string& key);

public:
    RawDataBuilder& AppendValue(std::shared_ptr<EncodedParam> param);
    RawDataBuilder& AppendDomain(const std::string& domain);
    RawDataBuilder& AppendName(const std::string& name);
    RawDataBuilder& AppendType(const int eventType);
    RawDataBuilder& AppendTimeStamp(const uint64_t timeStamp);
    RawDataBuilder& AppendTimeZone(const std::string& timeZone);
    RawDataBuilder& AppendTimeZone(const uint8_t timeZone);
    RawDataBuilder& AppendUid(const uint32_t uid);
    RawDataBuilder& AppendPid(const uint32_t pid);
    RawDataBuilder& AppendTid(const uint32_t tid);
    RawDataBuilder& AppendId(const uint64_t id);
    RawDataBuilder& AppendId(const std::string& id);
    RawDataBuilder& AppendTraceId(const uint64_t traceId);
    RawDataBuilder& AppendSpanId(const uint32_t spanId);
    RawDataBuilder& AppendPSpanId(const uint32_t pSpanId);
    RawDataBuilder& AppendTraceFlag(const uint8_t traceFlag);
    RawDataBuilder& AppendTraceInfo(const uint64_t traceId, const uint32_t spanId,
        const uint32_t pSpanId, const uint8_t traceFlag);

    template<typename T>
    bool ParseValueByKey(const std::string& key, T& dest)
    {
        if (IsBaseInfo(key)) {
            return GetBaseInfoValueByKey(key, dest);
        }
        return GetValueByKey(key, dest);
    }

    template<typename T>
    RawDataBuilder& AppendValue(const std::string& key, T val)
    {
        if (IsBaseInfo(key)) {
            return AppendBaseInfoValue(key, val);
        }
        if constexpr (std::is_same_v<std::decay_t<T>, std::string> ||
                    std::is_same_v<std::decay_t<T>, const char*>) {
            return AppendValue(std::make_shared<StringEncodedParam>(key, val));
        }
        if constexpr (std::is_same_v<std::decay_t<T>, float> || std::is_same_v<std::decay_t<T>, double>) {
            return AppendValue(std::make_shared<FloatingNumberEncodedParam<double>>(key,
                static_cast<double>(val)));
        }
        if constexpr (std::is_same_v<std::decay_t<T>, bool> || std::is_same_v<std::decay_t<T>, int8_t> ||
            std::is_same_v<std::decay_t<T>, int16_t> || std::is_same_v<std::decay_t<T>, int32_t> ||
            std::is_same_v<std::decay_t<T>, int64_t>) {
            return AppendValue(std::make_shared<SignedVarintEncodedParam<int64_t>>(key,
                static_cast<int64_t>(val)));
        }
        if constexpr (std::is_same_v<std::decay_t<T>, uint8_t> || std::is_same_v<std::decay_t<T>, uint16_t> ||
            std::is_same_v<std::decay_t<T>, uint32_t> || std::is_same_v<std::decay_t<T>, uint64_t>) {
            return AppendValue(std::make_shared<UnsignedVarintEncodedParam<uint64_t>>(key,
                static_cast<uint64_t>(val)));
        }
        return AppendArrayValue(key, val);
    }

private:
    template<typename T>
    RawDataBuilder& UpdateType(const T val)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, std::uint8_t> ||
            std::is_same_v<std::decay_t<T>, std::uint16_t> ||
            std::is_same_v<std::decay_t<T>, std::uint32_t> ||
            std::is_same_v<std::decay_t<T>, uint64_t>) {
            return AppendType(static_cast<int>(val));
        }
        return *this;
    }

    template<typename T>
    RawDataBuilder& UpdateUid(const T val)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, std::uint8_t> ||
            std::is_same_v<std::decay_t<T>, std::uint16_t> ||
            std::is_same_v<std::decay_t<T>, std::uint32_t> ||
            std::is_same_v<std::decay_t<T>, uint64_t>) {
            return AppendUid(static_cast<uint32_t>(val));
        }
        return *this;
    }

    template<typename T>
    RawDataBuilder& UpdatePid(const T val)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, std::uint8_t> ||
            std::is_same_v<std::decay_t<T>, std::uint16_t> ||
            std::is_same_v<std::decay_t<T>, std::uint32_t> ||
            std::is_same_v<std::decay_t<T>, uint64_t>) {
            return AppendPid(static_cast<uint32_t>(val));
        }
        return *this;
    }

    template<typename T>
    RawDataBuilder& UpdateTid(const T val)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, std::uint8_t> ||
            std::is_same_v<std::decay_t<T>, std::uint16_t> ||
            std::is_same_v<std::decay_t<T>, std::uint32_t> ||
            std::is_same_v<std::decay_t<T>, uint64_t>) {
            return AppendTid(static_cast<uint32_t>(val));
        }
        return *this;
    }

    template<typename T>
    RawDataBuilder& UpdateId(const T val)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, std::string> ||
                    std::is_same_v<std::decay_t<T>, const char*>) {
            return AppendId(val);
        }
        if constexpr (std::is_same_v<std::decay_t<T>, uint8_t> ||
            std::is_same_v<std::decay_t<T>, std::uint16_t> ||
            std::is_same_v<std::decay_t<T>, std::uint32_t> ||
            std::is_same_v<std::decay_t<T>, uint64_t>) {
            return AppendId(static_cast<uint64_t>(val));
        }
        return *this;
    }

    template<typename T>
    void TransHexStrToNum(const std::string& hexStr, T& num)
    {
        std::stringstream ss;
        ss << std::hex << hexStr;
        ss >> num >> std::dec;
    }

    template<typename T>
    RawDataBuilder& UpdateTraceId(const T val)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, std::string> ||
                    std::is_same_v<std::decay_t<T>, const char*>) {
            uint64_t traceId = 0;
            TransHexStrToNum(val, traceId);
            return AppendTraceId(traceId);
        }
        if constexpr (std::is_same_v<std::decay_t<T>, uint8_t> ||
            std::is_same_v<std::decay_t<T>, std::uint16_t> ||
            std::is_same_v<std::decay_t<T>, std::uint32_t> ||
            std::is_same_v<std::decay_t<T>, uint64_t>) {
            return AppendTraceId(static_cast<uint64_t>(val));
        }
        return *this;
    }

    template<typename T>
    RawDataBuilder& UpdateSpanId(const T val)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, std::string> ||
                    std::is_same_v<std::decay_t<T>, const char*>) {
            uint32_t spanId = 0;
            TransHexStrToNum(val, spanId);
            return AppendSpanId(spanId);
        }
        if constexpr (std::is_same_v<std::decay_t<T>, uint8_t> ||
            std::is_same_v<std::decay_t<T>, std::uint16_t> ||
            std::is_same_v<std::decay_t<T>, std::uint32_t> ||
            std::is_same_v<std::decay_t<T>, uint64_t>) {
            return AppendSpanId(static_cast<uint32_t>(val));
        }
        return *this;
    }

    template<typename T>
    RawDataBuilder& UpdatePSpanId(const T val)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, std::string> ||
                    std::is_same_v<std::decay_t<T>, const char*>) {
            uint32_t pSpanId = 0;
            TransHexStrToNum(val, pSpanId);
            return AppendPSpanId(pSpanId);
        }
        if constexpr (std::is_same_v<std::decay_t<T>, uint8_t> ||
            std::is_same_v<std::decay_t<T>, std::uint16_t> ||
            std::is_same_v<std::decay_t<T>, std::uint32_t> ||
            std::is_same_v<std::decay_t<T>, uint64_t>) {
            return AppendPSpanId(static_cast<uint32_t>(val));
        }
        return *this;
    }

    template<typename T>
    RawDataBuilder& UpdateTraceFlag(const T val)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, uint8_t> ||
            std::is_same_v<std::decay_t<T>, std::uint16_t> ||
            std::is_same_v<std::decay_t<T>, std::uint32_t> ||
            std::is_same_v<std::decay_t<T>, uint64_t>) {
            return AppendTraceFlag(static_cast<uint8_t>(val));
        }
        return *this;
    }

    template<typename T>
    RawDataBuilder& AppendArrayValue(const std::string& key, T val)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, std::vector<std::string>>) {
            return AppendValue(std::make_shared<StringEncodedArrayParam>(key, val));
        }
        if constexpr (std::is_same_v<std::decay_t<T>, std::vector<float>> ||
            std::is_same_v<std::decay_t<T>, std::vector<double>>) {
            std::vector<double> dVector;
            for (auto item : val) {
                dVector.emplace_back(static_cast<double>(item));
            }
            return AppendValue(std::make_shared<FloatingNumberEncodedArrayParam<double>>(key,
                dVector));
        }
        if constexpr (std::is_same_v<std::decay_t<T>, std::vector<bool>> ||
            std::is_same_v<std::decay_t<T>, std::vector<int8_t>> ||
            std::is_same_v<std::decay_t<T>, std::vector<int16_t>> ||
            std::is_same_v<std::decay_t<T>, std::vector<int32_t>> ||
            std::is_same_v<std::decay_t<T>, std::vector<int64_t>>) {
            std::vector<int64_t> i64Vector;
            for (auto item : val) {
                i64Vector.emplace_back(static_cast<int64_t>(item));
            }
            return AppendValue(std::make_shared<SignedVarintEncodedArrayParam<int64_t>>(key,
                i64Vector));
        }
        if constexpr (std::is_same_v<std::decay_t<T>, std::vector<uint8_t>> ||
            std::is_same_v<std::decay_t<T>, std::vector<uint16_t>> ||
            std::is_same_v<std::decay_t<T>, std::vector<uint32_t>> ||
            std::is_same_v<std::decay_t<T>, std::vector<uint64_t>>) {
            std::vector<uint64_t> i64Vector;
            for (auto item : val) {
                i64Vector.emplace_back(static_cast<uint64_t>(item));
            }
            return AppendValue(std::make_shared<UnsignedVarintEncodedArrayParam<uint64_t>>(key,
                i64Vector));
        }
        return *this;
    }

    template<typename T>
    RawDataBuilder& AppendBaseInfoValue(const std::string& key, T val)
    {
        std::unordered_map<std::string, std::function<RawDataBuilder&(T)>> appendFuncs = {
            {BASE_INFO_KEY_DOMAIN, std::bind([this] (T val) -> decltype(auto) {
                    if constexpr (std::is_same_v<std::decay_t<T>, std::string> ||
                        std::is_same_v<std::decay_t<T>, const char*>) {
                        return this->AppendDomain(val);
                    }
                    return *this;
                }, std::placeholders::_1)},
            {BASE_INFO_KEY_NAME, std::bind([this] (T val) -> decltype(auto) {
                    if constexpr (std::is_same_v<std::decay_t<T>, std::string> ||
                        std::is_same_v<std::decay_t<T>, const char*>) {
                        return this->AppendName(val);
                    }
                    return *this;
                }, std::placeholders::_1)},
            {BASE_INFO_KEY_TYPE, std::bind(&RawDataBuilder::UpdateType<T>, this, std::placeholders::_1)},
            {BASE_INFO_KEY_TIME_STAMP, std::bind([this] (T val) -> decltype(auto) {
                    if constexpr (std::is_same_v<std::decay_t<T>, uint64_t>) {
                        return this->AppendTimeStamp(val);
                    }
                    return *this;
                }, std::placeholders::_1)},
            {BASE_INFO_KEY_TIME_ZONE, std::bind([this] (T val) -> decltype(auto) {
                    if constexpr (std::is_same_v<std::decay_t<T>, std::string> ||
                        std::is_same_v<std::decay_t<T>, const char*>) {
                        return this->AppendTimeZone(val);
                    }
                    return *this;
                }, std::placeholders::_1)},
            {BASE_INFO_KEY_ID, std::bind(&RawDataBuilder::UpdateId<T>, this, std::placeholders::_1)},
            {BASE_INFO_KEY_PID, std::bind(&RawDataBuilder::UpdatePid<T>, this, std::placeholders::_1)},
            {BASE_INFO_KEY_TID, std::bind(&RawDataBuilder::UpdateTid<T>, this, std::placeholders::_1)},
            {BASE_INFO_KEY_UID, std::bind(&RawDataBuilder::UpdateUid<T>, this, std::placeholders::_1)},
            {BASE_INFO_KEY_TRACE_ID, std::bind(&RawDataBuilder::UpdateTraceId<T>, this, std::placeholders::_1)},
            {BASE_INFO_KEY_SPAN_ID, std::bind(&RawDataBuilder::UpdateSpanId<T>, this, std::placeholders::_1)},
            {BASE_INFO_KEY_PARENT_SPAN_ID, std::bind(&RawDataBuilder::UpdatePSpanId<T>, this, std::placeholders::_1)},
            {BASE_INFO_KEY_TRACE_FLAG, std::bind(&RawDataBuilder::UpdateTraceFlag<T>, this, std::placeholders::_1)},
        };
        auto iter = appendFuncs.find(key);
        if (iter == appendFuncs.end()) {
            return *this;
        }
        return iter->second(val);
    }

    template<typename T>
    bool GetArrayValueByKey(std::shared_ptr<EncodedParam> encodedParam, T& val)
    {
        std::unordered_map<DataCodedType,
            std::function<bool(std::shared_ptr<EncodedParam>, T&)>> getFuncs = {
            {DataCodedType::UNSIGNED_VARINT_ARRAY, std::bind([] (std::shared_ptr<EncodedParam> param, T& val) {
                    if constexpr (std::is_same_v<std::decay_t<T>, std::vector<uint64_t>>) {
                        param->AsUint64Vec(val);
                        return true;
                    }
                    return false;
                }, std::placeholders::_1, std::placeholders::_2)},
            {DataCodedType::SIGNED_VARINT_ARRAY, std::bind([] (std::shared_ptr<EncodedParam> param, T& val) {
                    if constexpr (std::is_same_v<std::decay_t<T>, std::vector<int64_t>>) {
                        param->AsInt64Vec(val);
                        return true;
                    }
                    return false;
                }, std::placeholders::_1, std::placeholders::_2)},
            {DataCodedType::FLOATING_ARRAY, std::bind([] (std::shared_ptr<EncodedParam> param, T& val) {
                    if constexpr (std::is_same_v<std::decay_t<T>, std::vector<double>>) {
                        param->AsDoubleVec(val);
                        return true;
                    }
                    return false;
                }, std::placeholders::_1, std::placeholders::_2)},
            {DataCodedType::DSTRING_ARRAY, std::bind([] (std::shared_ptr<EncodedParam> param, T& val) {
                    if constexpr (std::is_same_v<std::decay_t<T>, std::vector<std::string>>) {
                        param->AsStringVec(val);
                        return true;
                    }
                    return false;
                }, std::placeholders::_1, std::placeholders::_2)},
        };
        auto iter = getFuncs.find(encodedParam->GetDataCodedType());
        if (iter == getFuncs.end()) {
            return false;
        }
        return iter->second(encodedParam, val);
    }

    template<typename T>
    bool GetValueByKey(const std::string& key, T& val)
    {
        auto encodedParam = GetValue(key);
        if (encodedParam == nullptr) {
            return false;
        }
        std::unordered_map<DataCodedType, std::function<bool(std::shared_ptr<EncodedParam>, T&)>> getFuncs = {
            {DataCodedType::UNSIGNED_VARINT, std::bind([] (std::shared_ptr<EncodedParam> param, T& val) {
                    if constexpr (std::is_same_v<std::decay_t<T>, uint64_t>) {
                        param->AsUint64(val);
                        return true;
                    }
                    return false;
                }, std::placeholders::_1, std::placeholders::_2)},
            {DataCodedType::SIGNED_VARINT, std::bind([] (std::shared_ptr<EncodedParam> param, T& val) {
                    if constexpr (std::is_same_v<std::decay_t<T>, int64_t>) {
                        param->AsInt64(val);
                        return true;
                    }
                    return false;
                }, std::placeholders::_1, std::placeholders::_2)},
            {DataCodedType::FLOATING, std::bind([] (std::shared_ptr<EncodedParam> param, T& val) {
                    if constexpr (std::is_same_v<std::decay_t<T>, double>) {
                        param->AsDouble(val);
                        return true;
                    }
                    return false;
                }, std::placeholders::_1, std::placeholders::_2)},
            {DataCodedType::DSTRING, std::bind([] (std::shared_ptr<EncodedParam> param, T& val) {
                    if constexpr (std::is_same_v<std::decay_t<T>, std::string> ||
                        std::is_same_v<std::decay_t<T>, const char*>) {
                        param->AsString(val);
                        return true;
                    }
                    return false;
                }, std::placeholders::_1, std::placeholders::_2)},
        };
        auto iter = getFuncs.find(encodedParam->GetDataCodedType());
        if (iter == getFuncs.end()) {
            return GetArrayValueByKey(encodedParam, val);
        }
        return iter->second(encodedParam, val);
    }

    template<typename T>
    bool GetBaseInfoValueByKey(const std::string& key, T& val)
    {
        std::unordered_map<std::string, std::function<bool(T&)>> parseFuncs = {
            {BASE_INFO_KEY_DOMAIN, std::bind([this] (T& val) -> bool {
                    return this->ParseValue(val, std::string(header_.domain));
                }, std::placeholders::_1)},
            {BASE_INFO_KEY_NAME, std::bind([this] (T& val) -> bool {
                    return this->ParseValue(val, std::string(header_.name));
                }, std::placeholders::_1)},
            {BASE_INFO_KEY_TYPE, std::bind([this] (T& val) -> bool {
                    return this->ParseValue(val, header_.type);
                }, std::placeholders::_1)},
            {BASE_INFO_KEY_TIME_STAMP, std::bind([this] (T& val) -> bool {
                    return this->ParseValue(val, header_.timestamp);
                }, std::placeholders::_1)},
            {BASE_INFO_KEY_TIME_ZONE, std::bind(&RawDataBuilder::PareTimeZone<T>, this, std::placeholders::_1)},
            {BASE_INFO_KEY_ID, std::bind([this] (T& val) -> bool {
                    return this->ParseValue(val, header_.id);
                }, std::placeholders::_1)},
            {BASE_INFO_KEY_PID, std::bind([this] (T& val) -> bool {
                    return this->ParseValue(val, header_.pid);
                }, std::placeholders::_1)},
            {BASE_INFO_KEY_TID, std::bind([this] (T& val) -> bool {
                    return this->ParseValue(val, header_.tid);
                }, std::placeholders::_1)},
            {BASE_INFO_KEY_UID, std::bind([this] (T& val) -> bool {
                    return this->ParseValue(val, header_.uid);
                }, std::placeholders::_1)},
            {BASE_INFO_KEY_TRACE_ID, std::bind([this] (T& val) -> bool {
                    return this->ParseValue(val, traceInfo_.traceId);
                }, std::placeholders::_1)},
            {BASE_INFO_KEY_SPAN_ID, std::bind([this] (T& val) -> bool {
                    return this->ParseValue(val, traceInfo_.spanId);
                }, std::placeholders::_1)},
            {BASE_INFO_KEY_PARENT_SPAN_ID, std::bind([this] (T& val) -> bool {
                    return this->ParseValue(val, traceInfo_.pSpanId);
                }, std::placeholders::_1)},
            {BASE_INFO_KEY_TRACE_FLAG, std::bind(&RawDataBuilder::PareTraceFlag<T>, this, std::placeholders::_1)},
        };
        auto iter = parseFuncs.find(key);
        if (iter == parseFuncs.end()) {
            return false;
        }
        return iter->second(val);
    }

    template<typename T, typename V>
    bool ParseValue(T& val, V dest)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, std::decay_t<V>>) {
            val = dest;
            return true;
        }
        return false;
    }

    template<typename T>
    bool PareTimeZone(T& val)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, int8_t> ||
            std::is_same_v<std::decay_t<T>, int16_t> ||
            std::is_same_v<std::decay_t<T>, int32_t>) {
            val = static_cast<std::decay_t<T>>(header_.timeZone);
            return true;
        }
        if constexpr (std::is_same_v<std::decay_t<T>, std::string> ||
                    std::is_same_v<std::decay_t<T>, const char*>) {
            val = ParseTimeZone(header_.timeZone);
            return true;
        }
        return false;
    }

    template<typename T>
    bool PareTraceFlag(T& val)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, uint8_t> ||
            std::is_same_v<std::decay_t<T>, uint16_t> ||
            std::is_same_v<std::decay_t<T>, uint32_t>) {
            val = static_cast<std::decay_t<T>>(traceInfo_.traceFlag);
            return true;
        }
        return false;
    }

    bool BuildHeader();
    bool BuildCustomizedParams();

private:
    struct HiSysEventHeader header_ = {
        .domain = {0},
        .name = {0},
        .timestamp = 0,
        .timeZone = 0,
        .uid = 0,
        .pid = 0,
        .tid = 0,
        .id = 0,
        .type = 0,
        .isTraceOpened = 0,
    };
    struct TraceInfo traceInfo_ {
        .traceFlag = 0,
        .traceId = 0,
        .spanId = 0,
        .pSpanId = 0,
    };
    std::vector<std::shared_ptr<EncodedParam>> allParams_;
    RawData rawData_;
};
} // namespace EventRaw
} // namespace HiviewDFX
} // namespace OHOS

#endif // BASE_EVENT_RAW_ENCODE_INCLUDE_RAW_DATA_BUILDER_H
