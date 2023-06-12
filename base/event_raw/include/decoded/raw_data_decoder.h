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

#ifndef BASE_EVENT_RAW_DECODE_INCLUDE_RAW_DATA_DECODER_H
#define BASE_EVENT_RAW_DECODE_INCLUDE_RAW_DATA_DECODER_H

#include "base/raw_data_base_def.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventRaw {
class RawDataDecoder {
public:
    static bool FloatingNumberDecoded(uint8_t* rawData, const size_t maxLen, size_t& pos, double& dest);
    static bool SignedVarintDecoded(uint8_t* rawData, const size_t maxLen, size_t& pos, int64_t& dest);
    static bool StringValueDecoded(uint8_t* rawData, const size_t maxLen, size_t& pos, std::string& dest);
    static bool UnsignedVarintDecoded(uint8_t* rawData, const size_t maxLen, size_t& pos, uint64_t& dest);
    static bool ValueTypeDecoded(uint8_t* rawData, const size_t maxLen, size_t& pos, struct ParamValueType& dest);

private:
    static constexpr int TAG_BYTE_OFFSET = 5;
    static constexpr int TAG_BYTE_BOUND  = (1 << TAG_BYTE_OFFSET);
    static constexpr int TAG_BYTE_MASK = (TAG_BYTE_BOUND - 1);

    static constexpr int NON_TAG_BYTE_OFFSET = 7;
    static constexpr int NON_TAG_BYTE_BOUND = (1 << NON_TAG_BYTE_OFFSET);
    static constexpr int NON_TAG_BYTE_MASK = (NON_TAG_BYTE_BOUND - 1);
};
} // namespace EventRaw
} // namespace HiviewDFX
} // namespace OHOS

#endif // BASE_EVENT_RAW_DECODE_INCLUDE_RAW_DATA_DECODER_H