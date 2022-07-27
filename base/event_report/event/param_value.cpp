/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "param_value.h"

namespace OHOS {
namespace HiviewDFX {
ParamValue::ParamValue()
{}

ParamValue::ParamValue(uint8_t value)
{
    this->value_ = value;
}

ParamValue::ParamValue(uint16_t value)
{
    this->value_ = value;
}

ParamValue::ParamValue(uint32_t value)
{
    this->value_ = value;
}

ParamValue::ParamValue(uint64_t value)
{
    this->value_ = value;
}

ParamValue::ParamValue(const std::string& value)
{
    this->value_ = value;
}

ParamValue::ParamValue(const std::vector<uint32_t>& value)
{
    this->value_ = value;
}

ParamValue::ParamValue(const std::vector<std::string>& value)
{
    this->value_ = value;
}

size_t ParamValue::GetType() const
{
    return this->value_.index();
}

uint8_t ParamValue::GetUint8() const
{
    return IsUint8() ? std::get<uint8_t>(this->value_) : DEFAULT_UINT8;
}

uint16_t ParamValue::GetUint16() const
{
    return IsUint16() ? std::get<uint16_t>(this->value_) : DEFAULT_UINT16;
}

uint32_t ParamValue::GetUint32() const
{
    return IsUint32() ? std::get<uint32_t>(this->value_) : DEFAULT_UINT32;
}

uint64_t ParamValue::GetUint64() const
{
    return IsUint64() ? std::get<uint64_t>(this->value_) : DEFAULT_UINT64;
}

std::string ParamValue::GetString() const
{
    return IsString() ? std::get<std::string>(this->value_) : DEFAULT_STRING;
}

std::vector<uint32_t> ParamValue::GetUint32Vec() const
{
    return IsUint32Vec() ? std::get<std::vector<uint32_t>>(this->value_) : DEFAULT_UINT32_VEC;
}

std::vector<std::string> ParamValue::GetStringVec() const
{
    return IsStringVec() ? std::get<std::vector<std::string>>(this->value_) : DEFAULT_STRING_VEC;
}

bool ParamValue::IsUint8() const
{
    return std::get_if<uint8_t>(&this->value_) != nullptr;
}

bool ParamValue::IsUint16() const
{
    return std::get_if<uint16_t>(&this->value_) != nullptr;
}

bool ParamValue::IsUint32() const
{
    return std::get_if<uint32_t>(&this->value_) != nullptr;
}

bool ParamValue::IsUint64() const
{
    return std::get_if<uint64_t>(&this->value_) != nullptr;
}

bool ParamValue::IsString() const
{
    return std::get_if<std::string>(&this->value_) != nullptr;
}

bool ParamValue::IsUint32Vec() const
{
    return std::get_if<std::vector<uint32_t>>(&this->value_) != nullptr;
}

bool ParamValue::IsStringVec() const
{
    return std::get_if<std::vector<std::string>>(&this->value_) != nullptr;
}
} // namespace HiviewDFX
} // namespace OHOS
