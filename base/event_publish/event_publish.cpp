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

#include "event_publish.h"

#include "bundle_mgr_client.h"
#include "file_util.h"
#include "json/json.h"
#include "logger.h"
#include "string_util.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
DEFINE_LOG_TAG("HiView-EventPublish");
constexpr int VALUE_MOD = 200000;
constexpr int DELAY_TIME = 30;
const std::string PATH_DIR = "/data/log/hiview/system_event_db/events/temp";
const std::string FILE_PREFIX = "/hiappevent_";
const std::string FILE_SUFFIX = ".evt";
const std::string DOMAIN_PROPERTY = "domain";
const std::string NAME_PROPERTY = "name";
const std::string EVENT_TYPE_PROPERTY = "eventType";
const std::string PARAM_PROPERTY = "params";
const std::string DOMAIN_OS = "OS";

std::string GetTempFilePath(int32_t uid)
{
    std::string srcPath = PATH_DIR;
    srcPath.append(FILE_PREFIX).append(std::to_string(uid)).append(FILE_SUFFIX);
    return srcPath;
}

std::string GetBundleNameById(int32_t uid)
{
    std::string bundleName;
    AppExecFwk::BundleMgrClient client;
    if (client.GetNameForUid(uid, bundleName) != 0) {
        HIVIEW_LOGW("Failed to query bundleName from bms, uid=%{public}d.", uid);
    } else {
        HIVIEW_LOGD("bundleName of uid=%{public}d, bundleName=%{public}s", uid, bundleName.c_str());
    }
    return bundleName;
}

std::string GetSandBoxPathByUid(int32_t uid)
{
    int userId = uid / VALUE_MOD;
    std::string bundleName = GetBundleNameById(uid);
    std::string path;
    path.append("/data/app/el2/")
        .append(std::to_string(userId))
        .append("/base/")
        .append(bundleName)
        .append("/cache/hiappevent");
    return path;
}
}

void EventPublish::StartSendingThread()
{
    if (sendingThread_ == nullptr) {
        HIVIEW_LOGI("start send thread.");
        sendingThread_ = std::make_unique<std::thread>(&EventPublish::SendEventToSandBox, this);
        sendingThread_->detach();
    }
}

void EventPublish::SendEventToSandBox()
{
    std::this_thread::sleep_for(std::chrono::seconds(DELAY_TIME));
    std::lock_guard<std::mutex> lock(mutex_);
    std::string timeStr = std::to_string(TimeUtil::GetMilliseconds());
    std::vector<std::string> files;
    FileUtil::GetDirFiles(PATH_DIR, files, false);
    for (const auto& srcPath : files) {
        std::string uidStr = StringUtil::GetMidSubstr(srcPath, FILE_PREFIX, FILE_SUFFIX);
        if (uidStr.empty()) {
            continue;
        }
        int32_t uid = StringUtil::StrToInt(uidStr);
        std::string desPath = GetSandBoxPathByUid(uid);
        if (!FileUtil::FileExists(desPath)) {
            HIVIEW_LOGE("SendEventToSandBox not exit desPath=%{public}s.", desPath.c_str());
            (void)FileUtil::RemoveFile(srcPath);
            continue;
        }
        desPath.append(FILE_PREFIX).append(timeStr).append(".txt");
        if (FileUtil::CopyFile(srcPath, desPath) == -1) {
            HIVIEW_LOGE("failed to move file=%{public}s to desFile=%{public}s.",
                srcPath.c_str(), desPath.c_str());
            continue;
        }
        HIVIEW_LOGI("copy srcPath=%{public}s, desPath=%{public}s.", srcPath.c_str(), desPath.c_str());
        (void)FileUtil::RemoveFile(srcPath);
    }
    sendingThread_.reset();
}

void EventPublish::PushEvent(int32_t uid, const std::string& eventName, HiSysEvent::EventType eventType,
    const std::string& paramJson)
{
    if (eventName.empty() || paramJson.empty() || uid < 0) {
        HIVIEW_LOGW("empty param.");
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!FileUtil::FileExists(PATH_DIR) && !FileUtil::ForceCreateDirectory(PATH_DIR)) {
        HIVIEW_LOGE("failed to create resourceDir.");
        return;
    }
    std::string srcPath = GetTempFilePath(uid);
    std::string desPath = GetSandBoxPathByUid(uid);
    if (!FileUtil::FileExists(desPath)) {
        HIVIEW_LOGD("PushEvent not exit desPath=%{public}s.", desPath.c_str());
        (void)FileUtil::RemoveFile(srcPath);
        return;
    }

    Json::Value eventJson;
    eventJson[DOMAIN_PROPERTY] = DOMAIN_OS;
    eventJson[NAME_PROPERTY] = eventName;
    eventJson[EVENT_TYPE_PROPERTY] = eventType;
    Json::Value params;
    Json::Reader reader;
    if (!reader.parse(paramJson, params)) {
        HIVIEW_LOGE("failed to parse paramJson.");
        return;
    }
    eventJson[PARAM_PROPERTY] = params;
    std::string eventStr = Json::FastWriter().write(eventJson);
    if (!FileUtil::SaveStringToFile(srcPath, eventStr, false)) {
        HIVIEW_LOGE("failed to persist event to file.");
    }
    StartSendingThread();
}
} // namespace HiviewDFX
} // namespace OHOS