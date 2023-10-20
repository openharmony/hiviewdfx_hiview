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
#include <chrono>
#include <ctime>
#include <sys/stat.h>
#include <vector>

#include "file_util.h"
#include "trace_utils.h"
#include "logger.h"
#include "string_util.h"

DEFINE_LOG_TAG("UCollectUtil-TraceCollector");

using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace HiviewDFX {
namespace {
    const std::string UNIFIED_SHARE_PATH = "/data/log/hiview/unified_collection/trace/share/";
    const std::string UNIFIED_SPECIAL_PATH = "/data/log/hiview/unified_collection/trace/special/";
    const std::string RELIABILITY = "Reliability";
    const std::string XPERF = "Xperf";
    const std::string XPOWER = "Xpower";
    const std::string BETACLUB = "BetaClub";
    const std::string OTHER = "Other";
    const uint32_t UNIFIED_SHARE_COUNTS = 20;
    const uint32_t UNIFIED_SPECIAL_XPERF = 3;
    const uint32_t UNIFIED_SPECIAL_OTHER = 5;
    const int64_t XPERF_SIZE = 1835008000;       // 1750 * 1024 * 1024
    const int64_t XPOWER_SIZE = 734003200;       // 700 * 1024 * 1024
    const int64_t RELIABILITY_SIZE = 367001600;  // 350 * 1024 * 1024
    const int64_t SECOND_PER_DAY = 24 * 60 * 60;
    const uint32_t DB_LENGTH = 4;
    const float TEN_PERCENT_LIMIT = 0.1;
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
    return std::make_shared<SpecialOtherCleanPolicy>(type);
}

void FileRemove(TraceCollector::Caller &caller)
{
    std::shared_ptr<CleanPolicy> shareCleaner = GetCleanPolicy(SHARE, caller);
    std::shared_ptr<CleanPolicy> specialCleaner = GetCleanPolicy(SPECIAL, caller);
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
}

void CreateTracePath(const std::string &filePath)
{
    if (!FileUtil::FileExists(filePath)) {
        if (!CreateMultiDirectory(filePath)) {
            HIVIEW_LOGE("failed to create multidirectory %{public}s.", filePath.c_str());
            return;
        }
    }
}

void ControlPolicy::InitTraceData()
{
    std::vector<uint64_t> values = QueryDb();
    HIVIEW_LOGI("trace storage: values.size() = %{public}d.", values.size());
    for (auto value : values) {
        HIVIEW_LOGI("trace storage: value = %{public}d.", value);
    }
    if (values.size() != DB_LENGTH) {
        HIVIEW_LOGE("trace storage: db is destoryed.");
        return;
    }
    systemTime_ = values[0];
    xperfSize_ = values[1];
    xpowerSize_ = values[2];
    reliabilitySize_ = values[3];
}

void ControlPolicy::InitTraceStorage()
{
    CreateTracePath(UNIFIED_SHARE_PATH);
    CreateTracePath(UNIFIED_SPECIAL_PATH);

    traceStorage_ = std::make_shared<TraceStorage>();
}

ControlPolicy::ControlPolicy()
{
    InitTraceStorage();
    InitTraceData();
}

bool ControlPolicy::NeedDump(TraceCollector::Caller &caller)
{
    int64_t nowDays = GetDate();
    HIVIEW_LOGI("start to dump, nowDays = %{public}d, systemTime_ = %{public}d.", nowDays, systemTime_);
    if (nowDays != systemTime_) {
        UpdateTraceStorage(nowDays, 0, 0, 0);
        return true;
    }

    switch (caller) {
        case TraceCollector::Caller::RELIABILITY:
            return reliabilitySize_ < RELIABILITY_SIZE;
        case TraceCollector::Caller::XPERF:
            return xperfSize_ < XPERF_SIZE;
        case TraceCollector::Caller::XPOWER:
            return xpowerSize_ < XPOWER_SIZE;
        default:
            return true;
    }
}

bool ControlPolicy::NeedUpload(TraceCollector::Caller &caller, TraceRetInfo ret)
{
    int64_t traceSize = GetTraceSize(ret);
    HIVIEW_LOGI("start to upload , systemTime_ = %{public}d, traceSize = %{public}d.", systemTime_, traceSize);
    switch (caller) {
        case TraceCollector::Caller::RELIABILITY:
            if (IsLowerLimit(reliabilitySize_, traceSize, RELIABILITY_SIZE)) {
                reliabilitySize_ = reliabilitySize_ + traceSize;
                return true;
            } else {
                return false;
            }
        case TraceCollector::Caller::XPERF:
            if (IsLowerLimit(xperfSize_, traceSize, XPERF_SIZE)) {
                xperfSize_ = xperfSize_ + traceSize;
                return true;
            } else {
                return false;
            }
        case TraceCollector::Caller::XPOWER:
            if (IsLowerLimit(xpowerSize_, traceSize, XPOWER_SIZE)) {
                xpowerSize_ = xpowerSize_ + traceSize;
                return true;
            } else {
                return false;
            }
        default:
            return true;
    }
}

bool ControlPolicy::IsLowerLimit(int64_t nowSize, int64_t traceSize, int64_t limitSize)
{
    if (limitSize == 0) {
        HIVIEW_LOGE("error, limit size is zero.");
        return false;
    }

    int64_t totalSize = nowSize + traceSize;
    if (totalSize < limitSize) {
        return true;
    }

    float limit = static_cast<float>(totalSize - limitSize) / limitSize;
    if (limit > TEN_PERCENT_LIMIT) {
        return false;
    }
    return true;
}

void ControlPolicy::UpdateTraceStorage(int64_t systemTime, int64_t xperfSize,
                                       int64_t xpowerSize, int64_t reliabilitySize)
{
    HIVIEW_LOGI("systemTime_ = %{public}d.", systemTime);
    systemTime_ = systemTime;
    xperfSize_ = xperfSize;
    xpowerSize_ = xpowerSize;
    reliabilitySize_ = reliabilitySize;
}

void ControlPolicy::StoreDb()
{
    struct UcollectionTraceStorage traceCollection;
    traceCollection.system_time = systemTime_;
    traceCollection.xperf_size = xperfSize_;
    traceCollection.xpower_size = xpowerSize_;
    traceCollection.reliability_size = reliabilitySize_;

    HIVIEW_LOGI("storeDb, system_time:%{public}d, xperfSize_:%{public}d, xpowerSize_:%{public}d, reliabilitySize_:%{public}d.",
        systemTime_, xperfSize_, xpowerSize_, reliabilitySize_);
    traceStorage_->Store(traceCollection);
}

int64_t ControlPolicy::GetTraceSize(TraceRetInfo ret)
{
    struct stat fileInfo;
    int64_t traceSize = 0;
    for (const auto &tracePath : ret.outputFiles) {
        stat(tracePath.c_str(), &fileInfo);
        traceSize += fileInfo.st_size;
    }
    return traceSize;
}

int64_t ControlPolicy::GetDate()
{
    auto now = std::chrono::system_clock::now();
    typedef std::chrono::duration<int64_t, std::ratio<SECOND_PER_DAY>> Day;
    Day days = std::chrono::duration_cast<Day>(now.time_since_epoch());
    return days.count();
}

/*
 * values: <int64_t, int64_t, int64_t, int64_t>
 * <system_time, xperf_size, xpower_size, reliability_size>
 */
std::vector<uint64_t> ControlPolicy::QueryDb()
{
    std::vector<uint64_t> values;
    traceStorage_->Query(values);
    return values;
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

// Save three traces for xperf
void CopyXperfToSpecialPath(const std::string &trace, const std::string &traceFile)
{
    if (!FileUtil::FileExists(UNIFIED_SPECIAL_PATH)) {
        if (!CreateMultiDirectory(UNIFIED_SPECIAL_PATH)) {
            HIVIEW_LOGE("failed to create multidirectory.");
            return;
        }
    }

    std::string dst = UNIFIED_SPECIAL_PATH + XPERF + "_" + traceFile;
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
        // copy xperf trace to */trace/special/, reserve 3 trace in */trace/special/
        if (EnumToString(caller) == XPERF) {
            CopyXperfToSpecialPath(tracePath, traceFile);
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
