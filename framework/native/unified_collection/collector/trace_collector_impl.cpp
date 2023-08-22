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
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include "hilog/log.h"
#include "hitrace_dump.h"
#include <iostream>
#include "securec.h"
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <zip.h>

#include "file_util.h"
#include "trace_collector.h"
#include "trace_manager.h"

using namespace std;
using namespace OHOS::HiviewDFX::Hitrace;
using namespace OHOS::HiviewDFX::UCollectUtil;
namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D03, "Hiview-Framework-Collector" };
const std::string UNIFIED_HITRACE_PATH = "/data/log/hiview/unified_collection/trace/share/";
const int READ_MORE_LENGTH = 102400;
constexpr int32_t ERR_CODE = -1;
}

namespace {
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

zipFile CreateZipFile(const std::string& zipPath)
{
    return zipOpen(zipPath.c_str(), APPEND_STATUS_CREATE);
}

void CloseZipFile(zipFile& zipfile)
{
    zipClose(zipfile, nullptr);
}

FILE* GetFileHandle(const std::string& file)
{
    std::string realPath;
    if (!FileUtil::PathToRealPath(file, realPath)) {
        return nullptr;
    }
    return fopen(realPath.c_str(), "rb");
}

int32_t AddFileInZip(zipFile& zipfile, const std::string& srcFile)
{
    zip_fileinfo zipInfo;
    errno_t result = memset_s(&zipInfo, sizeof(zipInfo), 0, sizeof(zipInfo));
    if (result != EOK) {
        HiLog::Error(LABEL, "AddFileInZip memset_s error, file:%{public}s.", srcFile.c_str());
        return ERR_CODE;
    }
    FILE *srcFp = GetFileHandle(srcFile);
    if (srcFp == nullptr) {
        HiLog::Error(LABEL, "get file handle failed:%{public}s, errno: %{public}d.", srcFile.c_str(), errno);
        return ERR_CODE;
    }

    zipOpenNewFileInZip(zipfile, srcFile.c_str(), &zipInfo,
        nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION);

    int errcode = 0;
    char buf[READ_MORE_LENGTH] = {0};
    while (!feof(srcFp)) {
        size_t numBytes = fread(buf, 1, sizeof(buf), srcFp);
        if (numBytes <= 0) {
            HiLog::Error(LABEL, "zip file failed, size is zero.");
            errcode = ERR_CODE;
            break;
        }
        zipWriteInFileInZip(zipfile, buf, static_cast<unsigned int>(numBytes));
        if (ferror(srcFp)) {
            HiLog::Error(LABEL, "zip file failed:%{public}s, errno: %{public}d.", srcFile.c_str(), errno);
            errcode = ERR_CODE;
            break;
        }
    }
    (void)fclose(srcFp);
    zipCloseFileInZip(zipfile);
    return errcode;
}

bool PackFiles(const std::string& fileName, const std::string& zipFileName)
{
    HiLog::Info(LABEL, "start pack file %{public}s to %{public}s.", fileName.c_str(), zipFileName.c_str());
    zipFile compressZip = CreateZipFile(zipFileName);
    if (compressZip == nullptr) {
        HiLog::Error(LABEL, "create zip file failed.");
        return false;
    }
    AddFileInZip(compressZip, fileName);
    CloseZipFile(compressZip);
    return true;
}

std::string EnumToString(TraceCollector::Caller &caller)
{
    switch (caller) {
        case TraceCollector::Caller::RELIABILITY:
            return "RELIABILITY";
        case TraceCollector::Caller::XPERF:
            return "XPERF";
        case TraceCollector::Caller::XPOWER:
            return "XPOWER";
        case TraceCollector::Caller::OTHER:
            return "OTHER";
        default:
            return "NONE";
    }
}

std::vector<std::string> GetUnifiedTraceFiles(TraceRetInfo ret, TraceCollector::Caller &caller)
{
    if (ret.errorCode != TraceErrorCode::SUCCESS) {
        HiLog::Error(LABEL, "DumpTrace: failed to dump trace, error code (%{public}d).", ret.errorCode);
        return {};
    }

    if (!FileUtil::FileExists(UNIFIED_HITRACE_PATH)) {
        if (!CreateMultiDirectory(UNIFIED_HITRACE_PATH)) {
            HiLog::Error(LABEL, "failed to create multidirectory.");
            return {};
        }
    }

    for (auto trace : ret.outputFiles) {
        size_t pos = trace.rfind('/');
        size_t end = trace.rfind('.');
        if (pos == std::string::npos) {
            HiLog::Error(LABEL, "GetUnifiedTraceFiles Error: copy to this dir failed.");
            return {};
        }
        const std::string dst = UNIFIED_HITRACE_PATH + EnumToString(caller) + "_"
            + trace.substr(pos + 1, end - pos - 1) + ".zip";
        HiLog::Info(LABEL, "UNIFIED_HITRACE_PATH zip : %{public}s.", dst.c_str());

        // for zip copy
        if (!PackFiles(trace, dst)) {
            HiLog::Error(LABEL, "GetUnifiedTraceFiles Error: zip and copy to this dir failed.");
        }
    }

    std::vector<std::string> files;
    FileUtil::GetDirFiles(UNIFIED_HITRACE_PATH, files);
    return files;
}
} // namespace

namespace UCollectUtil {
class TraceCollectorImpl : public TraceCollector {
public:
    TraceCollectorImpl() = default;
    virtual ~TraceCollectorImpl() = default;

public:
    virtual CollectResult<std::vector<std::string>> DumpTrace(TraceCollector::Caller &caller) override;
    virtual CollectResult<int32_t> TraceOn() override;
    virtual CollectResult<std::vector<std::string>> TraceOff() override;
};

std::shared_ptr<TraceCollector> TraceCollector::Create()
{
    return std::make_shared<TraceCollectorImpl>();
}

CollectResult<std::vector<std::string>> TraceCollectorImpl::DumpTrace(TraceCollector::Caller &caller)
{
    CollectResult<std::vector<std::string>> result;
    TraceRetInfo ret = OHOS::HiviewDFX::Hitrace::DumpTrace();
    std::vector<std::string> outputFiles = GetUnifiedTraceFiles(ret, caller);
    if (ret.errorCode == TraceErrorCode::SUCCESS) {
        result.data = outputFiles;
        result.retCode = UcError::SUCCESS;
    } else {
        result.retCode = UcError::UPSUPPORT;
    }
    return result;
}

CollectResult<int32_t> TraceCollectorImpl::TraceOn()
{
    CollectResult<int32_t> result;
    if (OHOS::HiviewDFX::Hitrace::DumpTraceOn() == TraceErrorCode::SUCCESS) {
        result.retCode = UcError::SUCCESS;
    } else {
        result.retCode = UcError::UPSUPPORT;
    }
    return result;
}

CollectResult<std::vector<std::string>> TraceCollectorImpl::TraceOff()
{
    CollectResult<std::vector<std::string>> result;
    TraceRetInfo ret = OHOS::HiviewDFX::Hitrace::DumpTraceOff();
    if (ret.errorCode == TraceErrorCode::SUCCESS) {
        result.data = ret.outputFiles;
        result.retCode = UcError::SUCCESS;
    } else {
        result.retCode = UcError::UPSUPPORT;
    }
    return result;
}
} // UCollectUtil
} // HiViewDFX
} // OHOS
