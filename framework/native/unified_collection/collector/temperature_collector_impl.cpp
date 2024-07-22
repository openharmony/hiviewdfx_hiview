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
#include <unordered_map>

#include "common_util.h"
#include "file_util.h"
#include "hiview_logger.h"
#include "temperature_decorator.h"
#ifdef THERMAL_MANAGER_ENABLE
#include "thermal_mgr_client.h"
#endif

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
    std::string path = thermalZone + "/temp";
    return CommonUtil::ReadNodeWithOnlyNumber(path);
}

std::string GetZonePathByType(const std::vector<std::string>& thermalZones, DeviceZone deviceZone)
{
    const std::unordered_map<DeviceZone, std::string> deviceZoneMap = {
        {SHELL_FRONT, "shell_front"},
        {SHELL_FRAME, "shell_frame"},
        {SHELL_BACK, "shell_back"},
        {SOC_THERMAL, "soc_thermal"},
        {SYSTEM, "system_h"}
    };
    std::string typeStr = deviceZoneMap.at(deviceZone);
    for (const auto& zone : thermalZones) {
        if (typeStr == GetThermalType(zone)) {
            return zone;
        }
    }
    return "";
}
}

std::shared_ptr<TemperatureCollector> TemperatureCollector::Create()
{
    return std::make_shared<TemperatureDecorator>(std::make_shared<TemperatureCollectorImpl>());
}

CollectResult<uint32_t> TemperatureCollectorImpl::CollectDevThermal(DeviceZone deviceZone)
{
    CollectResult<uint32_t> result;
    std::vector<std::string> thermalZones;
    FileUtil::GetDirDirs(THERMAL_PATH, thermalZones);
    std::string zonePath = GetZonePathByType(thermalZones, deviceZone);
    if (zonePath.empty()) {
        return result;
    }
    result.data = GetThermalValue(zonePath);
    result.retCode = UcError::SUCCESS;
    return result;
}

CollectResult<uint32_t> TemperatureCollectorImpl::CollectThermaLevel()
{
    CollectResult<uint32_t> result;
#ifdef THERMAL_MANAGER_ENABLE
    auto& thermalMgrClient = PowerMgr::ThermalMgrClient::GetInstance();
    PowerMgr::ThermalLevel thermalLevel = thermalMgrClient.GetThermalLevel();
    result.retCode = UcError::SUCCESS;
    result.data = static_cast<uint32_t>(thermalLevel);
#endif
    return result;
}
} // namespace UCollectUtil
} // namespace HiviewDFX
} // namespace OHOS
