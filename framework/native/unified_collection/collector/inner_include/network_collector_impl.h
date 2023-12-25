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

#ifndef HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_NETWORK_COLLECTOR_IMPL_H
#define HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_NETWORK_COLLECTOR_IMPL_H

#include "network_collector.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
class NetworkCollectorImpl : public NetworkCollector {
public:
    NetworkCollectorImpl() = default;
    virtual ~NetworkCollectorImpl() = default;
    virtual CollectResult<NetworkRate> CollectRate() override;
    virtual CollectResult<NetworkPackets> CollectSysPackets() override;
};
} // namespace UCollectUtil
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_NETWORK_COLLECTOR_IMPL_H