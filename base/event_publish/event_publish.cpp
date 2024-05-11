/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "hiview_logger.h"
#include "storage_acl.h"
#include "string_util.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
DEFINE_LOG_TAG("HiView-EventPublish");
constexpr int VALUE_MOD = 200000;
constexpr int DELAY_TIME = 30;
const std::string PATH_DIR = "/data/log/hiview/system_event_db/events/temp";
const std::string SANDBOX_DIR = "/data/storage/el2/log";
const std::string FILE_PREFIX = "/hiappevent_";
const std::string FILE_SUFFIX = ".evt";
const std::string DOMAIN_PROPERTY = "domain";
const std::string NAME_PROPERTY = "name";
const std::string EVENT_TYPE_PROPERTY = "eventType";
const std::string PARAM_PROPERTY = "params";
const std::string DOMAIN_OS = "OS";
const std::string LOG_OVER_LIMIT = "log_over_limit";
const std::string EXTERNAL_LOG = "external_log";
const std::string PID = "pid";
const std::string MAIN_THREAD_JANK = "MAIN_THREAD_JANK";
constexpr uint64_t MAX_FILE_SIZE = 5 * 1024 * 1024; // 5M
constexpr uint64_t WATCHDOG_MAX_FILE_SIZE = 10 * 1024 * 1024; // 10M

struct ExternalLogInfo {
    std::string extensionType_;
    std::string subPath_;
    uint64_t maxFileSize_;
};

void GetExternalLogInfo(const std::string &eventName, ExternalLogInfo &externalLogInfo)
{
    if (eventName == MAIN_THREAD_JANK) {
        externalLogInfo.extensionType_ = ".trace";
        externalLogInfo.subPath_ = "watchdog";
        externalLogInfo.maxFileSize_ = WATCHDOG_MAX_FILE_SIZE;
    } else {
        externalLogInfo.extensionType_ = ".log";
        externalLogInfo.subPath_ = "hiappevent";
        externalLogInfo.maxFileSize_ = MAX_FILE_SIZE;
    }
}


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

std::string GetSandBoxBasePath(int32_t uid, const std::string& bundleName)
{
    int userId = uid / VALUE_MOD;
    std::string path;
    path.append("/data/app/el2/")
        .append(std::to_string(userId))
        .append("/base/")
        .append(bundleName)
        .append("/cache/hiappevent");
    return path;
}

std::string GetSandBoxLogPath(int32_t uid, const std::string& bundleName, const ExternalLogInfo &externalLogInfo)
{
    int userId = uid / VALUE_MOD;
    std::string path;
    path.append("/data/app/el2/")
        .append(std::to_string(userId))
        .append("/log/")
        .append(bundleName);
    path.append("/").append(externalLogInfo.subPath_);
    return path;
}

bool CopyExternalLog(int32_t uid, const std::string& externalLog, const std::string& destPath)
{
    if (FileUtil::CopyFile(externalLog, destPath) == 0) {
        std::string entryTxt = "g:" + std::to_string(uid) + ":rwx";
        if (OHOS::StorageDaemon::AclSetAccess(destPath, entryTxt) != 0) {
            HIVIEW_LOGE("failed to set acl access dir=%{public}s", destPath.c_str());
        }
        return true;
    }
    HIVIEW_LOGE("failed to move log file=%{public}s to destPath=%{public}s.", externalLog.c_str(), destPath.c_str());
    return false;
}

void SendLogToSandBox(int32_t uid, const std::string& eventName, std::string& sandBoxLogPath, Json::Value& params,
    const ExternalLogInfo &externalLogInfo)
{
    if (!params.isMember(EXTERNAL_LOG) || !params[EXTERNAL_LOG].isArray() || params[EXTERNAL_LOG].empty()) {
        HIVIEW_LOGE("no external log need to copy.");
        return;
    }
    params[LOG_OVER_LIMIT] = false;
    Json::Value externalLogJson(Json::arrayValue);
    uint64_t dirSize = FileUtil::GetFolderSize(sandBoxLogPath);
    for (Json::ArrayIndex i = 0; i < params[EXTERNAL_LOG].size(); ++i) {
        std::string externalLog = params[EXTERNAL_LOG][i].asString();
        if (externalLog.empty()) {
            HIVIEW_LOGI("externalLog is empty.");
            continue;
        }
        if (externalLog.find(SANDBOX_DIR) == 0) {
            HIVIEW_LOGI("file in sandbox path not copy.");
            externalLogJson.append(externalLog);
            continue;
        }
        uint64_t fileSize = FileUtil::GetFileSize(externalLog);
        if (dirSize + fileSize <= externalLogInfo.maxFileSize_) {
            std::string timeStr = std::to_string(TimeUtil::GetMilliseconds());
            int pid = 0;
            if (params.isMember(PID) && params[PID].isInt()) {
                pid = params[PID].asInt();
            }
            std::string destPath;
            std::string desFileName = eventName + "_" + timeStr + "_" + std::to_string(pid)
                + externalLogInfo.extensionType_;
            destPath.append(sandBoxLogPath).append("/").append(desFileName);
            if (CopyExternalLog(uid, externalLog, destPath)) {
                dirSize += fileSize;
                externalLogJson.append("/data/storage/el2/log/" + externalLogInfo.subPath_ + "/" + desFileName);
                HIVIEW_LOGI("move log file=%{public}s to sandBoxLogPath=%{public}s.",
                    externalLog.c_str(), sandBoxLogPath.c_str());
            }
        } else {
            HIVIEW_LOGE("sand box log dir overlimit file=%{public}s, dirSzie=%{public}" PRIu64
                ", limitSize=%{public}" PRIu64, externalLog.c_str(), dirSize, externalLogInfo.maxFileSize_);
            params[LOG_OVER_LIMIT] = true;
            break;
        }
    }
    params[EXTERNAL_LOG] = externalLogJson;
}

