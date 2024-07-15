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

#include "temperature_collector_impl.h"

#include <string>
#include <unordered_set>

#include "file_util.h"
#include "hiview_logger.h"
#include "string_util.h"
#include "temperature_decorator.h"
#include "thermal_mgr_client.h"

using namespace OHOS::HiviewDFX::UCollect;

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
namespace {
DEFINE_LOG_TAG("TemperatureCollector");
const std::string THERMAL_PATH = "/sys/class/thermal/";

std::string GetThermalType(const std::string& thermalZone)
{
    std::string type;
    FileUtil::LoadStringFromFile(thermalZone + "/type", type);
    return type;
}

uint32_t GetThermalValue(const std::string& thermalZone)
{
    std::string value;
    FileUtil::LoadStringFromFile(thermalZone + "/temp", value);
    return StringUtil::StringToUl(value);
}

void UpdateThermalValue(DeviceThermal& deviceThermal, const std::string& thermalZone)
{
    std::string type = GetThermalType(thermalZone);
    if (strcmp(type.c_str(), "shell_front") == 0) {
        deviceThermal.shellFront = GetThermalValue(thermalZone);
    } else if (strcmp(type.c_str(), "shell_frame") == 0) {
        deviceThermal.shellFrame = GetThermalValue(thermalZone);
    } else if (strcmp(type.c_str(), "shell_back") == 0) {
        deviceThermal.shellBack = GetThermalValue(thermalZone);
    } else if (strcmp(type.c_str(), "soc_thermal") == 0) {
        deviceThermal.socThermal = GetThermalValue(thermalZone);
    } else if (strcmp(type.c_str(), "system_h") == 0) {
        deviceThermal.system = GetThermalValue(thermalZone);
    }
}
}


std::shared_ptr<TemperatureCollector> TemperatureCollector::Create()
{
    return std::make_shared<TemperatureDecorator>(std::make_shared<TemperatureCollectorImpl>());
}

CollectResult<DeviceThermal> TemperatureCollectorImpl::CollectDevThermal()
{
    CollectResult<DeviceThermal> result;
    DeviceThermal& deviceThermal = result.data;
    std::vector<std::string> thermalZones;
    FileUtil::GetDirDirs(THERMAL_PATH, thermalZones);
    for (const auto& zone : thermalZones) {
        UpdateThermalValue(deviceThermal, zone);
    }
    result.retCode = UcError::SUCCESS;
    return result;
}

CollectResult<uint32_t> TemperatureCollectorImpl::CollectThermaLevel()
{
    CollectResult<uint32_t> result;
    auto& thermalMgrClient = PowerMgr::ThermalMgrClient::GetInstance();
    PowerMgr::ThermalLevel thermalLevel = thermalMgrClient.GetThermalLevel();
    result.retCode = UcError::SUCCESS;
    result.data = static_cast<uint32_t>(thermalLevel);
    return result;
}
} // namespace UCollectUtil
} // namespace HiviewDFX
} // namespace OHOS