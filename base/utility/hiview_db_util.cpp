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
#include "hiview_db_util.h"

#include <algorithm>

#include "file_util.h"
#include "hiview_logger.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiviewDbUtil {
DEFINE_LOG_TAG("HiviewDbUtil");
namespace {
bool IsDbFile(const std::string& dbPath)
{
    std::string dbFileName = FileUtil::ExtractFileName(dbPath);
    std::string dbFileExt = FileUtil::ExtractFileExt(dbFileName);
    return dbFileExt == "db";
}

void ClearDbFilesByTimestampOrder(const std::vector<std::string>& dbFiles, uint32_t maxFileNum)
{
    uint32_t numOfCleanFiles = dbFiles.size() - maxFileNum;
    for (size_t i = 0; i < numOfCleanFiles; i++) {
        HIVIEW_LOGI("start to clear db file=%{public}s", dbFiles[i].c_str());
        if (!FileUtil::RemoveFile(dbFiles[i])) {
            HIVIEW_LOGW("failed to delete db file=%{public}s", dbFiles[i].c_str());
        }
    }
}
}

std::string CreateFileNameByDate(const std::string& prefix)
{
    // xxx_yyyymmdd.db
    std::string dbFileName;
    std::string dateStr = TimeUtil::TimestampFormatToDate(TimeUtil::GetSeconds(), "%Y%m%d");
    dbFileName.append(prefix).append(dateStr).append(".db");
    return dbFileName;
}

std::string GetDateFromDbFile(const std::string& dbPath)
{
    // /data/../xxx_yyyymmdd.db
    if (dbPath.empty()) {
        return "";
    }

    std::string dbFileName = FileUtil::ExtractFileName(dbPath);
    size_t startPos = dbFileName.find("_");
    if (startPos == std::string::npos) {
        return "";
    }
    size_t endPos = dbFileName.rfind(".");
    if (endPos == std::string::npos) {
        return "";
    }
    return startPos < endPos ? dbFileName.substr(startPos + 1, endPos - startPos - 1) : "";
}

bool InitDbUploadPath(const std::string& dbPath, std::string& uploadPath)
{
    if (!uploadPath.empty()) {
        return true;
    }

    if (dbPath.empty()) {
        HIVIEW_LOGE("db path is empty");
        return false;
    }

    const std::string uploadDirName = "upload";
    std::string tmpUploadPath = FileUtil::IncludeTrailingPathDelimiter(
        FileUtil::ExtractFilePath(dbPath)).append(uploadDirName);
    if (!FileUtil::IsDirectory(tmpUploadPath) && !FileUtil::ForceCreateDirectory(tmpUploadPath)) {
        HIVIEW_LOGE("failed to create upload dir=%{public}s", tmpUploadPath.c_str());
        return false;
    }
    uploadPath = tmpUploadPath;
    HIVIEW_LOGI("init db upload path=%{public}s", uploadPath.c_str());
    return true;
}

void MoveDbFilesToUploadDir(const std::string& dbPath, const std::string& uploadPath)
{
    std::vector<std::string> dbFiles;
    FileUtil::GetDirFiles(FileUtil::ExtractFilePath(dbPath), dbFiles, false);
    for (auto& dbFile : dbFiles) {
        // upload only xxx.db, and delete xxx.db-shm/xxx.db-wal
        if (IsDbFile(dbFile)) {
            MoveDbFileToUploadDir(dbFile, uploadPath);
            continue;
        }
        // skip upload dir
        if (dbFile.find("/upload") != std::string::npos) {
            continue;
        }
        if (!FileUtil::RemoveFile(dbFile)) {
            HIVIEW_LOGW("failed to remove db file=%{public}s", dbFile.c_str());
            continue;
        }
        HIVIEW_LOGI("succ to remove db file=%{public}s", dbFile.c_str());
    }
}

void MoveDbFileToUploadDir(const std::string dbPath, const std::string& uploadPath)
{
    std::string uploadFilePath = FileUtil::IncludeTrailingPathDelimiter(uploadPath)
        .append(FileUtil::ExtractFileName(dbPath));
    HIVIEW_LOGI("start to move db file, src=%{public}s, dst=%{public}s", dbPath.c_str(), uploadFilePath.c_str());
    if (FileUtil::CopyFile(dbPath, uploadFilePath) != 0) {
        HIVIEW_LOGW("failed to copy db file=%{public}s", dbPath.c_str());
        return;
    }
    if (!FileUtil::RemoveFile(dbPath)) {
        HIVIEW_LOGW("failed to delete db file=%{public}s", dbPath.c_str());
    }
}

void TryToAgeUploadDbFiles(const std::string& uploadPath, uint32_t maxFileNum)
{
    std::vector<std::string> dbFiles;
    FileUtil::GetDirFiles(uploadPath, dbFiles);
    if (dbFiles.size() <= maxFileNum) {
        return;
    }
    HIVIEW_LOGI("start to clean db files, size=%{public}zu", dbFiles.size());
    std::sort(dbFiles.begin(), dbFiles.end());
    ClearDbFilesByTimestampOrder(dbFiles, maxFileNum);
}
} // namespace HiviewDbUtil
} // namespace HiviewDFX
} // namespace OHOS
