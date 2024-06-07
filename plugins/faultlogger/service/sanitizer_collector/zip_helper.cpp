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

#include "zip_helper.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <file_util.h>
#include <regex>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time_util.h>
#include <unistd.h>

#include "bundle_mgr_client.h"
#include "climits"
#include "event_publish.h"
#include "hisysevent.h"
#include "json/json.h"
#include "sanitizerd_log.h"
#include "securec.h"
#include "string_ex.h"
#include "parameters.h"
#include "parameter_ex.h"

namespace OHOS {
namespace HiviewDFX {
using namespace OHOS::AppExecFwk;
std::vector<std::string> SplitString(const std::string& input, const std::string& regex)
{
    std::regex re(regex);
    std::sregex_token_iterator first {input.begin(), input.end(), re, -1}, last;
    return {first, last};
}

unsigned HashString(const std::string& input)
{
    unsigned hash = 0;
    for (size_t i = 0; i < input.length(); ++i) {
        hash = hash * HASH_FACTOR + input[i];
    }
    return hash;
}

bool IsNameValid(const std::string& name, const std::string& sep, bool canEmpty)
{
    std::vector<std::string> nameVec;
    SplitStr(name, sep, nameVec, canEmpty, false);
    std::regex re("^[a-zA-Z0-9][a-zA-Z0-9_]{0,127}$");
    for (auto const& splitName : nameVec) {
        if (!std::regex_match(splitName, re)) {
            HILOG_INFO(LOG_CORE, "Invalid splitName:%{public}s", splitName.c_str());
            return false;
        }
    }
    return true;
}

bool IsModuleNameValid(const std::string& name)
{
    if (name.empty() || name.size() > MAX_NAME_LENGTH) {
        HILOG_INFO(LOG_CORE, "invalid log name.");
        return false;
    }

    if (name.find("/") != std::string::npos || name.find(".") == std::string::npos) {
        std::string path = name.substr(1); // may skip first .
        path.erase(path.find_last_not_of(" \n\r\t") + 1);
        HILOG_INFO(LOG_CORE, "module name:%{public}s", name.c_str());
        return IsNameValid(path, "/", false);
    }

    return IsNameValid(name, ".", true);
}

std::string GetApplicationNameById(int32_t uid)
{
    std::string bundleName;
    AppExecFwk::BundleMgrClient client;
    if (client.GetNameForUid(uid, bundleName) != ERR_OK) {
        HILOG_WARN(LOG_CORE, "Failed to query bundleName from bms, uid:%{public}d.", uid);
    } else {
        HILOG_INFO(LOG_CORE, "bundleName of uid:%{public}d is %{public}s", uid, bundleName.c_str());
    }
    return bundleName;
}

bool GetDfxBundleInfo(const std::string& bundleName, DfxBundleInfo& bundleInfo)
{
    AppExecFwk::BundleInfo info;
    AppExecFwk::BundleMgrClient client;
    if (!client.GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT,
                              info, Constants::ALL_USERID)) {
        HILOG_WARN(LOG_CORE, "Failed to query BundleInfo from bms, bundle:%{public}s.", bundleName.c_str());
        return false;
    } else {
        HILOG_INFO(LOG_CORE, "The version of %{public}s is %{public}s", bundleName.c_str(),
            info.versionName.c_str());
    }
    bundleInfo.isPreInstalled = info.isPreInstallApp;
    bundleInfo.versionName = info.versionName;
    bundleInfo.versionCode = info.versionCode;
    return true;
}

int32_t CreateMultiTierDirectory(const std::string &directoryPath, const std::string &rootDirPath,
                                 const uid_t dirOwner, const gid_t dirGroup)
{
    int32_t ret = -1;
    uint32_t dirPathLen = directoryPath.length();
    if (dirPathLen > PATH_MAX) {
        return ret;
    }
    char tmpDirPath[PATH_MAX] = { 0 };
    for (uint32_t i = 0; i < dirPathLen; ++i) {
        tmpDirPath[i] = directoryPath[i];
        if (i < rootDirPath.length() && directoryPath[i] != rootDirPath[i]) {
            return ret;
        } else if (i < rootDirPath.length() || tmpDirPath[i] != '/') {
            continue;
        }
        if (access(tmpDirPath, 0) != 0) {
            ret = mkdir(tmpDirPath, DEFAULT_LOG_DIR_MODE);
            ret += chown(tmpDirPath, dirOwner, dirGroup);
            if (ret != 0) {
                HILOG_ERROR(LOG_CORE, "Fail to create dir %{public}s,  err: %{public}s.",
                    tmpDirPath, strerror(errno));
                return ret;
            }
        }
    }
    return 0;
}

static std::string GetCollectedDataSavePath(const T_SANITIZERD_PARAMS *params)
{
    std::string faultLogPath = std::string(ROOT_FAULTLOG_LOG_PATH);
    std::string filePath = std::string(CUSTOM_SANITIZER_LOG_PATH);
    return filePath;
}

static std::string CalcCollectedLogName(T_SANITIZERD_PARAMS *params)
{
    std::string filePath = GetCollectedDataSavePath(params);
    if (filePath.size() == 0) {
        return filePath;
    }
    std::string prefix = std::string(SANITIZERD_TYPE_STR[params->type][PREFIXFILENAME]);
    std::string name = params->procName;
    if (name.find("/") != std::string::npos) {
        name = params->procName.substr(params->procName.find_last_of("/") + 1);
    }

    std::string fileName = "";
    fileName.append(prefix);
    fileName.append("-");
    fileName.append(name);
    fileName.append("-");
    fileName.append(std::to_string(params->uid));
    fileName.append("-");
    fileName.append(std::to_string(params->happenTime));

    std::string fullName = filePath + fileName;

    params->logName = fileName;
    params->logPath = fullName;

    return fullName;
}

static int32_t CreateLogFile(const std::string& name)
{
    int32_t fd = -1;
    if (!FileUtil::FileExists(name)) {
        HILOG_WARN(LOG_CORE, "file %{public}s is creating now.", name.c_str());
    }
    fd = open(name.c_str(), O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_LOG_FILE_MODE);
    return fd;
}

static bool WriteNewFile(const int32_t fd, const T_SANITIZERD_PARAMS *params)
{
    if (fd < 0) {
        return false;
    }

    FileUtil::SaveStringToFd(fd, "Generated by HiviewDFX @OpenHarmony " +
            system::GetParameter(DEVICE_OHOS_VERSION_PARAM, EMPTY_PARAM) + "\n" +
            "=================================================================\n" +
            "TIMESTAMP:" + std::to_string(params->happenTime) + "\n" +
            "Device Info:" + Parameter::GetString("const.product.name", "Unknown") + "\n" +
            "Build Info:" + Parameter::GetString("const.product.software.version", "Unknown") + "\n" +
            "Pid:" + std::to_string(params->pid) + "\n" +
            "Uid:" + std::to_string(params->uid) + "\n" +
            "Process name:" + params->procName + "\n" +
            "Reason:" + std::string(SANITIZERD_TYPE_STR[params->type][ORISANITIZERTYPE]) + ":" +
            params->errType + "\n" +
            "Fault thread info:\n" +
            params->description);

    close(fd);
    return true;
}

void WriteCollectedData(T_SANITIZERD_PARAMS *params)
{
    std::string fullName = CalcCollectedLogName(params);
    if (fullName.size() == 0) {
        return;
    }
    int32_t fd = CreateLogFile(fullName);
    if (fd < 0) {
        return;
    }

    if (!WriteNewFile(fd, params)) {
        HILOG_ERROR(LOG_CORE, "Fail to write %{public}s,  err: %{public}s.", fullName.c_str(), strerror(errno));
    }
    Json::Value eventParams;
    auto timeNow = TimeUtil::GetMilliseconds();
    eventParams["time"] = timeNow;
    eventParams["type"] = params->errType;
    eventParams["bundle_version"] = params->appVersion;
    eventParams["bundle_name"] = params->procName;
    eventParams["external_log"] = params->logPath;
    eventParams["pid"] = params->pid;
    eventParams["uid"] = params->uid;

    std::string paramsStr = Json::FastWriter().write(eventParams);
    HILOG_INFO(LOG_CORE, "ReportAppEvent: uid:%{public}d, json:%{public}s.",
        params->uid, paramsStr.c_str());
    EventPublish::GetInstance().PushEvent(params->uid, "ADDRESS_SANITIZER", HiSysEvent::EventType::FAULT, paramsStr);
}
} // namespace HiviewDFX
} // namespace OHOS
