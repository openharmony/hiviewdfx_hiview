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

namespace OHOS {
namespace HiviewDFX {
namespace ConfigUtil {
namespace {
constexpr char CFG_VERSION_FILE_NAME[] = "hiview_config_version";
constexpr char LOCAL_CFG_PATH[] = "/system/etc/hiview/";
constexpr char CLOUD_CFG_PATH[] = "/data/system/hiview/";
constexpr char LOCAL_CFG_UNZIP_DIR[] = "unzip_configs/";

inline std::string GetConfigFilePath(const std::string& fileName, bool isLocal)
{
    if (isLocal) {
        return LOCAL_CFG_PATH + fileName;
    }
    return CLOUD_CFG_PATH + fileName;
}

inline std::string GetConfigFilePath(const std::string& localVer, const std::string& cloudVer,
    const std::string& fileName)
{
    std::string cloudConfigFilePath = GetConfigFilePath(fileName, false);
    if ((cloudVer > localVer) && FileUtil::FileExists(cloudConfigFilePath)) {
        return cloudConfigFilePath;
    }
    return GetConfigFilePath(fileName, true);
}

inline std::string GetConfigVersion(bool isLocal)
{
    std::string versionFile = GetConfigFilePath(CFG_VERSION_FILE_NAME, isLocal);
    std::string version;
    (void)FileUtil::LoadStringFromFile(versionFile, version);
    return version;
}

bool CopyConfigVersionFile(bool isLocal, const std::string& destConfigDir)
{
    if (!FileUtil::FileExists(destConfigDir) && !FileUtil::ForceCreateDirectory(destConfigDir)) {
        return false;
    }
    std::string originVersionFile = GetConfigFilePath(CFG_VERSION_FILE_NAME, isLocal);
    if (FileUtil::CopyFile(originVersionFile, destConfigDir + CFG_VERSION_FILE_NAME) != 0) {
        return false;
    }
    return true;
}
}

std::string GetUnZipConfigDir()
{
    return std::string(CLOUD_CFG_PATH) + LOCAL_CFG_UNZIP_DIR;
}

std::string GetConfigFilePath(const std::string& fileName)
{
    std::string localVer = GetConfigVersion(true);
    std::string cloudVer = GetConfigVersion(false);
    return GetConfigFilePath(localVer, cloudVer, fileName);
}

std::string GetConfigFilePathWithHandler(const std::string destFileName, const std::string& destConfigDir,
    ConfigFileHandler configHandler)
{
    std::string destVer;
    (void)FileUtil::LoadStringFromFile(destConfigDir + CFG_VERSION_FILE_NAME, destVer);

    std::string cloudVer = GetConfigVersion(false);
    std::string localVer = GetConfigVersion(true);
    std::string destConfigFilePath = destConfigDir + destFileName;
    if (!destVer.empty() && destVer >= std::max(cloudVer, localVer) && FileUtil::FileExists(destConfigFilePath)) {
        return destConfigFilePath;
    }

    if (configHandler == nullptr) {
        return GetConfigFilePath(localVer, cloudVer, destFileName);
    }

    // do cloud update if cloud version is newer than local version
    if ((cloudVer > localVer) && configHandler(CLOUD_CFG_PATH, destConfigDir, destFileName) &&
        CopyConfigVersionFile(false, destConfigDir)) {
        return destConfigFilePath;
    }
    // do local update if local version is newer than cloud version or cloud update is failed
    if (configHandler(LOCAL_CFG_PATH, destConfigDir, destFileName) &&
        CopyConfigVersionFile(true, destConfigDir)) {
        return destConfigFilePath;
    }
    return GetConfigFilePath(localVer, cloudVer, destFileName);
}
} // namespace ConfigUtil
} // namespace HiviewDFX
} // namespace OHOS
