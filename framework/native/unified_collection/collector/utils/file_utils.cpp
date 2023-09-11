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
#include "logger.h"
#include "string_util.h"

DEFINE_LOG_TAG("UCollectUtil");

using namespace OHOS::HiviewDFX::Hitrace;
using namespace OHOS::HiviewDFX::UCollectUtil;

namespace OHOS {
namespace HiviewDFX {

class CleanPolicy {
public:
    explicit CleanPolicy(int type) : type_(type) {};
    virtual ~CleanPolicy() {};

public:
    virtual bool IsMine(const std::string &fileName) = 0;
    virtual uint32_t MyThreshold() = 0;

    void LoadAllFiles(std::vector<std::string> &files);
    void DoClean();

private:
    int type_;
};

void CleanPolicy::LoadAllFiles(std::vector<std::string> &files)
{
    // set path
    std::string path;
    if (type_ == share) {
        path = UnifiedPath::UNIFIED_SHARE_PATH;
    } else {
        path = UnifiedPath::UNIFIED_SPECIAL_PATH;
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
    std::map<uint64_t, std::string> myFiles;
    // create file time
    std::vector<uint64_t> fileTime;
    for (const auto &file : files) {
        if (IsMine(file)) {
            struct stat fileInfo;
            stat(file.c_str(), &fileInfo);
            fileTime.emplace_back(fileInfo.st_mtime);
            myFiles.insert(std::pair<uint64_t, std::string>(fileInfo.st_mtime, file));
        }
    }

    // sort
    std::sort(fileTime.begin(), fileTime.end());
    HIVIEW_LOGD("fileTime size : %{public}d, myFiles size : %{public}d.", fileTime.size(), myFiles.size());

    // Clean up old files
    while (fileTime.size() > MyThreshold()) {
        uint64_t delFileTime = fileTime.front();
        std::string delFileName = myFiles[delFileTime];
        FileUtil::RemoveFile(delFileName);
        fileTime.erase(fileTime.begin());
        if (fileTime.size() == 0) {
            return;
        }
    }
}

class ShareCleanPolicy : public CleanPolicy {
public:
    ShareCleanPolicy(int type) : CleanPolicy(type) {}
    ~ShareCleanPolicy() {}

public:
    bool IsMine(const std::string &fileName)
    {
        return true;
    }
    uint32_t MyThreshold()
    {
        return TraceCount::UNIFIED_SHARE_COUNTS;
    }
};

class SpecialXperfCleanPolicy : public CleanPolicy {
public:
    explicit SpecialXperfCleanPolicy(int type) : CleanPolicy(type) {}
    ~SpecialXperfCleanPolicy() {}

public:
    bool IsMine(const std::string &fileName)
    {
        // check xperf trace
        size_t posXperf = fileName.find(TraceCaller::XPERF);
        if (posXperf == std::string::npos) {
            return false;
        }
        return true;
    }
    uint32_t MyThreshold()
    {
        return TraceCount::UNIFIED_SPECIAL_XPERF;
    }
};

class SpecialOtherCleanPolicy : public CleanPolicy {
public:
    explicit SpecialOtherCleanPolicy(int type) : CleanPolicy(type) {}
    ~SpecialOtherCleanPolicy() {}

public:
    bool IsMine(const std::string &fileName)
    {
        // check Betaclub and other trace
        size_t posBeta = fileName.find(TraceCaller::BETACLUB);
        size_t posOther = fileName.find(TraceCaller::OTHER);
        if (posBeta == std::string::npos && posOther == std::string::npos) {
            return false;
        }
        return true;
    }
    uint32_t MyThreshold()
    {
        return TraceCount::UNIFIED_SPECIAL_OTHER;
    }
};

std::shared_ptr<CleanPolicy> GetCleanPolicy(int type, TraceCollector::Caller &caller)
{
    if (type == share) {
        return std::make_shared<ShareCleanPolicy>(type);
    }

    if (caller == TraceCollector::Caller::XPERF) {
        return std::make_shared<SpecialXperfCleanPolicy>(type);
    }
    return std::make_shared<SpecialOtherCleanPolicy>(type);
}

void FileRemove(TraceCollector::Caller &caller)
{
    std::shared_ptr<CleanPolicy> shareCleaner = GetCleanPolicy(share, caller);
    std::shared_ptr<CleanPolicy> specialCleaner = GetCleanPolicy(special, caller);
    switch (caller) {
        case TraceCollector::Caller::RELIABILITY:
        case TraceCollector::Caller::XPOWER:
            shareCleaner->DoClean();
            break;
        case TraceCollector::Caller::XPERF:
            shareCleaner->DoClean();
            specialCleaner->DoClean();
            break;
        default:
            specialCleaner->DoClean();
            break;
    }
    return;
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
            return TraceCaller::RELIABILITY;
        case TraceCollector::Caller::XPERF:
            return TraceCaller::XPERF;
        case TraceCollector::Caller::XPOWER:
            return TraceCaller::XPOWER;
        case TraceCollector::Caller::BETACLUB:
            return TraceCaller::BETACLUB;
        default:
            return TraceCaller::OTHER;
    }
}

std::vector<std::string> GetUnifiedFiles(TraceRetInfo ret, TraceCollector::Caller &caller)
{
    if (EnumToString(caller) == TraceCaller::OTHER || EnumToString(caller) == TraceCaller::BETACLUB) {
        return GetUnifiedSpecialFiles(ret, caller);
    } else {
        return GetUnifiedShareFiles(ret, caller);
    }
}

bool IsTraceExists(const std::string &trace)
{
    std::vector<std::string> files;
    FileUtil::GetDirFiles(UnifiedPath::UNIFIED_SHARE_PATH, files);
    for (const auto &file : files) {
        std::string revRightStr = StringUtil::GetRrightSubstr(trace, "/");
        size_t posMatch = file.find(revRightStr);
        if (posMatch != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Save three traces for xperf
void CopyXperfToSpecialPath(const std::string &trace, const std::string &revRightStr)
{
    std::string dst = UnifiedPath::UNIFIED_SPECIAL_PATH + TraceCaller::XPERF
        + "_" + revRightStr;
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

    if (!FileUtil::FileExists(UnifiedPath::UNIFIED_SHARE_PATH)) {
        if (!CreateMultiDirectory(UnifiedPath::UNIFIED_SHARE_PATH)) {
            HIVIEW_LOGE("failed to create multidirectory.");
            return {};
        }
    }

    std::vector<std::string> files;
    for (const auto &trace : ret.outputFiles) {
        std::string revRightStr = StringUtil::GetRrightSubstr(trace, "/");
        // check trace exists or not
        if (IsTraceExists(trace)) {
            continue;
        }

        // copy xperf trace to */trace/special/, reserve 3 trace in */trace/special/
        if (EnumToString(caller) == TraceCaller::XPERF) {
            CopyXperfToSpecialPath(trace, revRightStr);
        }
        const std::string dst = UnifiedPath::UNIFIED_SHARE_PATH + revRightStr;
        // for copy
        FileUtil::CopyFile(trace, dst);
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

    if (!FileUtil::FileExists(UnifiedPath::UNIFIED_SPECIAL_PATH)) {
        if (!CreateMultiDirectory(UnifiedPath::UNIFIED_SPECIAL_PATH)) {
            HIVIEW_LOGE("Failed to dump trace, error code (%{public}d).", ret.errorCode);
            return {};
        }
    }

    std::vector<std::string> files;
    for (const auto &trace : ret.outputFiles) {
        std::string revRightStr = StringUtil::GetRrightSubstr(trace, "/");
        const std::string dst = UnifiedPath::UNIFIED_SPECIAL_PATH + EnumToString(caller) + "_" + revRightStr;

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
