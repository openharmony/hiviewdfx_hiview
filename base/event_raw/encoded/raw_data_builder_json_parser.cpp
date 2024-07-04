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

#include "encoded/raw_data_builder_json_parser.h"

#include <cinttypes>
#include <functional>
#include <sstream>
#include <vector>
#include <unordered_map>

namespace OHOS {
namespace HiviewDFX {
namespace EventRaw {
namespace {
constexpr int NUM_MIN_CHAR = static_cast<int>('0');
constexpr int NUM_MAX_CHAR = static_cast<int>('9');
constexpr int DOUBLE_QUOTA_CHAR = static_cast<int>('"');
constexpr int COLON_CHAR = static_cast<int>(':');
constexpr int POINT_CHAR = static_cast<int>('.');
constexpr int COMMA_CHAR = static_cast<int>(',');
constexpr int LEFT_BRACE_CHAR = static_cast<int>('{');
constexpr int RIGHT_BRACE_CHAR = static_cast<int>('}');
constexpr int LEFT_BRACKET_CHAR = static_cast<int>('[');
constexpr int RIGHT_BRACKET_CHAR = static_cast<int>(']');
constexpr int MINUS_CHAR = static_cast<int>('-');
constexpr int ESCAPE_CHAR = static_cast<int>('\\');
constexpr int POSITIVE_CHAR = static_cast<int>('+');
constexpr int SICENTIFIC_NOTAITION_CHAR = static_cast<int>('e');

template<typename T>
static void TransStrToType(const std::string& str, T& val)
{
    std::stringstream ss(str);
    ss >> val;
}
}

RawDataBuilderJsonParser::RawDataBuilderJsonParser(const std::string& jsonStr)
{
    jsonStr_ = jsonStr;
    builder_ = std::make_shared<RawDataBuilder>();
    InitNoneStatus();
    InitRunStatus();
    InitKeyParseStatus();
    InitValueParseStatus();
    InitStringParseStatus();
    InitDoubleParseStatus();
    InitIntParseStatus();
    InitArrayParseSatus();
    InitStringItemParseStatus();
    InitDoubleItemParseStatus();
    InitIntItemParseStatus();
    InitEscapeCharParseStatus();
    InitEscapeCharItemParseStatus();
}

void RawDataBuilderJsonParser::InitNoneStatus()
{
    for (int i = 0; i < CHAR_RANGE; ++i) {
        statusTabs_[STATUS_NONE][i] = STATUS_NONE;
    }
    statusTabs_[STATUS_NONE][LEFT_BRACE_CHAR] = STATUS_RUN;
}

void RawDataBuilderJsonParser::InitRunStatus()
{
    for (int i = 0; i < CHAR_RANGE; ++i) {
        statusTabs_[STATUS_RUN][i] = STATUS_RUN;
    }
    statusTabs_[STATUS_RUN][DOUBLE_QUOTA_CHAR] = STATUS_KEY_PARSE;
    statusTabs_[STATUS_RUN][COLON_CHAR] = STATUS_VALUE_PARSE;
    statusTabs_[STATUS_RUN][RIGHT_BRACE_CHAR] = STATUS_NONE;
}

void RawDataBuilderJsonParser::InitKeyParseStatus()
{
    for (int i = 0; i < CHAR_RANGE; ++i) {
        statusTabs_[STATUS_KEY_PARSE][i] = STATUS_KEY_PARSE;
    }
    statusTabs_[STATUS_KEY_PARSE][DOUBLE_QUOTA_CHAR] = STATUS_RUN;
}

void RawDataBuilderJsonParser::InitValueParseStatus()
{
    for (int i = 0; i < CHAR_RANGE; ++i) {
        if ((i >= NUM_MIN_CHAR && i <= NUM_MAX_CHAR) || (i == MINUS_CHAR)) {
            statusTabs_[STATUS_VALUE_PARSE][i] = STATUS_INT_PARSE;
            continue;
        }
        statusTabs_[STATUS_VALUE_PARSE][i] = STATUS_VALUE_PARSE;
    }
    statusTabs_[STATUS_VALUE_PARSE][DOUBLE_QUOTA_CHAR] = STATUS_STRING_PARSE;
    statusTabs_[STATUS_VALUE_PARSE][POINT_CHAR] = STATUS_DOUBLE_PARSE;
    statusTabs_[STATUS_VALUE_PARSE][LEFT_BRACKET_CHAR] = STATUS_ARRAY_PARSE;
}

void RawDataBuilderJsonParser::InitStringParseStatus()
{
    for (int i = 0; i < CHAR_RANGE; ++i) {
        statusTabs_[STATUS_STRING_PARSE][i] = STATUS_STRING_PARSE;
    }
    statusTabs_[STATUS_STRING_PARSE][DOUBLE_QUOTA_CHAR] = STATUS_RUN;
    statusTabs_[STATUS_STRING_PARSE][ESCAPE_CHAR] = STATUS_ESCAPE_CHAR_PARSE;
}

void RawDataBuilderJsonParser::InitDoubleParseStatus()
{
    for (int i = 0; i < CHAR_RANGE; ++i) {
        if (i >= NUM_MIN_CHAR && i <= NUM_MAX_CHAR) {
            statusTabs_[STATUS_DOUBLE_PARSE][i] = STATUS_DOUBLE_PARSE;
            continue;
        }
        statusTabs_[STATUS_DOUBLE_PARSE][i] = STATUS_NONE;
    }
    statusTabs_[STATUS_DOUBLE_PARSE][POSITIVE_CHAR] = STATUS_DOUBLE_PARSE;
    statusTabs_[STATUS_DOUBLE_PARSE][SICENTIFIC_NOTAITION_CHAR] = STATUS_DOUBLE_PARSE;
    statusTabs_[STATUS_DOUBLE_PARSE][COMMA_CHAR] = STATUS_RUN;
}

void RawDataBuilderJsonParser::InitIntParseStatus()
{
    for (int i = 0; i < CHAR_RANGE; ++i) {
        if (i >= NUM_MIN_CHAR && i <= NUM_MAX_CHAR) {
            statusTabs_[STATUS_INT_PARSE][i] = STATUS_INT_PARSE;
            continue;
        }
        statusTabs_[STATUS_INT_PARSE][i] = STATUS_NONE;
    }
    statusTabs_[STATUS_INT_PARSE][POINT_CHAR] = STATUS_DOUBLE_PARSE;
    statusTabs_[STATUS_INT_PARSE][COMMA_CHAR] = STATUS_RUN;
}

void RawDataBuilderJsonParser::InitArrayParseSatus()
{
    for (int i = 0; i < CHAR_RANGE; ++i) {
        if ((i >= NUM_MIN_CHAR && i <= NUM_MAX_CHAR) || (i == MINUS_CHAR)) {
            statusTabs_[STATUS_ARRAY_PARSE][i] = STATUS_INT_ITEM_PARSE;
            continue;
        }
        statusTabs_[STATUS_ARRAY_PARSE][i] = STATUS_ARRAY_PARSE;
    }
    statusTabs_[STATUS_ARRAY_PARSE][DOUBLE_QUOTA_CHAR] = STATUS_STRING_ITEM_PARSE;
    statusTabs_[STATUS_ARRAY_PARSE][POINT_CHAR] = STATUS_DOUBLE_ITEM_PARSE;
    statusTabs_[STATUS_ARRAY_PARSE][RIGHT_BRACKET_CHAR] = STATUS_RUN;
    statusTabs_[STATUS_ARRAY_PARSE][COMMA_CHAR] = STATUS_ARRAY_PARSE;
}

void RawDataBuilderJsonParser::InitStringItemParseStatus()
{
    for (int i = 0; i < CHAR_RANGE; ++i) {
        statusTabs_[STATUS_STRING_ITEM_PARSE][i] = STATUS_STRING_ITEM_PARSE;
    }
    statusTabs_[STATUS_STRING_ITEM_PARSE][DOUBLE_QUOTA_CHAR] = STATUS_ARRAY_PARSE;
    statusTabs_[STATUS_STRING_ITEM_PARSE][COMMA_CHAR] = STATUS_ARRAY_PARSE;
    statusTabs_[STATUS_STRING_ITEM_PARSE][ESCAPE_CHAR] = STATUS_ESCAPE_CHAR_ITEM_PARSE;
}

void RawDataBuilderJsonParser::InitDoubleItemParseStatus()
{
    for (int i = 0; i < CHAR_RANGE; ++i) {
        if (i >= NUM_MIN_CHAR && i <= NUM_MAX_CHAR) {
            statusTabs_[STATUS_DOUBLE_ITEM_PARSE][i] = STATUS_DOUBLE_ITEM_PARSE;
            continue;
        }
        statusTabs_[STATUS_DOUBLE_ITEM_PARSE][i] = STATUS_NONE;
    }
    statusTabs_[STATUS_DOUBLE_ITEM_PARSE][POSITIVE_CHAR] = STATUS_DOUBLE_ITEM_PARSE;
    statusTabs_[STATUS_DOUBLE_ITEM_PARSE][SICENTIFIC_NOTAITION_CHAR] = STATUS_DOUBLE_ITEM_PARSE;
    statusTabs_[STATUS_DOUBLE_ITEM_PARSE][COMMA_CHAR] = STATUS_ARRAY_PARSE;
    statusTabs_[STATUS_DOUBLE_ITEM_PARSE][RIGHT_BRACKET_CHAR] = STATUS_RUN;
}

void RawDataBuilderJsonParser::InitIntItemParseStatus()
{
    for (int i = 0; i < CHAR_RANGE; ++i) {
        if (i >= NUM_MIN_CHAR && i <= NUM_MAX_CHAR) {
            statusTabs_[STATUS_INT_ITEM_PARSE][i] = STATUS_INT_ITEM_PARSE;
            continue;
        }
        statusTabs_[STATUS_INT_ITEM_PARSE][i] = STATUS_NONE;
    }
    statusTabs_[STATUS_INT_ITEM_PARSE][COMMA_CHAR] = STATUS_ARRAY_PARSE;
    statusTabs_[STATUS_INT_ITEM_PARSE][RIGHT_BRACKET_CHAR] = STATUS_RUN;
    statusTabs_[STATUS_INT_ITEM_PARSE][POINT_CHAR] = STATUS_DOUBLE_ITEM_PARSE;
}

void RawDataBuilderJsonParser::InitEscapeCharParseStatus()
{
    for (int i = 0; i < CHAR_RANGE; ++i) {
        statusTabs_[STATUS_ESCAPE_CHAR_PARSE][i] = STATUS_STRING_PARSE;
    }
    statusTabs_[STATUS_ESCAPE_CHAR_PARSE][ESCAPE_CHAR] = STATUS_ESCAPE_CHAR_PARSE;
}

void RawDataBuilderJsonParser::InitEscapeCharItemParseStatus()
{
    for (int i = 0; i < CHAR_RANGE; ++i) {
        statusTabs_[STATUS_ESCAPE_CHAR_ITEM_PARSE][i] = STATUS_STRING_ITEM_PARSE;
    }
    statusTabs_[STATUS_ESCAPE_CHAR_ITEM_PARSE][ESCAPE_CHAR] = STATUS_ESCAPE_CHAR_ITEM_PARSE;
}

void RawDataBuilderJsonParser::HandleStatusNone()
{
    AppendValueToBuilder();
}

void RawDataBuilderJsonParser::HandleStatusKeyParse()
{
    if (lastStatus_ == STATUS_KEY_PARSE) {
        key_.append(1, charactor_);
        return;
    }
    AppendValueToBuilder();
    key_.clear();
}

void RawDataBuilderJsonParser::HandleStatusRun()
{
    if (lastStatus_ == STATUS_STRING_ITEM_PARSE ||
        lastStatus_ == STATUS_DOUBLE_ITEM_PARSE ||
        lastStatus_ == STATUS_INT_ITEM_PARSE) {
        values_.emplace_back(value_);
        value_.clear();
    }
    if (lastStatus_ == STATUS_STRING_PARSE) { // special for parsing empty string value
        lastValueParseStatus_ = lastStatus_;
    }
    if (lastStatus_ == STATUS_ARRAY_PARSE && (lastValueParseStatus_ != STATUS_STRING_ITEM_PARSE)) { // "KEY":[]
        lastValueParseStatus_ = STATUS_INT_ITEM_PARSE;
    }
}

void RawDataBuilderJsonParser::HandleStatusValueParse()
{
    value_.clear();
    if (lastStatus_ == STATUS_RUN) {
        values_.clear();
    }
    if (lastStatus_ == STATUS_STRING_ITEM_PARSE ||
        lastStatus_ == STATUS_DOUBLE_ITEM_PARSE ||
        lastStatus_ == STATUS_INT_ITEM_PARSE) {
        values_.emplace_back(value_);
        value_.clear();
    }
}

void RawDataBuilderJsonParser::HandleStatusArrayParse()
{
    if (lastStatus_ == STATUS_RUN) {
        values_.clear();
    }
    if (lastStatus_ == STATUS_STRING_ITEM_PARSE ||
        lastStatus_ == STATUS_DOUBLE_ITEM_PARSE ||
        lastStatus_ == STATUS_INT_ITEM_PARSE) {
        values_.emplace_back(value_);
        value_.clear();
    }
}

void RawDataBuilderJsonParser::HandleStatusStringParse()
{
    lastValueParseStatus_ = status_;
    if (lastStatus_ != STATUS_STRING_PARSE && lastStatus_ != STATUS_ESCAPE_CHAR_PARSE) {
        value_.clear();
        return;
    }
    value_.append(1, charactor_);
}

void RawDataBuilderJsonParser::HandleStatusStringItemParse()
{
    lastValueParseStatus_ = status_;
    if (lastStatus_ != STATUS_STRING_ITEM_PARSE && lastStatus_ != STATUS_ESCAPE_CHAR_ITEM_PARSE) {
        value_.clear();
        return;
    }
    value_.append(1, charactor_);
}

void RawDataBuilderJsonParser::HandleStatusValueAppend()
{
    value_.append(1, charactor_);
    lastValueParseStatus_ = status_;
}

void RawDataBuilderJsonParser::BuilderAppendStringValue(const std::string& key, const std::string& value)
{
    if (builder_ == nullptr) {
        return;
    }
    builder_->AppendValue(key, value);
}

void RawDataBuilderJsonParser::BuilderAppendIntValue(const std::string& key, const std::string& value)
{
    if (builder_ == nullptr || value.empty()) {
        return;
    }
    if (value.find("-") != std::string::npos) {
        int64_t i64Value = 0;
        TransStrToType(value, i64Value);
        builder_->AppendValue(key, i64Value);
        return;
    }
    uint64_t u64Value = 0;
    TransStrToType(value, u64Value);
    builder_->AppendValue(key, u64Value);
}

void RawDataBuilderJsonParser::BuilderAppendFloatingValue(const std::string& key, const std::string& value)
{
    if (builder_ == nullptr) {
        return;
    }
    double dlValue = 0.0;
    TransStrToType(value, dlValue);
    builder_->AppendValue(key, dlValue);
}

void RawDataBuilderJsonParser::BuilderAppendStringArrayValue(const std::string& key,
    const std::vector<std::string>& values)
{
    if (builder_ == nullptr) {
        return;
    }
    builder_->AppendValue(key, values);
}

void RawDataBuilderJsonParser::BuilderAppendIntArrayValue(const std::string& key,
    const std::vector<std::string>& values)
{
    if (builder_ == nullptr) {
        return;
    }
    if (any_of(values.begin(), values.end(), [] (auto& item) {
        return !item.empty() && item.find(".") != std::string::npos;
    })) {
        BuilderAppendFloatingArrayValue(key, values);
        return;
    }
    if (any_of(values.begin(), values.end(), [] (auto& item) {
        return !item.empty() && item.find("-") != std::string::npos;
    })) {
        std::vector<int64_t> i64Values;
        int64_t i64Value = 0;
        for (auto value : values) {
            if (value.empty()) {
                continue;
            }
            TransStrToType(value, i64Value);
            i64Values.emplace_back(i64Value);
        }
        builder_->AppendValue(key, i64Values);
        return;
    }
    std::vector<uint64_t> u64Values;
    uint64_t u64Value = 0;
    for (auto value : values) {
        TransStrToType(value, u64Value);
        u64Values.emplace_back(u64Value);
    }
    builder_->AppendValue(key, u64Values);
}

void RawDataBuilderJsonParser::BuilderAppendFloatingArrayValue(const std::string& key,
    const std::vector<std::string>& values)
{
    if (builder_ == nullptr) {
        return;
    }
    std::vector<double> dlValues;
    double dlValue = 0.0;
    for (auto value : values) {
        TransStrToType(value, dlValue);
        dlValues.emplace_back(dlValue);
    }
    builder_->AppendValue(key, dlValues);
}

void RawDataBuilderJsonParser::AppendValueToBuilder()
{
    if (key_.empty()) { // ignore any Key-Value with empty key directly
        return;
    }
    std::unordered_map<int, std::function<void(const std::string&, const std::string&)>> valueAppendFuncs = {
        {STATUS_STRING_PARSE, [this] (const std::string& key, const std::string& value) {
                return this->BuilderAppendStringValue(key, value);
            }
        },
        {STATUS_INT_PARSE, [this] (const std::string& key, const std::string& value) {
                return this->BuilderAppendIntValue(key, value);
            }
        },
        {STATUS_DOUBLE_PARSE, [this] (const std::string& key, const std::string& value) {
                return this->BuilderAppendFloatingValue(key, value);
            }
        }
    };
    auto valueIter = valueAppendFuncs.find(lastValueParseStatus_);
    if (valueIter != valueAppendFuncs.end()) {
        valueIter->second(key_, value_);
        return;
    }
    std::unordered_map<int,
        std::function<void(const std::string&, const std::vector<std::string>&)>> arrayValueAppendFuncs = {
        {STATUS_STRING_ITEM_PARSE, [this] (const std::string& key, const std::vector<std::string>& values) {
                return this->BuilderAppendStringArrayValue(key, values);
            }
        },
        {STATUS_INT_ITEM_PARSE, [this] (const std::string& key, const std::vector<std::string>& values) {
                return this->BuilderAppendIntArrayValue(key, values);
            }
        },
        {STATUS_DOUBLE_ITEM_PARSE, [this] (const std::string& key, const std::vector<std::string>& values) {
                return this->BuilderAppendFloatingArrayValue(key, values);
            }
        }
    };
    auto arrayValueIter = arrayValueAppendFuncs.find(lastValueParseStatus_);
    if (arrayValueIter != arrayValueAppendFuncs.end()) {
        arrayValueIter->second(key_, values_);
    }
}

void RawDataBuilderJsonParser::InitParamHandlers(std::unordered_map<int, std::function<void()>>& handlers)
{
    handlers.emplace(STATUS_NONE, [this] () {
        this->HandleStatusNone();
    });
    handlers.emplace(STATUS_KEY_PARSE, [this] () {
        this->HandleStatusKeyParse();
    });
    handlers.emplace(STATUS_RUN, [this] () {
        this->HandleStatusRun();
    });
    handlers.emplace(STATUS_VALUE_PARSE, [this] () {
        this->HandleStatusValueParse();
    });
    handlers.emplace(STATUS_ARRAY_PARSE, [this] () {
        this->HandleStatusArrayParse();
    });
    handlers.emplace(STATUS_STRING_PARSE, [this] () {
        this->HandleStatusStringParse();
    });
    handlers.emplace(STATUS_STRING_ITEM_PARSE, [this] () {
            this->HandleStatusStringItemParse();
        });

    auto statusValueAppendHandler = [this] () {
        this->HandleStatusValueAppend();
    };
    handlers.emplace(STATUS_DOUBLE_PARSE, statusValueAppendHandler);
    handlers.emplace(STATUS_INT_PARSE, statusValueAppendHandler);
    handlers.emplace(STATUS_DOUBLE_ITEM_PARSE, statusValueAppendHandler);
    handlers.emplace(STATUS_INT_ITEM_PARSE, statusValueAppendHandler);
    handlers.emplace(STATUS_ESCAPE_CHAR_PARSE, statusValueAppendHandler);
    handlers.emplace(STATUS_ESCAPE_CHAR_ITEM_PARSE, statusValueAppendHandler);
}

std::shared_ptr<RawDataBuilder> RawDataBuilderJsonParser::Parse()
{
    if (jsonStr_.empty()) {
        return builder_;
    }
    std::unordered_map<int, std::function<void()>> handlers;
    InitParamHandlers(handlers);
    for (auto c : jsonStr_) {
        charactor_ = static_cast<int>(c);
        status_ = statusTabs_[status_][charactor_];
        auto iter = handlers.find(status_);
        if (iter != handlers.end()) {
            iter->second();
        }
        lastStatus_ = status_;
    }
    return builder_;
}
} // namespace EventRaw
} // namespace HiviewDFX
} // namespace OHOS