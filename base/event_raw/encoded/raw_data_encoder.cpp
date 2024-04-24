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

#include "encoded/raw_data_encoder.h"

#include "hiview_logger.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventRaw {
DEFINE_LOG_TAG("HiView-RawDataEncoder");
uint8_t RawDataEncoder::EncodedTag(uint8_t type)
{
    return (type << (TAG_BYTE_OFFSET + 1));
}

bool RawDataEncoder::StringValueEncoded(RawData& data, const std::string& val)
{
    if (!UnsignedVarintEncoded(data, EncodeType::LENGTH_DELIMITED, val.length())) {
        return false;
    }
    if (!data.Append(reinterpret_cast<uint8_t*>(const_cast<char*>(val.c_str())),
        val.length())) {
        HIVIEW_LOGE("string value copy failed.");
        return false;
    }
    return true;
}

bool RawDataEncoder::ValueTypeEncoded(RawData& data, bool isArray, ValueType type, uint8_t count)
{
    struct ParamValueType kvType {
        .isArray = isArray ? 1 : 0,
        .valueType = static_cast<uint8_t>(type),
        .valueByteCnt = count,
    };
    if (!data.Append(reinterpret_cast<uint8_t*>(&kvType), sizeof(struct ParamValueType))) {
        return false;
    }
    return true;
}
} // namespace EventRaw
} // namespace HiviewDFX
} // namespace OHOS
