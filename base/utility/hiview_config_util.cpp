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

#include "file_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace ConfigUtil {
namespace {
constexpr char CFG_VERSION_FILE_NAME[] = "hiview_config_version";
constexpr char LOCAL_CFG_PATH[] = "/system/etc/hiview/";
constexpr char CLOUD_CFG_PATH[] = "/data/system/hiview/";

std::string GetConfigFilePath(const std::string& fileName, bool isCloud)
{
    std::string dir = FileUtil::IncludeTrailingPathDelimiter(LOCAL_CFG_PATH);
    if (isCloud) {
        dir = FileUtil::IncludeTrailingPathDelimiter(CLOUD_CFG_PATH);
    }
    return dir + fileName;
}

std::string GetConfigVersion(bool isCloud)
{
    std::string versionFile = GetConfigFilePath(CFG_VERSION_FILE_NAME, isCloud);
    std::string version;
    (void)FileUtil::LoadStringFromFile(versionFile, version);
    return version;
}
}

std::string GetConfigFilePath(const std::string& fileName)
{
    std::string localVer = GetConfigVersion(false);
    std::string cloudVer = GetConfigVersion(true);
    return GetConfigFilePath(fileName, cloudVer > localVer);
}
} // namespace ConfigUtil
} // namespace HiviewDFX
} // namespace OHOS
