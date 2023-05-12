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

#ifndef HIVIEW_BASE_EVENT_STORE_UTILITY_CRC_GENERATOR_H
#define HIVIEW_BASE_EVENT_STORE_UTILITY_CRC_GENERATOR_H

#include <cstdint>

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
class CrcGenerator {
public:
    static uint32_t GetCrc32(uint8_t* bufPtr, uint32_t count);
    static void Initialize();

    static constexpr uint32_t NUMBER_RANGE { 256 }; // 256 means 2^8
    static constexpr uint32_t BIT_RANGE { 8 }; // means 8-bit

private:
    static uint32_t crcTable_[NUMBER_RANGE];
}; // CrcGenerator
} // EventStore
} // HiviewDFX
} // OHOS
#endif // HIVIEW_BASE_EVENT_STORE_UTILITY_CRC_GENERATOR_H
