/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#include "logger.h"
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
    "MANIFEST.MF"
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

    if (!VerifyConfigFiles()) {
        HIVIEW_LOGE("VerifyParamFile error, param is invalid");
        return;
    }

    CopyConfigFiles();
    OnUpdateNotice(LOCAL_CFG_PATH, CLOUD_CFG_PATH);
};

bool ParamManager::CopyConfigFiles()
{
    std::vector<std::string> files;
    FileUtil::GetDirFiles(CFG_PATH, files, false);
    for (std::string file : files) {
        std::string name = file.substr(file.rfind("/") + 1);
        if (IsFileNeedIgnore(name)) {
            continue;
        }
        std::string dest = CLOUD_CFG_PATH + "/" + name;
        int flag = FileUtil::CopyFile(file, dest);
        if (flag != 0) {
            HIVIEW_LOGI("copy file failed:%{public}s", name.c_str());
        }
    }
    return true;
}

bool ParamManager::VerifyConfigFiles()
{
    std::vector<std::string> files;
    FileUtil::GetDirFiles(CFG_PATH, files, false);
    for (std::string file : files) {
        std::string name = file.substr(file.rfind("/") + 1);
        if (IsFileNeedIgnore(name)) {
            continue;
        }
        if (!(ParamReader::VerifyParamFile(name))) {
            HIVIEW_LOGE("VerifyParamFile error:%{public}s", name.c_str());
            return false;
        }
    }
    return true;
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
