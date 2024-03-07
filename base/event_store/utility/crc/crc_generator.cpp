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
#include "crc_generator.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
namespace {
struct Initializer {
    Initializer()
    {
        CrcGenerator::Initialize();
    }
};
}
uint32_t CrcGenerator::crcTable_[NUMBER_RANGE] = { 0 };

void CrcGenerator::Initialize()
{
    constexpr uint32_t polynomial = 0x1EDC6F41;
    for (uint32_t i = 0; i < NUMBER_RANGE; ++i) {
        uint32_t crc = i << 24; // 24: get the lowest byte
        for (uint32_t j = 0; j < BIT_RANGE; ++j) {
            if ((crc & 0x80000000) != 0) { // 0x80000000: get the highest bit
                crc = polynomial ^ (crc << 1); // 1: subtract the highest bit
            } else {
                crc = crc << 1; // 1: too
            }
        }
        crcTable_[i] = crc;
    }
}

uint32_t CrcGenerator::GetCrc32(uint8_t* bufPtr, uint32_t count)
{
    static Initializer initialize;
    uint32_t crc = 0x0;
    while (count-- > 0) {
        uint32_t data = static_cast<uint32_t>(*bufPtr);
        crc = (crc << 8) ^ crcTable_[((crc >> 24) ^ data) & 0x000000FF]; // 8,24,0x000000FF: calc crc value
    }
    return crc;
}
} // EventStore
} // HiviewDFX
} // OHOS