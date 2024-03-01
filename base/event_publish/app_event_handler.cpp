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

#include "app_event_handler.h"

#include <sstream>

#include "bundle_mgr_client.h"
#include "event_publish.h"
#include "logger.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
DEFINE_LOG_TAG("HiView-AppEventHandler");

int32_t GetUidByBundleName(const std::string& bundleName)
{
    AppExecFwk::BundleInfo info;
    AppExecFwk::BundleMgrClient client;
    if (!client.GetBundleInfo(bundleName, AppExecFwk::GET_BUNDLE_DEFAULT, info,
        AppExecFwk::Constants::ALL_USERID)) {
        HIVIEW_LOGE("Failed to query uid from bms, bundleName=%{public}s.", bundleName.c_str());
    } else {
        HIVIEW_LOGD("bundleName of uid=%{public}d, bundleName=%{public}s", info.uid, bundleName.c_str());
    }
    return info.uid;
}

std::string CovertVectorToStr(const std::vector<uint64_t>& vec)
{
    if (vec.empty()) {
        return "";
    }
    std::stringstream ss;
    ss << "\"[";
    std::copy(vec.begin(), vec.end() - 1, std::ostream_iterator<int>(ss, ","));
    ss << vec.back() << "]\"";
    return ss.str();
}

template <typename T>
void AddValueToJsonString(const std::string& key, const T& value, std::stringstream& jsonStr, bool isLastValue = false)
{
    jsonStr << "\"" << key << "\":";
    if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
        jsonStr << "\"" << value << "\"";
    } else if constexpr (std::is_same_v<std::decay_t<T>, std::vector<uint64_t>>) {
        jsonStr << CovertVectorToStr(value);
    } else {
        jsonStr << value;
    }
    if (isLastValue) {
        jsonStr << "}";
    } else {
        jsonStr << ",";
    }
}

void AddObjectToJsonString(const std::string& name, std::stringstream& jsonStr)
{
    jsonStr << "\"" << name << "\":{";
}

void AddTimeToJsonString(std::stringstream& jsonStr)
{
    auto time = TimeUtil::GetMilliseconds();
    AddValueToJsonString("time", time, jsonStr);
}

void AddBundleInfoToJsonString(const AppEventHandler::BundleInfo& event, std::stringstream& jsonStr)
{
    AddValueToJsonString("bundle_version", event.bundleVersion, jsonStr);
    AddValueToJsonString("bundle_name", event.bundleName, jsonStr);
}

void AddProcessInfoToJsonString(const AppEventHandler::ProcessInfo& event, std::stringstream& jsonStr)
{
    AddValueToJsonString("process_name", event.processName, jsonStr);
}

void AddAbilityInfoToJsonString(const AppEventHandler::AbilityInfo& event, std::stringstream& jsonStr)
{
    AddValueToJsonString("ability_name", event.abilityName, jsonStr);
}
}

int AppEventHandler::PostEvent(const AppLaunchInfo& event)
{
    if (event.bundleName.empty()) {
        HIVIEW_LOGW("bundleName empty.");
        return -1;
    }
    int32_t uid = GetUidByBundleName(event.bundleName);
    std::stringstream jsonStr;
    jsonStr << "{";
    AddTimeToJsonString(jsonStr);
    AddBundleInfoToJsonString(event, jsonStr);
    AddProcessInfoToJsonString(event, jsonStr);
    AddValueToJsonString("start_type", event.startType, jsonStr);
    AddValueToJsonString("icon_input_time", event.iconInputTime, jsonStr);
    AddValueToJsonString("animation_finish_time", event.animationFinishTime, jsonStr);
    AddValueToJsonString("extend_time", event.extendTime, jsonStr, true);
    jsonStr << std::endl;
    EventPublish::GetInstance().PushEvent(uid, "APP_LAUNCH", HiSysEvent::EventType::BEHAVIOR, jsonStr.str());
    return 0;
}

int AppEventHandler::PostEvent(const ScrollJankInfo& event)
{
    if (event.bundleName.empty()) {
        HIVIEW_LOGW("bundleName empty.");
        return -1;
    }
    int32_t uid = GetUidByBundleName(event.bundleName);
    std::stringstream jsonStr;
    jsonStr << "{";
    AddTimeToJsonString(jsonStr);
    AddBundleInfoToJsonString(event, jsonStr);
    AddProcessInfoToJsonString(event, jsonStr);
    AddAbilityInfoToJsonString(event, jsonStr);
    AddValueToJsonString("begin_time", event.beginTime, jsonStr);
    AddValueToJsonString("duration", event.duration, jsonStr);
    AddValueToJsonString("total_app_frames", event.totalAppFrames, jsonStr);
    AddValueToJsonString("total_app_missed_frames", event.totalAppMissedFrames, jsonStr);
    AddValueToJsonString("max_app_frametime", event.maxAppFrametime, jsonStr);
    AddValueToJsonString("max_app_seq_frames", event.maxAppSeqFrames, jsonStr);
    AddValueToJsonString("total_render_frames", event.totalRenderFrames, jsonStr);
    AddValueToJsonString("total_render_missed_frames", event.totalRenderMissedFrames, jsonStr);
    AddValueToJsonString("max_render_frametime", event.maxRenderFrametime, jsonStr);
    AddValueToJsonString("max_render_seq_frames", event.maxRenderSeqFrames, jsonStr, true);
    jsonStr << std::endl;
    EventPublish::GetInstance().PushEvent(uid, "SCROLL_JANK", HiSysEvent::EventType::FAULT, jsonStr.str());
    return 0;
}

