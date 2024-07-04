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

#ifndef BASE_EVENT_RAW_ENCODE_INCLUDE_RAW_DATA_BUILDER_JSON_PARSER_H
#define BASE_EVENT_RAW_ENCODE_INCLUDE_RAW_DATA_BUILDER_JSON_PARSER_H

#include "base/raw_data_base_def.h"

#include <string>
#include <vector>

#include "encoded/raw_data_builder.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventRaw {
class RawDataBuilderJsonParser {
public:
    RawDataBuilderJsonParser(const std::string& jsonStr);

public:
    std::shared_ptr<RawDataBuilder> Parse();

private:
    static constexpr int CHAR_RANGE = 256;
    static constexpr int STATUS_NUM = 13;

    static constexpr int STATUS_NONE = 0;
    static constexpr int STATUS_RUN = 1;
    static constexpr int STATUS_KEY_PARSE = 2;
    static constexpr int STATUS_VALUE_PARSE = 3;
    static constexpr int STATUS_STRING_PARSE = 4;
    static constexpr int STATUS_DOUBLE_PARSE = 5;
    static constexpr int STATUS_INT_PARSE = 6;
    static constexpr int STATUS_ARRAY_PARSE = 7;
    static constexpr int STATUS_STRING_ITEM_PARSE = 8;
    static constexpr int STATUS_DOUBLE_ITEM_PARSE = 9;
    static constexpr int STATUS_INT_ITEM_PARSE = 10;
    static constexpr int STATUS_ESCAPE_CHAR_PARSE = 11;
    static constexpr int STATUS_ESCAPE_CHAR_ITEM_PARSE = 12;

private:
    void HandleStatusNone();
    void HandleStatusKeyParse();
    void HandleStatusRun();
    void HandleStatusValueParse();
    void HandleStatusArrayParse();
    void HandleStatusStringParse();
    void HandleStatusStringItemParse();
    void HandleStatusValueAppend();
    void AppendValueToBuilder();
    
private:
    void BuilderAppendStringValue(const std::string& key, const std::string& value);
    void BuilderAppendIntValue(const std::string& key, const std::string& value);
    void BuilderAppendFloatingValue(const std::string& key, const std::string& value);
    void BuilderAppendStringArrayValue(const std::string& key, const std::vector<std::string>& values);
    void BuilderAppendIntArrayValue(const std::string& key, const std::vector<std::string>& values);
    void BuilderAppendFloatingArrayValue(const std::string& key, const std::vector<std::string>& values);

private:
    void InitNoneStatus();
    void InitRunStatus();
    void InitKeyParseStatus();
    void InitValueParseStatus();
    void InitStringParseStatus();
    void InitDoubleParseStatus();
    void InitIntParseStatus();
    void InitArrayParseSatus();
    void InitStringItemParseStatus();
    void InitDoubleItemParseStatus();
    void InitIntItemParseStatus();
    void InitEscapeCharParseStatus();
    void InitEscapeCharItemParseStatus();
    void InitParamHandlers(std::unordered_map<int, std::function<void()>>& handlers);

private:
    std::string jsonStr_;
    std::shared_ptr<RawDataBuilder> builder_ = nullptr;

private:
    int statusTabs_[STATUS_NUM][CHAR_RANGE];
    int status_ = STATUS_NONE;
    int lastStatus_ = STATUS_NONE;
    int lastValueParseStatus_ = STATUS_NONE;
    std::string key_;
    std::string value_;
    std::vector<std::string> values_;
    int charactor_;
};
} // namespace EventRaw
} // namespace HiviewDFX
} // namespace OHOS

#endif // BASE_EVENT_RAW_ENCODE_INCLUDE_RAW_DATA_BUILDER_JSON_PARSER_H