void WriteEventJson(Json::Value& eventJson, const std::string& filePath)
{
    std::string eventStr = Json::FastWriter().write(eventJson);
    if (!FileUtil::SaveStringToFile(filePath, eventStr, false)) {
        HIVIEW_LOGE("failed to save event, eventName=%{public}s, file=%{public}s",
            eventJson[NAME_PROPERTY].asString().c_str(), filePath.c_str());
        return;
    }
    HIVIEW_LOGI("save event finish, eventName=%{public}s, file=%{public}s, eventStr=%{public}s",
        eventJson[NAME_PROPERTY].asString().c_str(), filePath.c_str(), eventStr.c_str());
}

void SaveEventAndLogToSandBox(int32_t uid, const std::string& eventName, const std::string& bundleName,
    Json::Value& eventJson)
{
    ExternalLogInfo externalLogInfo;
    GetExternalLogInfo(eventName, externalLogInfo);
    std::string sandBoxLogPath = GetSandBoxLogPath(uid, bundleName, externalLogInfo);
    SendLogToSandBox(uid, eventName, sandBoxLogPath, eventJson[PARAM_PROPERTY], externalLogInfo);
    std::string desPath = GetSandBoxBasePath(uid, bundleName);
    std::string timeStr = std::to_string(TimeUtil::GetMilliseconds());
    desPath.append(FILE_PREFIX).append(timeStr).append(".txt");
    WriteEventJson(eventJson, desPath);
}

void SaveEventToTempFile(int32_t uid, Json::Value& eventJson)
{
    std::string tempPath = GetTempFilePath(uid);
    WriteEventJson(eventJson, tempPath);
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
        std::string bundleName = GetBundleNameById(uid);
        if (bundleName.empty()) {
            HIVIEW_LOGW("empty bundleName uid=%{public}d.", uid);
            (void)FileUtil::RemoveFile(srcPath);
            continue;
        }
        std::string desPath = GetSandBoxBasePath(uid, bundleName);
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
    std::string bundleName = GetBundleNameById(uid);
    if (bundleName.empty()) {
        HIVIEW_LOGW("empty bundleName uid=%{public}d.", uid);
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!FileUtil::FileExists(PATH_DIR) && !FileUtil::ForceCreateDirectory(PATH_DIR)) {
        HIVIEW_LOGE("failed to create resourceDir.");
        return;
    }
    std::string srcPath = GetTempFilePath(uid);
    std::string desPath = GetSandBoxBasePath(uid, bundleName);
    if (!FileUtil::FileExists(desPath)) {
        HIVIEW_LOGE("desPath=%{public}s not exit.", desPath.c_str());
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
        HIVIEW_LOGE("failed to parse paramJson bundleName=%{public}s, eventName=%{public}s.",
            bundleName.c_str(), eventName.c_str());
        return;
    }
    eventJson[PARAM_PROPERTY] = params;
    const std::unordered_set<std::string> immediateEvents = {"APP_CRASH", "APP_FREEZE", "ADDRESS_SANITIZER",
        "APP_LAUNCH", "CPU_USAGE_HIGH", MAIN_THREAD_JANK};
    if (immediateEvents.find(eventName) != immediateEvents.end()) {
        SaveEventAndLogToSandBox(uid, eventName, bundleName, eventJson);
    } else {
        SaveEventToTempFile(uid, eventJson);
        StartSendingThread();
    }
}
} // namespace HiviewDFX
} // namespace OHOS