int AppEventHandler::PostEvent(const ResourceOverLimitInfo& event)
{
    if (event.bundleName.empty()) {
        HIVIEW_LOGW("bundleName empty.");
        return -1;
    }
    int32_t uid = GetUidByBundleName(event.bundleName);
    std::stringstream jsonStr;
    jsonStr << "{";
    AddTimeToJsonString(jsonStr);
    AddBundleInfoToJsonString(event, jsonStr);
    AddValueToJsonString("pid", event.pid, jsonStr);
    AddValueToJsonString("uid", event.uid, jsonStr);
    AddValueToJsonString("resource_type", event.resourceType, jsonStr);
    AddObjectToJsonString("memory", jsonStr);
    AddValueToJsonString("pss", event.pss, jsonStr);
    AddValueToJsonString("rss", event.rss, jsonStr);
    AddValueToJsonString("vss", event.vss, jsonStr);
    AddValueToJsonString("sys_avail_mem", event.avaliableMem, jsonStr);
    AddValueToJsonString("sys_free_mem", event.freeMem, jsonStr);
    AddValueToJsonString("sys_total_mem", event.totalMem, jsonStr, true);
    jsonStr << "}" << std::endl;
    EventPublish::GetInstance().PushEvent(event.uid, "RESOURCE_OVERLIMIT", HiSysEvent::EventType::FAULT, jsonStr.str());
    return 0;
}

int AppEventHandler::PostEvent(const CpuHighLoadInfo& event)
{
    if (event.bundleName.empty()) {
        HIVIEW_LOGW("bundleName empty");
        return -1;
    }
    int32_t uid = GetUidByBundleName(event.bundleName);
    std::stringstream jsonStr;
    jsonStr << "{";
    AddTimeToJsonString(jsonStr);
    AddBundleInfoToJsonString(event, jsonStr);
    AddValueToJsonString("foreground", event.foreground, jsonStr);
    AddValueToJsonString("usage", event.usage, jsonStr);
    AddValueToJsonString("begin_time", event.beginTime, jsonStr);
    AddValueToJsonString("end_time", event.endTime, jsonStr, true);
    jsonStr << std::endl;
    EventPublish::GetInstance().PushEvent(uid, "CPU_USAGE_HIGH", HiSysEvent::EventType::FAULT, jsonStr.str());
    return 0;
}

int AppEventHandler::PostEvent(const PowerConsumptionInfo& event)
{
    if (event.bundleName.empty()) {
        HIVIEW_LOGW("bundleName empty");
        return -1;
    }
    int32_t uid = GetUidByBundleName(event.bundleName);
    std::stringstream jsonStr;
    jsonStr << "{";
    AddTimeToJsonString(jsonStr);
    AddBundleInfoToJsonString(event, jsonStr);
    AddValueToJsonString("begin_time", event.beginTime, jsonStr);
    AddValueToJsonString("end_time", event.endTime, jsonStr);
    AddValueToJsonString("foreground_usage", event.usage.foregroundValue, jsonStr);
    AddValueToJsonString("background_usage", event.usage.backgroundValue, jsonStr);
    AddValueToJsonString("cpu_foreground_energy", event.cpuEnergy.foregroundValue, jsonStr);
    AddValueToJsonString("cpu_background_energy", event.cpuEnergy.backgroundValue, jsonStr);
    AddValueToJsonString("gpu_foreground_energy", event.gpuEnergy.foregroundValue, jsonStr);
    AddValueToJsonString("gpu_background_energy", event.gpuEnergy.backgroundValue, jsonStr);
    AddValueToJsonString("ddr_foreground_energy", event.ddrEnergy.foregroundValue, jsonStr);
    AddValueToJsonString("ddr_background_energy", event.ddrEnergy.backgroundValue, jsonStr);
    AddValueToJsonString("display_foreground_energy", event.displayEnergy.foregroundValue, jsonStr);
    AddValueToJsonString("display_background_energy", event.displayEnergy.backgroundValue, jsonStr);
    AddValueToJsonString("audio_foreground_energy", event.audioEnergy.foregroundValue, jsonStr);
    AddValueToJsonString("audio_background_energy", event.audioEnergy.backgroundValue, jsonStr);
    AddValueToJsonString("modem_foreground_energy", event.modemEnergy.foregroundValue, jsonStr);
    AddValueToJsonString("modem_background_energy", event.modemEnergy.backgroundValue, jsonStr);
    AddValueToJsonString("rom_foreground_energy", event.romEnergy.foregroundValue, jsonStr);
    AddValueToJsonString("rom_background_energy", event.romEnergy.backgroundValue, jsonStr);
    AddValueToJsonString("wifi_foreground_energy", event.wifiEnergy.foregroundValue, jsonStr);
    AddValueToJsonString("wifi_background_energy", event.wifiEnergy.backgroundValue, jsonStr);
    AddValueToJsonString("others_foreground_energy", event.othersEnergy.foregroundValue, jsonStr);
    AddValueToJsonString("others_background_energy", event.othersEnergy.backgroundValue, jsonStr, true);
    jsonStr << std::endl;
    EventPublish::GetInstance().PushEvent(uid, "BATTERY_USAGE", HiSysEvent::EventType::STATISTIC, jsonStr.str());
    return 0;
}
} // namespace HiviewDFX
} // namespace OHOS