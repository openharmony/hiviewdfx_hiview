/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef INTERFACES_INNER_API_UNIFIED_COLLECTION_UTILITY_THERMAL_COLLECTOR_H
#define INTERFACES_INNER_API_UNIFIED_COLLECTION_UTILITY_THERMAL_COLLECTOR_H
#include <memory>
#include <vector>

#include "collect_result.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
enum ThermalZone {
    SHELL_FRONT,
    SHELL_FRAME,
    SHELL_BACK,
    SOC_THERMAL,
    SYSTEM
};

class ThermalCollector {
public:
    ThermalCollector() = default;
    virtual ~ThermalCollector() = default;

public:
    virtual CollectResult<int32_t> CollectDevThermal(ThermalZone thermalZone) = 0;
    virtual CollectResult<uint32_t> CollectThermaLevel() = 0;
    static std::shared_ptr<ThermalCollector> Create();
};
} // UCollectUtil
} // HiviewDFX
} // OHOS
#endif // INTERFACES_INNER_API_UNIFIED_COLLECTION_UTILITY_THERMAL_COLLECTOR_H
