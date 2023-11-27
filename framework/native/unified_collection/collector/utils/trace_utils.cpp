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
#include <algorithm>
#include <sys/stat.h>
#include <vector>

#include "file_util.h"
#include "trace_utils.h"
#include "logger.h"
#include "string_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
DEFINE_LOG_TAG("UCollectUtil-TraceCollector");
const std::string UNIFIED_SHARE_PATH = "/data/log/hiview/unified_collection/trace/share/";
const std::string UNIFIED_SPECIAL_PATH = "/data/log/hiview/unified_collection/trace/special/";
const std::string RELIABILITY = "Reliability";
const std::string XPERF = "Xperf";
const std::string XPOWER = "Xpower";
const std::string BETACLUB = "BetaClub";
const std::string OTHER = "Other";
const uint32_t UNIFIED_SHARE_COUNTS = 20;
const uint32_t UNIFIED_SPECIAL_XPERF = 3;
const uint32_t UNIFIED_SPECIAL_RELIABILITY = 3;
const uint32_t UNIFIED_SPECIAL_OTHER = 5;
}

enum {
    SHARE = 0,
    SPECIAL = 1,
};

UcError TransCodeToUcError(TraceErrorCode ret)
{
    if (CODE_MAP.find(ret) == CODE_MAP.end()) {
        HIVIEW_LOGE("ErrorCode is not exists.");
        return UcError::UNSUPPORT;
    } else {
        return CODE_MAP.at(ret);
    }
}

class CleanPolicy {
public:
    explicit CleanPolicy(int type) : type_(type) {}
    virtual ~CleanPolicy() {}
    void DoClean();

protected:
    virtual bool IsMine(const std::string &fileName) = 0;
    virtual uint32_t MyThreshold() = 0;

private:
    void LoadAllFiles(std::vector<std::string> &files);
    int type_;
};

void CleanPolicy::LoadAllFiles(std::vector<std::string> &files)
{
    // set path
    std::string path;
    if (type_ == SHARE) {
        path = UNIFIED_SHARE_PATH;
    } else {
        path = UNIFIED_SPECIAL_PATH;
    }
    // Load all files under the path
    FileUtil::GetDirFiles(path, files);
}

void CleanPolicy::DoClean()
{
    // Load all files under the path
    std::vector<std::string> files;
    LoadAllFiles(files);

    // Filter files that belong to me
    std::map<uint64_t, std::vector<std::string>> myFiles;
    for (const auto &file : files) {
        if (IsMine(file)) {
            struct stat fileInfo;
            stat(file.c_str(), &fileInfo);
            std::vector<std::string> fileLists;
            if (myFiles.find(fileInfo.st_mtime) != myFiles.end()) {
                fileLists = myFiles[fileInfo.st_mtime];
                fileLists.push_back(file);
                myFiles[fileInfo.st_mtime] = fileLists;
            } else {
                fileLists.push_back(file);
                myFiles.insert(std::pair<uint64_t, std::vector<std::string>>(fileInfo.st_mtime, fileLists));
            }
        }
    }

    HIVIEW_LOGD("myFiles size : %{public}d.", myFiles.size());

    // Clean up old files
    while (myFiles.size() > MyThreshold()) {
        for (const auto &file : myFiles.begin()->second) {
            FileUtil::RemoveFile(file);
        }
        myFiles.erase(myFiles.begin());
    }
}

class ShareCleanPolicy : public CleanPolicy {
public:
    explicit ShareCleanPolicy(int type) : CleanPolicy(type) {}
    ~ShareCleanPolicy() override {}

protected:
    bool IsMine(const std::string &fileName) override
    {
        return true;
    }

    uint32_t MyThreshold() override
    {
        return UNIFIED_SHARE_COUNTS;
    }
};

class SpecialXperfCleanPolicy : public CleanPolicy {
public:
    explicit SpecialXperfCleanPolicy(int type) : CleanPolicy(type) {}
    ~SpecialXperfCleanPolicy() override {}

protected:
    bool IsMine(const std::string &fileName) override
    {
        // check xperf trace
        size_t posXperf = fileName.find(XPERF);
        return posXperf != std::string::npos;
    }

