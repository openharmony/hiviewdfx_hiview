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

#include "hiview_config_util.h"

#include <algorithm>

#include "file_util.h"
#include "hiview_logger.h"
#include "hiview_zip_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiViewConfigUtil {
namespace {
DEFINE_LOG_TAG("HiViewConfigUtil");
constexpr char CFG_VERSION_FILE_NAME[] = "hiview_config_version";
constexpr char LOCAL_CFG_PATH[] = "/system/etc/hiview/";
constexpr char CLOUD_CFG_PATH[] = "/data/system/hiview/";
constexpr char LOCAL_CFG_UNZIP_DIR[] = "unzip_configs/";

inline std::string GetConfigFilePath(const std::string& configFileName, bool isLocal)
{
    if (isLocal) {
        return LOCAL_CFG_PATH + configFileName;
    }
    return CLOUD_CFG_PATH + configFileName;
}

inline std::string GetConfigFilePathByVersion(const std::string& localVer, const std::string& cloudVer,
    const std::string& configFileName)
{
    std::string cloudConfigFilePath = GetConfigFilePath(configFileName, false);
    if ((cloudVer > localVer) && FileUtil::FileExists(cloudConfigFilePath)) {
        return cloudConfigFilePath;
    }
    return GetConfigFilePath(configFileName, true);
}

inline std::string GetConfigVersion(bool isLocal)
{
    std::string versionFile = GetConfigFilePath(CFG_VERSION_FILE_NAME, isLocal);
    std::string version;
    (void)FileUtil::LoadStringFromFile(versionFile, version);
    return version;
}

bool CopyConfigVersionFile(const std::string& destConfigDir, bool isLocal)
{
    if (!FileUtil::FileExists(destConfigDir) && !FileUtil::ForceCreateDirectory(destConfigDir)) {
        HIVIEW_LOGE("%{public}s isn't exist and failed to create it", destConfigDir.c_str());
        return false;
    }
    std::string originVersionFile = GetConfigFilePath(CFG_VERSION_FILE_NAME, isLocal);
    std::string destVersionFile = destConfigDir + CFG_VERSION_FILE_NAME;
    if (FileUtil::CopyFile(originVersionFile, destVersionFile) != 0) {
        HIVIEW_LOGE("failed to copy %{public}s to %{public}s", originVersionFile.c_str(), destVersionFile.c_str());
        return false;
    }
    return true;
}

inline std::string GetUnZipConfigDir()
{
    return std::string(CLOUD_CFG_PATH) + LOCAL_CFG_UNZIP_DIR;
}

bool UnZipConfigFile(const std::string& srcDir, const std::string& zipFileName, const std::string& destDir,
    const std::string& configFileName)
{
    if (!FileUtil::FileExists(destDir) && !FileUtil::ForceCreateDirectory(destDir)) {
        HIVIEW_LOGE("%{public}s isn't exist and failed to create it", destDir.c_str());
        return false;
    }
    std::string zipConfigFile = srcDir + zipFileName;
    if (!FileUtil::FileExists(zipConfigFile)) {
        HIVIEW_LOGW("%{public}s isn't exist", zipConfigFile.c_str());
        std::string srcFile = srcDir + configFileName;
        std::string destFile = destDir + configFileName;
        if (FileUtil::CopyFile(srcFile, destFile) != 0) {
            HIVIEW_LOGE("failed to copy %{public}s to %{public}s", srcFile.c_str(), destFile.c_str());
            return false;
        }
        return true;
    }
    HiviewUnzipUnit unzipUnit(zipConfigFile, destDir);
    if (!unzipUnit.UnzipFile()) {
        HIVIEW_LOGE("failed to unzip %{public}s to %{public}s", zipConfigFile.c_str(), destDir.c_str());
        return false;
    }
    return true;
}
}

std::string GetConfigFilePath(const std::string& configFileName)
{
    std::string localVer = GetConfigVersion(true);
    std::string cloudVer = GetConfigVersion(false);
    return GetConfigFilePathByVersion(localVer, cloudVer, configFileName);
}

std::string GetConfigFilePath(const std::string& configZipFileName, const std::string& configDir,
    const std::string& configFileName)
{
    std::string destConfigDir = FileUtil::IncludeTrailingPathDelimiter(GetUnZipConfigDir() + configDir);
    std::string destVer;
    (void)FileUtil::LoadStringFromFile(destConfigDir + CFG_VERSION_FILE_NAME, destVer);

    std::string localVer = GetConfigVersion(true);
    std::string cloudVer = GetConfigVersion(false);
    std::string destConfigFilePath = destConfigDir + configFileName;
    HIVIEW_LOGI("versions:[%{public}s|%{public}s|%{public}s]", localVer.c_str(), cloudVer.c_str(), destVer.c_str());
    // if dest version is newest, return dest config file path directly
    if (!destVer.empty() && destVer >= std::max(cloudVer, localVer) && FileUtil::FileExists(destConfigFilePath)) {
        return destConfigFilePath;
    }
    // do cloud update if cloud version is newer than local version
    if ((cloudVer > localVer) &&
        UnZipConfigFile(CLOUD_CFG_PATH, configZipFileName, destConfigDir, configFileName) &&
        CopyConfigVersionFile(destConfigDir, false)) {
        HIVIEW_LOGI("succeed to do cloud update for %{public}s", configFileName.c_str());
        return destConfigFilePath;
    }
    // do local update if local version is newer than clouad version or cloud update is failed
    if (UnZipConfigFile(LOCAL_CFG_PATH, configZipFileName, destConfigDir, configFileName) &&
        CopyConfigVersionFile(destConfigDir, true)) {
        HIVIEW_LOGI("succeed to do local update for %{public}s", configFileName.c_str());
        return destConfigFilePath;
    }
    return GetConfigFilePathByVersion(localVer, cloudVer, configFileName);
}
} // namespace ConfigUtil
} // namespace HiviewDFX
} // namespace OHOS
