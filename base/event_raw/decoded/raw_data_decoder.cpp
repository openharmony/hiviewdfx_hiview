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

#include "decoded/raw_data_decoder.h"

#include "securec.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventRaw {

bool RawDataDecoder::FloatingNumberDecoded(uint8_t* rawData, const size_t maxLen, size_t& pos, double& dest)
{
    if (rawData == nullptr || pos >= maxLen) {
        return false;
    }
    uint64_t valByteCnt = 0; // default 0
    if (!UnsignedVarintDecoded(rawData, maxLen, pos, valByteCnt)) {
        return false;
    }
    if ((pos + valByteCnt) > maxLen) {
        return false;
    }
    if (valByteCnt == sizeof(float)) {
        float tmpf;
        if (memcpy_s(reinterpret_cast<uint8_t*>(&tmpf), valByteCnt, rawData + pos, valByteCnt) != EOK) {
            return false;
        }
        dest = static_cast<double>(tmpf);
    }
    if (valByteCnt == sizeof(double)) {
        double tmpd;
        if (memcpy_s(reinterpret_cast<uint8_t*>(&tmpd), valByteCnt, rawData + pos, valByteCnt) != EOK) {
            return false;
        }
        dest = tmpd;
    }
    pos += valByteCnt;
    return true;
}

bool RawDataDecoder::SignedVarintDecoded(uint8_t* rawData, const size_t maxLen, size_t& pos, int64_t& dest)
{
    if (rawData == nullptr || pos >= maxLen) {
        return false;
    }
    uint64_t uval = 0;
    if (!UnsignedVarintDecoded(rawData, maxLen, pos, uval)) {
        return false;
    }
    // unzigzag
    dest = (uval >> 1) ^ -static_cast<int64_t>(uval & 1);
    return true;
}

bool RawDataDecoder::StringValueDecoded(uint8_t* rawData, const size_t maxLen, size_t& pos, std::string& dest)
{
    if (rawData == nullptr || pos >= maxLen) {
        return false;
    }
    uint64_t valByteCnt = 0; // default 0
    if (!UnsignedVarintDecoded(rawData, maxLen, pos, valByteCnt) ||
        valByteCnt > maxLen || // for value flip
        ((pos + valByteCnt) > maxLen)) {
        return false;
    }
    if (valByteCnt == 0) { // no need to copy
        return true;
    }
    dest = std::string(reinterpret_cast<char*>(rawData + pos), valByteCnt);
    pos += valByteCnt;
    return true;
}

bool RawDataDecoder::UnsignedVarintDecoded(uint8_t* rawData, const size_t maxLen, size_t& pos, uint64_t& dest)
{
    if (rawData == nullptr || pos >= maxLen) {
        return false;
    }
    uint64_t val = rawData[pos] & TAG_BYTE_MASK;
    uint32_t offset = TAG_BYTE_OFFSET;
    if ((pos + 1) > maxLen) {
        return false;
    }
    if (rawData[pos++] & TAG_BYTE_BOUND) {
        do {
            if (pos >= maxLen) {
                return false;
            }
            val |= (static_cast<uint64_t>(rawData[pos] & NON_TAG_BYTE_MASK) << offset);
            offset += NON_TAG_BYTE_OFFSET;
        } while (rawData[pos++] & NON_TAG_BYTE_BOUND);
    }
    dest = val;
    return true;
}

bool RawDataDecoder::ValueTypeDecoded(uint8_t* rawData, const size_t maxLen, size_t& pos,
    struct ParamValueType& dest)
{
    if (rawData == nullptr || pos >= maxLen) {
        return false;
    }
    if ((pos + sizeof(struct ParamValueType)) > maxLen) {
        return false;
    }
    dest = *(reinterpret_cast<struct ParamValueType*>(rawData + pos));
    pos += sizeof(struct ParamValueType);
    return true;
}
} // namespace EventRaw
} // namespace HiviewDFX
} // namespace OHOS