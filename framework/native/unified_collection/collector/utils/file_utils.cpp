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
#include "file_utils.h"
#include "zip_utils.h"
#include "string_util.h"

using namespace std;
using namespace OHOS::HiviewDFX::Hitrace;
using namespace OHOS::HiviewDFX::UCollectUtil;

namespace OHOS {
namespace HiviewDFX {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D03, "Hiview-FileUtils" };
void CheckAndCreateDirectory(char* tmpDirPath)
{
    if (!FileUtil::FileExists(tmpDirPath)) {
        if (FileUtil::ForceCreateDirectory(tmpDirPath, FileUtil::FILE_PERM_775)) {
            HiLog::Debug(LABEL, "create listener log directory %{public}s succeed.", tmpDirPath);
        } else {
            HiLog::Error(LABEL, "create listener log directory %{public}s failed.", tmpDirPath);
        }
    }
}

bool CreateMultiDirectory(const std::string &directoryPath)
{
    uint32_t dirPathLen = directoryPath.length();
    if (dirPathLen > PATH_MAX) {
        return false;
    }
    char tmpDirPath[PATH_MAX] = { 0 };
    for (uint32_t i = 0; i < dirPathLen; ++i) {
        tmpDirPath[i] = directoryPath[i];
        if (tmpDirPath[i] == '/') {
            CheckAndCreateDirectory(tmpDirPath);
        }
    }
    return true;
}

const std::string EnumToString(TraceCollector::Caller &caller)
{
    switch (caller) {
        case TraceCollector::Caller::RELIABILITY:
            return "Reliability";
        case TraceCollector::Caller::XPERF:
            return "Xperf";
        case TraceCollector::Caller::XPOWER:
            return "Xpower";
        case TraceCollector::Caller::BETACLUB:
            return "BetaClub";
        default:
            return "Other";
    }
}

std::vector<std::string> GetUnifiedFiles(TraceRetInfo ret, TraceCollector::Caller &caller)
{
    if (EnumToString(caller) == "Other" || EnumToString(caller) == "BetaClub") {
        return GetUnifiedSpecialFiles(ret, caller);
    } else {
        return GetUnifiedShareFiles(ret, caller);
    }
}

bool CheckTraceIsExists(const std::string &trace)
{
    std::vector<std::string> files;
    FileUtil::GetDirFiles(UNIFIED_SHARE_PATH, files);
    std::string midTrace;
    for (auto it = files.begin(); it != files.end(); it++) {
        midTrace = StringUtil::GetMidSubstr(*it, "_", ".");
        size_t posMatch = trace.find(midTrace);
        if (posMatch != string::npos) {
            return true;
        }
    }
    return false;
}

// zip and copy
std::vector<std::string> GetUnifiedShareFiles(TraceRetInfo ret, TraceCollector::Caller &caller)
{
    if (ret.errorCode != TraceErrorCode::SUCCESS) {
        HiLog::Error(LABEL, "DumpTrace: failed to dump trace, error code (%{public}d).", ret.errorCode);
        return {};
    }

    if (!FileUtil::FileExists(UNIFIED_SHARE_PATH)) {
        if (!CreateMultiDirectory(UNIFIED_SHARE_PATH)) {
            HiLog::Error(LABEL, "failed to create multidirectory.");
            return {};
        }
    }

    std::string rRightStr;
    std::string leftStr;
    std::vector<std::string> files;
    for (const auto &trace : ret.outputFiles) {
        rRightStr = StringUtil::GetRrightSubstr(trace, "/");
        leftStr = StringUtil::GetLeftSubstr(rRightStr, ".");
        HiLog::Info(LABEL, "trace : %{public}s.", trace.c_str());
        // check trace exists in dst or not
        if (CheckTraceIsExists(trace)) {
            continue;
        }

        const std::string dst = UNIFIED_SHARE_PATH + EnumToString(caller) + "_"
            + leftStr + ".zip";
        // for zip copy
        if (!PackFiles(trace, dst)) {
            HiLog::Error(LABEL, "GetUnifiedTraceFiles Error: zip and copy to this dir failed.");
        }
        files.push_back(dst);
        HiLog::Info(LABEL, "%{public}s zip : %{public}s.", UNIFIED_SHARE_PATH.c_str(), dst.c_str());
    }

    return files;
}

// only copy for Betaclub and other scene
std::vector<std::string> GetUnifiedSpecialFiles(TraceRetInfo ret, TraceCollector::Caller &caller)
{
    if (ret.errorCode != TraceErrorCode::SUCCESS) {
        HiLog::Error(LABEL, "error|DumpTrace: failed to dump trace, error code (%{public}d).", ret.errorCode);
        return {};
    }

    if (!FileUtil::FileExists(UNIFIED_SPECIAL_PATH)) {
        if (!CreateMultiDirectory(UNIFIED_SPECIAL_PATH)) {
            HiLog::Error(LABEL, "error|DumpTrace: failed to dump trace, error code (%{public}d).", ret.errorCode);
            return {};
        }
    }

    std::string rRightStr;
    std::vector<std::string> files;
    for (const auto &trace : ret.outputFiles) {
        rRightStr = StringUtil::GetRrightSubstr(trace, "/");
        const std::string dst = UNIFIED_SPECIAL_PATH + EnumToString(caller) + "_" + rRightStr;

        // only for copy
        FileUtil::CopyFile(trace, dst);
        files.push_back(dst);
        HiLog::Info(LABEL, "%{public}s trace file : %{public}s.", UNIFIED_SPECIAL_PATH.c_str(), dst.c_str());
    }

    return files;
}
} // HiViewDFX
} // OHOS