    uint32_t MyThreshold() override
    {
        return UNIFIED_SPECIAL_XPERF;
    }
};

class SpecialReliabilityCleanPolicy : public CleanPolicy {
public:
    explicit SpecialReliabilityCleanPolicy(int type) : CleanPolicy(type) {}
    ~SpecialReliabilityCleanPolicy() override {}

protected:
    bool IsMine(const std::string &fileName) override
    {
        // check Reliability trace
        size_t posReliability = fileName.find(RELIABILITY);
        return posReliability != std::string::npos;
    }

    uint32_t MyThreshold() override
    {
        return UNIFIED_SPECIAL_RELIABILITY;
    }
};

class SpecialOtherCleanPolicy : public CleanPolicy {
public:
    explicit SpecialOtherCleanPolicy(int type) : CleanPolicy(type) {}
    ~SpecialOtherCleanPolicy() override {}

protected:
    bool IsMine(const std::string &fileName) override
    {
        // check Betaclub and other trace
        size_t posBeta = fileName.find(BETACLUB);
        size_t posOther = fileName.find(OTHER);
        return posBeta != std::string::npos || posOther != std::string::npos;
    }

    uint32_t MyThreshold() override
    {
        return UNIFIED_SPECIAL_OTHER;
    }
};

std::shared_ptr<CleanPolicy> GetCleanPolicy(int type, TraceCollector::Caller &caller)
{
    if (type == SHARE) {
        return std::make_shared<ShareCleanPolicy>(type);
    }

    if (caller == TraceCollector::Caller::XPERF) {
        return std::make_shared<SpecialXperfCleanPolicy>(type);
    }

    if (caller == TraceCollector::Caller::RELIABILITY) {
        return std::make_shared<SpecialReliabilityCleanPolicy>(type);
    }
    return std::make_shared<SpecialOtherCleanPolicy>(type);
}

void FileRemove(TraceCollector::Caller &caller)
{
    std::shared_ptr<CleanPolicy> shareCleaner = GetCleanPolicy(SHARE, caller);
    std::shared_ptr<CleanPolicy> specialCleaner = GetCleanPolicy(SPECIAL, caller);
    switch (caller) {
        case TraceCollector::Caller::XPOWER:
            shareCleaner->DoClean();
            break;
        case TraceCollector::Caller::RELIABILITY:
        case TraceCollector::Caller::XPERF:
            shareCleaner->DoClean();
            specialCleaner->DoClean();
            break;
        default:
            specialCleaner->DoClean();
            break;
    }
}

void CheckAndCreateDirectory(const std::string &tmpDirPath)
{
    if (!FileUtil::FileExists(tmpDirPath)) {
        if (FileUtil::ForceCreateDirectory(tmpDirPath, FileUtil::FILE_PERM_775)) {
            HIVIEW_LOGD("create listener log directory %{public}s succeed.", tmpDirPath.c_str());
        } else {
            HIVIEW_LOGE("create listener log directory %{public}s failed.", tmpDirPath.c_str());
        }
    }
}

bool CreateMultiDirectory(const std::string &dirPath)
{
    uint32_t dirPathLen = dirPath.length();
    if (dirPathLen > PATH_MAX) {
        return false;
    }
    char tmpDirPath[PATH_MAX] = { 0 };
    for (uint32_t i = 0; i < dirPathLen; ++i) {
        tmpDirPath[i] = dirPath[i];
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
            return RELIABILITY;
        case TraceCollector::Caller::XPERF:
            return XPERF;
        case TraceCollector::Caller::XPOWER:
            return XPOWER;
        case TraceCollector::Caller::BETACLUB:
            return BETACLUB;
        default:
            return OTHER;
    }
}

std::vector<std::string> GetUnifiedFiles(TraceRetInfo ret, TraceCollector::Caller &caller)
{
    if (EnumToString(caller) == OTHER || EnumToString(caller) == BETACLUB) {
        return GetUnifiedSpecialFiles(ret, caller);
    } else {
        return GetUnifiedShareFiles(ret, caller);
    }
}

