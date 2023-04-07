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

#include "flat_json_parser.h"

#include <algorithm>
#include <sstream>

namespace OHOS {
namespace HiviewDFX {
namespace {
    struct Initializer {
        Initializer()
        {
            FlatJsonParser::Initialize();
        }
    };
}

uint8_t FlatJsonParser::charFilter_[FlatJsonParser::CHAR_RANGE] { 0 };

void FlatJsonParser::AppendStringValue(const std::string& key, const std::string& value)
{
    if (std::any_of(kvList_.begin(), kvList_.end(), [&key] (auto& item) {
        return item.first == key;
    })) {
        return;
    }
    kvList_.emplace_back(key, "\"" + value + "\"");
}

void FlatJsonParser::AppendUInt64Value(const std::string& key, const uint64_t value)
{
    if (std::any_of(kvList_.begin(), kvList_.end(), [&key] (auto& item) {
        return item.first == key;
    })) {
        return;
    }
    std::ostringstream os;
    os << value;
    kvList_.emplace_back(key, os.str());
    os.clear();
}

void FlatJsonParser::Initialize()
{
    for (char c = '0'; c <= '9'; c++) {
        charFilter_[static_cast<uint8_t>(c)] = NUMBER_FLAG;
    }
    charFilter_[static_cast<uint8_t>('-')] = NUMBER_FLAG;
    charFilter_[static_cast<uint8_t>('+')] = NUMBER_FLAG;
    charFilter_[static_cast<uint8_t>('.')] = NUMBER_FLAG;
    charFilter_[static_cast<uint8_t>('"')] = STRING_FLAG;
    charFilter_[static_cast<uint8_t>('{')] = BRACKET_FLAG;
    charFilter_[static_cast<uint8_t>('[')] = BRACKET_FLAG;
}

FlatJsonParser::FlatJsonParser(const std::string& jsonStr)
{
    static Initializer initialize;
    Parse(jsonStr);
}

void FlatJsonParser::Parse(const std::string& jsonStr)
{
    index_ = 0;
    kvList_.clear();
    while (index_ < jsonStr.length()) {
        if (charFilter_[static_cast<uint8_t>(jsonStr[index_])] != STRING_FLAG) {
            ++index_;
            continue;
        }
        std::string key = ParseKey(jsonStr);
        std::string val = ParseValue(jsonStr);
        kvList_.emplace_back(key, val);
    }
}

std::string FlatJsonParser::Print()
{
    std::string jsonStr = "{";
    if (!kvList_.empty()) {
        for (size_t i = 0; i < kvList_.size() - 1; i++) {
            jsonStr += "\"" + kvList_[i].first + "\":" + kvList_[i].second + ",";
        }
        jsonStr += "\"" + kvList_.back().first + "\":" + kvList_.back().second;
    }
    jsonStr += "}";
    return jsonStr;
}

std::string FlatJsonParser::ParseKey(const std::string& jsonStr)
{
    std::string key;
    ++index_; // eat left quotation
    while (index_ < jsonStr.length()) {
        if (charFilter_[static_cast<uint8_t>(jsonStr[index_])] == STRING_FLAG) {
            break;
        }
        key.push_back(jsonStr[index_]);
        ++index_;
    }
    ++index_; // eat right quotation
    return key;
}

std::string FlatJsonParser::ParseValue(const std::string& jsonStr)
{
    std::string value;
    bool valueParsed = false;
    while (index_ < jsonStr.length()) {
        int charCode = static_cast<uint8_t>(jsonStr[index_]);
        switch (charFilter_[charCode]) {
            case BRACKET_FLAG:
                value = ParseBrackets(jsonStr, jsonStr[index_]);
                valueParsed = true;
                break;
            case NUMBER_FLAG:
                value = ParseNumer(jsonStr);
                valueParsed = true;
                break;
            case STRING_FLAG:
                value = ParseString(jsonStr);
                valueParsed = true;
                break;
            default:
                ++index_;
                valueParsed = false;
                break;
        }
        if (valueParsed) {
            break;
        }
    }
    return value;
}

std::string FlatJsonParser::ParseNumer(const std::string& jsonStr)
{
    std::string number;
    while (index_ < jsonStr.length()) {
        if (charFilter_[static_cast<uint8_t>(jsonStr[index_])] != NUMBER_FLAG) {
            break;
        }
        number.push_back(jsonStr[index_]);
        ++index_;
    }
    return number;
}

std::string FlatJsonParser::ParseString(const std::string& jsonStr)
{
    std::string txt;
    txt.push_back(jsonStr[index_++]);
    while (index_ < jsonStr.length()) {
        if (charFilter_[static_cast<uint8_t>(jsonStr[index_])] == STRING_FLAG &&
            jsonStr[index_ - 1] != '\\') {
            break;
        }
        txt.push_back(jsonStr[index_]);
        ++index_;
    }
    txt.push_back(jsonStr[index_++]);
    return txt;
}

std::string FlatJsonParser::ParseBrackets(const std::string& jsonStr, char leftBracket)
{
    std::string val;
    char rightBracket = leftBracket + 2; // 2: '[' + 2 = ']', '{' + 2 = '}'
    int counter = 1;
    val.push_back(jsonStr[index_++]);
    while (index_ < jsonStr.length()) {
        if (jsonStr[index_] == leftBracket) {
            ++counter;
        } else if (jsonStr[index_] == rightBracket) {
            --counter;
            if (counter == 0) {
                break;
            }
        }
        val.push_back(jsonStr[index_++]);
    }
    val.push_back(jsonStr[index_++]);
    return val;
}
} // namespace HiviewDFX
} // namespace OHOS
