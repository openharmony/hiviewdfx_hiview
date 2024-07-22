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
#ifndef HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_TEMPERATURE_DECORATOR_H
#define HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_TEMPERATURE_DECORATOR_H

#include "decorator.h"
#include "temperature_collector.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
class TemperatureDecorator : public TemperatureCollector, public UCDecorator {
public:
    TemperatureDecorator(std::shared_ptr<TemperatureCollector> collector) : temperatureCollector_(collector) {};
    virtual ~TemperatureDecorator() = default;

public:
    virtual CollectResult<uint32_t> CollectDevThermal(DeviceZone deviceZone) override;
    virtual CollectResult<uint32_t> CollectThermaLevel() override;
    static void SaveStatCommonInfo();
    static void ResetStatInfo();

private:
    std::shared_ptr<TemperatureCollector> temperatureCollector_;
    static StatInfoWrapper statInfoWrapper_;
};
} // namespace UCollectUtil
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_TEMPERATURE_DECORATOR_H