bool IsTraceExists(const std::string &trace)
{
    std::vector<std::string> files;
    FileUtil::GetDirFiles(UNIFIED_SHARE_PATH, files);
    for (const auto &file : files) {
        std::string traceFile = FileUtil::ExtractFileName(trace);
        size_t posMatch = file.find(traceFile);
        return posMatch != std::string::npos;
    }
    return false;
}

// Save three traces for xperf/Reliability
void CopyToSpecialPath(const std::string &trace, const std::string &traceFile, const std::string &traceCaller)
{
    if (!FileUtil::FileExists(UNIFIED_SPECIAL_PATH)) {
        if (!CreateMultiDirectory(UNIFIED_SPECIAL_PATH)) {
            HIVIEW_LOGE("failed to create multidirectory.");
            return;
        }
    }

    std::string dst = UNIFIED_SPECIAL_PATH + traceCaller + "_" + traceFile;
    FileUtil::CopyFile(trace, dst);
}

/*
 * apply to xperf, xpower and reliability
 * trace path eg.:
 *     /data/log/hiview/unified_collection/trace/share/trace_20230906111617@8290-81765922.sys
*/
std::vector<std::string> GetUnifiedShareFiles(TraceRetInfo ret, TraceCollector::Caller &caller)
{
    if (ret.errorCode != TraceErrorCode::SUCCESS) {
        HIVIEW_LOGE("DumpTrace: failed to dump trace, error code (%{public}d).", ret.errorCode);
        return {};
    }

    if (!FileUtil::FileExists(UNIFIED_SHARE_PATH)) {
        if (!CreateMultiDirectory(UNIFIED_SHARE_PATH)) {
            HIVIEW_LOGE("failed to create multidirectory.");
            return {};
        }
    }

    std::vector<std::string> files;
    for (const auto &tracePath : ret.outputFiles) {
        // check trace exists or not
        if (IsTraceExists(tracePath)) {
            continue;
        }

        std::string traceFile = FileUtil::ExtractFileName(tracePath);
        // copy xperf/reliability trace to */trace/special/, reserve 3 trace in */trace/special/
        std::string traceCaller = EnumToString(caller);
        if (traceCaller == XPERF || traceCaller == RELIABILITY) {
            CopyToSpecialPath(tracePath, traceFile, traceCaller);
        }
        const std::string dst = UNIFIED_SHARE_PATH + traceFile;
        // for copy
        FileUtil::CopyFile(tracePath, dst);
        files.push_back(dst);
        HIVIEW_LOGI("trace file : %{public}s.", dst.c_str());
    }

    // file delete
    FileRemove(caller);

    return files;
}

/*
 * apply to BetaClub and Other Scenes
 * trace path eg.:
 * /data/log/hiview/unified_collection/trace/special/BetaClub_trace_20230906111633@8306-299900816.sys
*/
std::vector<std::string> GetUnifiedSpecialFiles(TraceRetInfo ret, TraceCollector::Caller &caller)
{
    if (ret.errorCode != TraceErrorCode::SUCCESS) {
        HIVIEW_LOGE("Failed to dump trace, error code (%{public}d).", ret.errorCode);
        return {};
    }

    if (!FileUtil::FileExists(UNIFIED_SPECIAL_PATH)) {
        if (!CreateMultiDirectory(UNIFIED_SPECIAL_PATH)) {
            HIVIEW_LOGE("Failed to dump trace, error code (%{public}d).", ret.errorCode);
            return {};
        }
    }

    std::vector<std::string> files;
    for (const auto &trace : ret.outputFiles) {
        std::string traceFile = FileUtil::ExtractFileName(trace);
        const std::string dst = UNIFIED_SPECIAL_PATH + EnumToString(caller) + "_" + traceFile;

        // for copy
        FileUtil::CopyFile(trace, dst);
        files.push_back(dst);
        HIVIEW_LOGI("trace file : %{public}s.", dst.c_str());
    }

    // file delete
    FileRemove(caller);

    return files;
}
} // HiViewDFX
} // OHOS
