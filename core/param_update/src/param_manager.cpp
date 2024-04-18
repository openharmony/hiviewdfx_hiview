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

#include "param_manager.h"

#include <iostream>
#include <fstream>
#include <set>

#include "hiview_platform.h"
#include "hiview_logger.h"
#include "param_const_common.h"
#include "param_reader.h"
#include "string_util.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("Hiview-ParamUpdate");

namespace {
const std::set<std::string> IGNORE_FILE_LIST = {
    "CERT.ENC",
    "CERT.SF",
    "MANIFEST.MF",
    "version.txt"
};
}

bool ParamManager::IsFileNeedIgnore(const std::string& fileName)
{
    return IGNORE_FILE_LIST.find(fileName) != IGNORE_FILE_LIST.end();
}

void ParamManager::InitParam()
{
    if (!ParamReader::VerifyCertFile()) {
        HIVIEW_LOGE("VerifyCertSfFile error, param is invalid");
        return;
    }

    std::vector<std::string> validFiles;
    GetValidFiles(validFiles);
    if (validFiles.empty()) {
        return;
    }
    CopyConfigFiles(validFiles);
    OnUpdateNotice(LOCAL_CFG_PATH, CLOUD_CFG_PATH);
}

bool ParamManager::CopyFile(const std::string& srcFile, const std::string& dstFile)
{
    std::string dstPath(FileUtil::ExtractFilePath(dstFile));
    // create subdir if not exist
    if (dstPath != CLOUD_CFG_PATH && !FileUtil::ForceCreateDirectory(dstPath)) {
        HIVIEW_LOGW("create dst path failed: %{public}s", dstPath.c_str());
        return false;
    }
    return FileUtil::CopyFile(srcFile, dstFile) == 0;
}

bool ParamManager::CopyConfigFiles(const std::vector<std::string>& files)
{
    for (const std::string& file : files) {
        std::string dstFile(file);
        dstFile.replace(0, CFG_PATH.length(), CLOUD_CFG_PATH);
        if (!CopyFile(file, dstFile)) {
            HIVIEW_LOGI("copy file failed: %{public}s", file.c_str());
        }
    }
    return true;
}

void ParamManager::GetValidFiles(std::vector<std::string>& validFiles)
{
    std::vector<std::string> files;
    FileUtil::GetDirFiles(CFG_PATH, files, true);
    for (const std::string& file : files) {
        std::string fileName(FileUtil::ExtractFileName(file));
        if (IsFileNeedIgnore(fileName)) {
            continue;
        }
        std::string relativedPath(file.substr(CFG_PATH.length()));
        if (!ParamReader::VerifyParamFile(relativedPath)) {
            HIVIEW_LOGE("verify file failed: %{public}s", fileName.c_str());
            validFiles.clear();
            break;
        }
        validFiles.emplace_back(file);
    }
}

void ParamManager::OnUpdateNotice(const std::string& localCfgPath, const std::string& cloudCfgPath)
{
    auto plugins = HiviewPlatform::GetInstance().GetPluginMap();
    for (auto& plugin : plugins) {
        auto businessPlugin = plugin.second;
        if (businessPlugin != nullptr) {
            businessPlugin->OnConfigUpdate(localCfgPath, cloudCfgPath);
        }
    }
};
}
}
