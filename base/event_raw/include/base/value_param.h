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

#ifndef BASE_EVENT_RAW_INCLUDE_VALUE_PARAM_H
#define BASE_EVENT_RAW_INCLUDE_VALUE_PARAM_H

#include <cstdarg>
#include <cstddef>
#include <cstdint>

namespace OHOS {
namespace HiviewDFX {
namespace EventRaw {
enum DataCodedType: uint8_t {
    UNSIGNED_VARINT,
    UNSIGNED_VARINT_ARRAY,
    SIGNED_VARINT,
    SIGNED_VARINT_ARRAY,
    FLOATING,
    FLOATING_ARRAY,
    DSTRING,
    DSTRING_ARRAY,
};

class ValueParam {
public:
    virtual bool AsUint64(uint64_t& dest) = 0;
    virtual bool AsInt64(int64_t& dest) = 0;
    virtual bool AsDouble(double& dest) = 0;
    virtual bool AsString(std::string& dest) = 0;
    virtual bool AsUint64Vec(std::vector<uint64_t>& dest) = 0;
    virtual bool AsInt64Vec(std::vector<int64_t>& dest) = 0;
    virtual bool AsDoubleVec(std::vector<double>& dest) = 0;
    virtual bool AsStringVec(std::vector<std::string>& dest) = 0;
    virtual DataCodedType GetDataCodedType() = 0;
};
} // namespace EventRaw
} // namespace HiviewDFX
} // namespace OHOS

#endif // BASE_EVENT_RAW_INCLUDE_VALUE_PARAM_H