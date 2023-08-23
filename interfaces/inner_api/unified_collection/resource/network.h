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
#ifndef INTERFACES_INNER_API_UNIFIED_COLLECTION_RESOURCE_NETWORK_H
#define INTERFACES_INNER_API_UNIFIED_COLLECTION_RESOURCE_NETWORK_H
#include <cinttypes>

namespace OHOS {
namespace HiviewDFX {
struct NetworkRate {
    int32_t rssi;
    int32_t txBitRate;
    int32_t rxBitRate;
};

struct NetworkPackets {
    int32_t currentSpeed;
    int32_t currentTxBytes;
    int32_t currentRxBytes;
};
} // HiviewDFX
} // OHOS
#endif // INTERFACES_INNER_API_UNIFIED_COLLECTION_RESOURCE_NETWORK_H