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

#include "param_reader.h"

#include <memory>
#include <iostream>
#include <fstream>

#include "hiview_logger.h"
#include "log_sign_tools.h"
#include "param_const_common.h"
#include "string_util.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("Hiview-ParamUpdate");

namespace {
    const int MIN_SIZE = 2;
}

bool ParamReader::VerifyCertFile()
{
    std::string certFile = CFG_PATH + "CERT.ENC";
    std::string verifyFile = CFG_PATH + "CERT.SF";
    if (!LogSignTools::VerifyFileSign(PUBKEY_PATH, certFile, verifyFile)) {
        HIVIEW_LOGE("verify failed %{public}s,%{public}s, %{public}s", PUBKEY_PATH.c_str(),
            certFile.c_str(), verifyFile.c_str());
        return false;
    }

    std::string manifestFile = CFG_PATH + "MANIFEST.MF";
    std::ifstream file(verifyFile);
    if (!file.good()) {
        HIVIEW_LOGE("Verify is not good");
        return false;
    }
    std::string line;
    std::string sha256Digest;
    std::getline(file, line);
    file.close();
    std::vector<std::string> strs;
    StringUtil::SplitStr(line, ":", strs);
    if (strs.size() < MIN_SIZE) {
        HIVIEW_LOGE("get sha256Digest failed.");
        return false;
    }
    sha256Digest = strs[1];
    StringUtil::TrimStr(sha256Digest);

    std::string manifestDigest = LogSignTools::CalcFileSha256Digest(manifestFile);
    if (sha256Digest == manifestDigest) {
        HIVIEW_LOGI("Verify manifestFile success");
        return true;
    }
    HIVIEW_LOGE("verify cert file failed");
    return false;
};

bool ParamReader::VerifyParamFile(const std::string &filePathStr)
{
    std::string manifestFile = CFG_PATH + "MANIFEST.MF";
    std::ifstream file(manifestFile);
    if (!file.good()) {
        HIVIEW_LOGE("manifestFile is not good");
        return false;
    }
    std::string absFilePath = CFG_PATH + filePathStr;
    std::ifstream paramFile(absFilePath);
    if (!paramFile.good()) {
        HIVIEW_LOGE("paramFile is not good");
        return false;
    }

    std::string sha256Digest;
    std::string line;
    while (std::getline(file, line)) {
        std::string nextline;
        if (line.find("Name: " + filePathStr) != std::string::npos) {
            std::getline(file, nextline);
            std::vector<std::string> strs;
            StringUtil::SplitStr(nextline, ":", strs);
            if (strs.size() < MIN_SIZE) {
                HIVIEW_LOGE("get sha256Digest failed.");
                return false;
            }
            sha256Digest = strs[1];
            StringUtil::TrimStr(sha256Digest);
            break;
        }
    }
    if (sha256Digest.empty()) {
        HIVIEW_LOGE("VerifyParamFile failed, sha256Digest is empty");
        return false;
    }

    std::string sha256Str = LogSignTools::CalcFileSha256Digest(absFilePath);
    if (sha256Digest == sha256Str) {
        HIVIEW_LOGI("VerifyParamFile success");
        return true;
    }
    HIVIEW_LOGE("VerifyParamFile failed");
    return false;
};
} // namespace HiviewDFX
} // namespace OHOS
