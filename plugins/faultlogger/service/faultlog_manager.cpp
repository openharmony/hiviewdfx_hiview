/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "faultlog_manager.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <fcntl.h>
#include <filesystem>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "defines.h"
#include "file_util.h"
#include "log_store_ex.h"
#include "hiview_logger.h"
#include "time_util.h"

#include "faultlog_database.h"
#include "faultlog_formatter.h"
#include "faultlog_info.h"
#include "faultlog_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr char DEFAULT_FAULTLOG_FOLDER[] = "/data/log/faultlog/faultlogger/";
constexpr int32_t MAX_FAULT_LOG_PER_HAP = 10;
}

DEFINE_LOG_LABEL(0xD002D11, "FaultLogManager");
LogStoreEx::LogFileFilter CreateLogFileFilter(time_t time, int32_t id, int32_t faultLogType, const std::string& module)
{
    LogStoreEx::LogFileFilter filter = [time, id, faultLogType, module](const LogFile &file) {
        FaultLogInfo info = ExtractInfoFromFileName(file.name_);
        if (info.time <= time) {
            return false;
        }

        if ((id != -1) && (info.id != id)) {
            return false;
        }

        if ((faultLogType != 0) && (info.faultLogType != faultLogType)) {
            return false;
        }

        if ((!module.empty()) && (info.module != module)) {
            return false;
        }
        return true;
    };
    return filter;
}

FaultLogManager::~FaultLogManager()
{
    if (faultLogDb_ != nullptr) {
        delete faultLogDb_;
        faultLogDb_ = nullptr;
    }
}

int32_t FaultLogManager::CreateTempFaultLogFile(time_t time, int32_t id, int32_t faultType,
    const std::string &module) const
{
    FaultLogInfo info;
    info.time = time;
    info.id = id;
    info.faultLogType = faultType;
    info.module = module;
    auto fileName = GetFaultLogName(info);
    return store_->CreateLogFile(fileName);
}

void FaultLogManager::Init()
{
    store_ = std::make_unique<LogStoreEx>(DEFAULT_FAULTLOG_FOLDER, true);
    LogStoreEx::LogFileComparator comparator = [](const LogFile &lhs, const LogFile &rhs) {
        FaultLogInfo lhsInfo = ExtractInfoFromFileName(lhs.name_);
        FaultLogInfo rhsInfo = ExtractInfoFromFileName(rhs.name_);
        return lhsInfo.time > rhsInfo.time;
    };
    store_->SetLogFileComparator(comparator);
    store_->Init();
    faultLogDb_ = new FaultLogDatabase(looper_);
}

std::string FaultLogManager::SaveFaultLogToFile(FaultLogInfo &info) const
{
    auto fileName = GetFaultLogName(info);
    std::string filePath = std::string(DEFAULT_FAULTLOG_FOLDER) + fileName;
    if (FileUtil::FileExists(filePath)) {
        HIVIEW_LOGI("logfile %{public}s already exist.", filePath.c_str());
        return "";
    }
    auto fd = store_->CreateLogFile(fileName);
    if (fd < 0) {
        if (access(DEFAULT_FAULTLOG_FOLDER, F_OK) != 0) {
            HIVIEW_LOGE("%{public}s does not exist!!!", DEFAULT_FAULTLOG_FOLDER);
        }
        return "";
    }

    FaultLogger::WriteDfxLogToFile(fd);
    FaultLogger::WriteFaultLogToFile(fd, info.faultLogType, info.sectionMap);
    FaultLogger::WriteLogToFile(fd, info.logPath);
    if (info.sectionMap.count("HILOG") == 1) {
        FileUtil::SaveStringToFd(fd, "\nHiLog:\n");
        FileUtil::SaveStringToFd(fd, info.sectionMap["HILOG"]);
    }
    FaultLogger::LimitCppCrashLog(fd, info.faultLogType);
    close(fd);

    std::string logFile = info.logPath;
    if (logFile != "" && FileUtil::FileExists(logFile)) {
        if (!FileUtil::RemoveFile(logFile)) {
            HIVIEW_LOGW("remove logFile %{public}s failed.", logFile.c_str());
        } else {
            HIVIEW_LOGI("remove logFile %{public}s.", logFile.c_str());
        }
    }
    store_->ClearSameLogFilesIfNeeded(CreateLogFileFilter(0, info.id, info.faultLogType, info.module),
        MAX_FAULT_LOG_PER_HAP);
    info.logPath = std::string(DEFAULT_FAULTLOG_FOLDER) + fileName;
    HIVIEW_LOGI("create log %{public}s", fileName.c_str());
    return fileName;
}

std::list<FaultLogInfo> FaultLogManager::GetFaultInfoList(const std::string& module,
    int32_t id, int32_t faultType, int32_t maxNum) const
{
    std::list<FaultLogInfo> ret;
    if (faultLogDb_ != nullptr) {
        ret = faultLogDb_->GetFaultInfoList(module, id, faultType, maxNum);
        HIVIEW_LOGI("Find %{public}zu fault records for uid:%{public}d type:%{public}d",
            ret.size(), id, faultType);
    }
    return ret;
}

void FaultLogManager::SaveFaultInfoToRawDb(FaultLogInfo& info) const
{
    if (faultLogDb_ != nullptr) {
        faultLogDb_->SaveFaultLogInfo(info);
    }
}

void FaultLogManager::ReduceLogFileListSize(std::list<std::string> &infoVec, int32_t maxNum) const
{
    if ((maxNum < 0) || (infoVec.size() <= static_cast<uint32_t>(maxNum))) {
        return;
    }

    auto begin = infoVec.begin();
    std::advance(begin, maxNum);
    infoVec.erase(begin, infoVec.end());
}

std::list<std::string> FaultLogManager::GetFaultLogFileList(const std::string &module, time_t time, int32_t id,
                                                            int32_t faultType, int32_t maxNum) const
{
    LogStoreEx::LogFileFilter filter = CreateLogFileFilter(time, id, faultType, module);
    auto vec = store_->GetLogFiles(filter);
    std::list<std::string> ret;
    std::transform(vec.begin(), vec.end(), std::back_inserter(ret), [](const LogFile &file) { return file.path_; });
    ReduceLogFileListSize(ret, maxNum);
    return ret;
}

bool FaultLogManager::GetFaultLogContent(const std::string &name, std::string &content) const
{
    auto path = std::string(DEFAULT_FAULTLOG_FOLDER);
    std::string matchName;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        std::filesystem::path filePath = entry.path();
        std::string fileName = filePath.filename().string();
        if (fileName.find(name) != std::string::npos && matchName.compare(fileName) < 0) {
            matchName = fileName;
            continue;
        }
    }
    if (matchName.empty()) {
        return false;
    }
    path = path + matchName;
    return FileUtil::LoadStringFromFile(path, content);
}

bool FaultLogManager::IsProcessedFault(int32_t pid, int32_t uid, int32_t faultType)
{
    if (faultLogDb_ == nullptr) {
        return false;
    }

    return faultLogDb_->IsFaultExist(pid, uid, faultType);
}
} // namespace HiviewDFX
} // namespace OHOS
