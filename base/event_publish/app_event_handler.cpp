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

template <typename T>
void AddValueToJsonString(const std::string& key, const T& value, std::stringstream& jsonStr, bool isLastValue = false)
{
    jsonStr << "\"" << key << "\":";
    if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
        jsonStr << "\"" << value << "\"";
    } else {
        jsonStr << value;
    }
    if (isLastValue) {
        jsonStr << "}";
    } else {
        jsonStr << ",";
    }
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
} // namespace HiviewDFX
} // namespace OHOS