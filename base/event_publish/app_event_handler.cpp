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
    jsonStr << "\"" << "time" << "\":" << TimeUtil::GetMilliseconds() << ",";
    jsonStr << "\"" << "bundle_version" << "\":" << "\"" << event.bundleVersion << "\",";
    jsonStr << "\"" << "bundle_name" << "\":" << "\"" << event.bundleName << "\",";
    jsonStr << "\"" << "process_name" << "\":" << "\"" << event.processName << "\",";
    jsonStr << "\"" << "start_type" << "\":" << event.startType << ",";
    jsonStr << "\"" << "icon_input_time" << "\":" << event.iconInputTime << ",";
    jsonStr << "\"" << "animation_finish_time" << "\":" << event.animationFinishTime << ",";
    jsonStr << "\"" << "extend_time" << "\":" << event.extendTime;
    jsonStr << "}" << std::endl;
    EventPublish::GetInstance().PushEvent(uid, "APP_LAUNCH", HiSysEvent::EventType::BEHAVIOR, jsonStr.str());
    return 0;
}
} // namespace HiviewDFX
} // namespace OHOS