/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include "cpu_storage.h"

#include <algorithm>
#include <cmath>

#include "file_util.h"
#include "hisysevent.h"
#include "logger.h"
#include "process_status.h"
#include "rdb_helper.h"
#include "sql_util.h"
#include "string_util.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-CpuStorage");
using namespace OHOS::HiviewDFX::UCollectUtil;
namespace {
constexpr int32_t DB_VERSION = 1;
const std::string TABLE_NAME = "unified_collection_cpu";
const std::string COLUMN_START_TIME = "start_time";
const std::string COLUMN_END_TIME = "end_time";
const std::string COLUMN_PID = "pid";
const std::string COLUMN_PROC_NAME = "proc_name";
const std::string COLUMN_PROC_STATE = "proc_state";
const std::string COLUMN_CPU_LOAD = "cpu_load";
const std::string COLUMN_CPU_USAGE = "cpu_usage";
constexpr uint32_t MAX_NUM_OF_DB_FILES = 7; // save files for one week
constexpr uint32_t DEFAULT_PRECISION_OF_DECIMAL = 6; // 0.123456

std::string CreateDbFileName()
{
    std::string dbFileName;
    std::string dateStr = TimeUtil::TimestampFormatToDate(std::time(nullptr), "%Y%m%d");
    dbFileName.append("cpu_stat_").append(dateStr).append(".db"); // cpu_stat_yyyymmdd.db
    return dbFileName;
}

bool NeedCleanDbFiles(const std::vector<std::string>& dbFiles)
{
    return dbFiles.size() > MAX_NUM_OF_DB_FILES;
}

void ClearDbFilesByTimestampOrder(const std::vector<std::string>& dbFiles)
{
    uint32_t numOfCleanFiles = dbFiles.size() - MAX_NUM_OF_DB_FILES;
    for (size_t i = 0; i < numOfCleanFiles; i++) {
        HIVIEW_LOGI("start to clear db file=%{public}s", dbFiles[i].c_str());
        if (!FileUtil::RemoveFile(dbFiles[i])) {
            HIVIEW_LOGW("failed to delete db file=%{public}s", dbFiles[i].c_str());
        }
    }
}

bool IsDbFile(const std::string& dbFilePath)
{
    std::string dbFileName = FileUtil::ExtractFileName(dbFilePath);
    std::string dbFileExt = FileUtil::ExtractFileExt(dbFileName);
    return dbFileExt == "db";
}

bool IsValidProcess(const ProcessCpuStatInfo& cpuCollectionInfo)
{
    return (cpuCollectionInfo.pid > 0) && (!cpuCollectionInfo.procName.empty());
}

bool IsValidCpuLoad(const ProcessCpuStatInfo& cpuCollectionInfo)
{
    constexpr double storeFilteringThresholdOfCpuLoad = 0.0005; // 0.05%
    return cpuCollectionInfo.cpuLoad >= storeFilteringThresholdOfCpuLoad;
}

bool IsInvalidCpuLoad(const ProcessCpuStatInfo& cpuCollectionInfo)
{
    return cpuCollectionInfo.cpuLoad == 0;
}

bool IsValidCpuUsage(const ProcessCpuStatInfo& cpuCollectionInfo)
{
    constexpr double storeFilteringThresholdOfCpuUsage = 0.0005; // 0.05%
    return cpuCollectionInfo.cpuUsage >= storeFilteringThresholdOfCpuUsage;
}

bool NeedStoreInDb(const ProcessCpuStatInfo& cpuCollectionInfo)
{
    if (!IsValidProcess(cpuCollectionInfo)) {
        static uint32_t invalidProcNum = 0;
        invalidProcNum++;
        constexpr uint32_t logLimitNum = 1000;
        if (invalidProcNum % logLimitNum == 0) {
            HIVIEW_LOGW("invalid process num=%{public}u, pid=%{public}d, name=%{public}s",
                invalidProcNum, cpuCollectionInfo.pid, cpuCollectionInfo.procName.c_str());
        }
        return false;
    }
    return IsValidCpuLoad(cpuCollectionInfo)
        || (IsInvalidCpuLoad(cpuCollectionInfo) && IsValidCpuUsage(cpuCollectionInfo));
}

double TruncateDecimalWithNBitPrecision(double decimal, uint32_t precision = DEFAULT_PRECISION_OF_DECIMAL)
{
    auto truncateCoefficient = std::pow(10, precision);
    return std::floor(decimal * truncateCoefficient) / truncateCoefficient;
}

bool IsForegroundStateInCollectionPeriod(const ProcessCpuStatInfo& cpuCollectionInfo)
{
    int32_t pid = cpuCollectionInfo.pid;
    ProcessState procState = ProcessStatus::GetInstance().GetProcessState(pid);
    if (procState == FOREGROUND) {
        return true;
    }
    uint64_t procForegroundTime = ProcessStatus::GetInstance().GetProcessLastForegroundTime(pid);
    return procForegroundTime >= cpuCollectionInfo.startTime;
}

int32_t GetProcessStateInCollectionPeriod(const ProcessCpuStatInfo& cpuCollectionInfo)
{
    return IsForegroundStateInCollectionPeriod(cpuCollectionInfo)
        ? static_cast<int32_t>(FOREGROUND)
        : static_cast<int32_t>(ProcessStatus::GetInstance().GetProcessState(cpuCollectionInfo.pid));
}
}

class CpuDbStoreCallback : public NativeRdb::RdbOpenCallback {
public:
    int OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
};

int CpuDbStoreCallback::OnCreate(NativeRdb::RdbStore& rdbStore)
{
    HIVIEW_LOGD("create dbStore");
    return NativeRdb::E_OK;
}

int CpuDbStoreCallback::OnUpgrade(NativeRdb::RdbStore& rdbStore, int oldVersion, int newVersion)
{
    HIVIEW_LOGD("oldVersion=%{public}d, newVersion=%{public}d", oldVersion, newVersion);
    return NativeRdb::E_OK;
}

CpuStorage::CpuStorage(const std::string& workPath) : workPath_(workPath)
{
    InitDbStorePath();
    InitDbStore();
}

void CpuStorage::InitDbStorePath()
{
    std::string tempDbStorePath = FileUtil::IncludeTrailingPathDelimiter(workPath_);
    const std::string cpuDirName = "cpu";
    tempDbStorePath = FileUtil::IncludeTrailingPathDelimiter(tempDbStorePath.append(cpuDirName));
    if (!FileUtil::IsDirectory(tempDbStorePath) && !FileUtil::ForceCreateDirectory(tempDbStorePath)) {
        HIVIEW_LOGE("failed to create dir=%{public}s", tempDbStorePath.c_str());
        return;
    }
    tempDbStorePath.append(CreateDbFileName());
    dbStorePath_ = tempDbStorePath;
    HIVIEW_LOGI("succ to init db store path=%{public}s", dbStorePath_.c_str());
}

void CpuStorage::InitDbStore()
{
    NativeRdb::RdbStoreConfig config(dbStorePath_);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    CpuDbStoreCallback callback;
    auto ret = NativeRdb::E_OK;
    dbStore_ = NativeRdb::RdbHelper::GetRdbStore(config, DB_VERSION, callback, ret);
    if (ret != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to init db store, db store path=%{public}s", dbStorePath_.c_str());
        dbStore_ = nullptr;
        return;
    }
}

void CpuStorage::Store(const std::vector<ProcessCpuStatInfo>& cpuCollectionInfos)
{
    if (dbStore_ == nullptr) {
        HIVIEW_LOGW("db store is null, path=%{public}s", dbStorePath_.c_str());
        return;
    }

    for (auto& cpuCollectionInfo : cpuCollectionInfos) {
        if (NeedStoreInDb(cpuCollectionInfo)) {
            Store(cpuCollectionInfo);
        }
    }
}

void CpuStorage::Store(const ProcessCpuStatInfo& cpuCollectionInfo)
{
    InsertTable(cpuCollectionInfo);
}

void CpuStorage::InsertTable(const ProcessCpuStatInfo& cpuCollectionInfo)
{
    if (CreateTable() != 0) {
        return;
    }
    NativeRdb::ValuesBucket bucket;
    bucket.PutLong(COLUMN_START_TIME, static_cast<int64_t>(cpuCollectionInfo.startTime));
    bucket.PutLong(COLUMN_END_TIME, static_cast<int64_t>(cpuCollectionInfo.endTime));
    bucket.PutInt(COLUMN_PID, cpuCollectionInfo.pid);
    bucket.PutInt(COLUMN_PROC_STATE, GetProcessStateInCollectionPeriod(cpuCollectionInfo));
    bucket.PutString(COLUMN_PROC_NAME, cpuCollectionInfo.procName);
    bucket.PutDouble(COLUMN_CPU_LOAD, TruncateDecimalWithNBitPrecision(cpuCollectionInfo.cpuLoad));
    bucket.PutDouble(COLUMN_CPU_USAGE, TruncateDecimalWithNBitPrecision(cpuCollectionInfo.cpuUsage));
    int64_t seq = 0;
    if (dbStore_->Insert(seq, TABLE_NAME, bucket) != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to insert cpu data to db store, pid=%{public}d, proc_name=%{public}s", 0, "");
    }
}

int32_t CpuStorage::CreateTable()
{
    /**
     * table: unified_collection_cpu
     *
     * |-----|------------|----------|-----|------------|-----------|----------|-----------|
     * |  id | start_time | end_time | pid | proc_state | proc_name | cpu_load | cpu_usage |
     * |-----|------------|----------|-----|------------|-----------|----------|-----------|
     * | INT |    INT64   |   INT64  | INT |    INT     |  VARCHAR  |  DOUBLE  |   DOUBLE  |
     * |-----|------------|----------|-----|------------|-----------|----------|-----------|
     */
    const std::vector<std::pair<std::string, std::string>> fields = {
        {COLUMN_START_TIME, SqlUtil::COLUMN_TYPE_INT},
        {COLUMN_END_TIME, SqlUtil::COLUMN_TYPE_INT},
        {COLUMN_PID, SqlUtil::COLUMN_TYPE_INT},
        {COLUMN_PROC_STATE, SqlUtil::COLUMN_TYPE_INT},
        {COLUMN_PROC_NAME, SqlUtil::COLUMN_TYPE_STR},
        {COLUMN_CPU_LOAD, SqlUtil::COLUMN_TYPE_DOU},
        {COLUMN_CPU_USAGE, SqlUtil::COLUMN_TYPE_DOU},
    };
    std::string sql = SqlUtil::GenerateCreateSql(TABLE_NAME, fields);
    if (dbStore_->ExecuteSql(sql) != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to create table, sql=%{public}s", sql.c_str());
        return -1;
    }
    return 0;
}

void CpuStorage::Report()
{
    if (!NeedReport()) {
        return;
    }
    HIVIEW_LOGI("start to report cpu collection event");
    PrepareOldDbFilesBeforeReport();
    ReportCpuCollectionEvent();
    PrepareNewDbFilesAfterReport();
}

bool CpuStorage::NeedReport()
{
    if (dbStorePath_.empty()) {
        return false;
    }
    std::string nowDbFileName = FileUtil::ExtractFileName(dbStorePath_);
    std::string newDbFileName = CreateDbFileName();
    return newDbFileName != nowDbFileName;
}

void CpuStorage::PrepareOldDbFilesBeforeReport()
{
    // 1. Close the current db file
    ResetDbStore();
    // 2. Init upload directory
    if (!InitDbStoreUploadPath()) {
        return;
    }
    // 3. Move the db file to the upload directory
    MoveDbFilesToUploadDir();
    // 4. Aging upload db files, only the latest N db files are retained
    TryToAgeUploadDbFiles();
}

void CpuStorage::ResetDbStore()
{
    dbStore_ = nullptr;
}

bool CpuStorage::InitDbStoreUploadPath()
{
    if (!dbStoreUploadPath_.empty()) {
        return true;
    }
    const std::string uploadDirName = "upload";
    std::string tmpUploadPath = FileUtil::IncludeTrailingPathDelimiter(
        FileUtil::ExtractFilePath(dbStorePath_)).append(uploadDirName);
    if (!FileUtil::IsDirectory(tmpUploadPath) && !FileUtil::ForceCreateDirectory(tmpUploadPath)) {
        HIVIEW_LOGE("failed to create upload dir=%{public}s", tmpUploadPath.c_str());
        return false;
    }
    dbStoreUploadPath_ = tmpUploadPath;
    HIVIEW_LOGI("init db upload path=%{public}s", dbStoreUploadPath_.c_str());
    return true;
}

void CpuStorage::MoveDbFilesToUploadDir()
{
    std::vector<std::string> dbFiles;
    FileUtil::GetDirFiles(FileUtil::ExtractFilePath(dbStorePath_), dbFiles, false);
    for (auto& dbFile : dbFiles) {
        // upload only xxx.db, and delete xxx.db-shm/xxx.db-wal
        if (IsDbFile(dbFile)) {
            MoveDbFileToUploadDir(dbFile);
            continue;
        }
        HIVIEW_LOGI("start to remove db file=%{public}s", dbFile.c_str());
        if (!FileUtil::RemoveFile(dbFile)) {
            HIVIEW_LOGW("failed to remove db file=%{public}s", dbFile.c_str());
        }
    }
}

void CpuStorage::MoveDbFileToUploadDir(const std::string dbFilePath)
{
    std::string uploadFilePath = FileUtil::IncludeTrailingPathDelimiter(dbStoreUploadPath_)
        .append(FileUtil::ExtractFileName(dbFilePath));
    HIVIEW_LOGI("start to move db file, src=%{public}s, dst=%{public}s", dbFilePath.c_str(), uploadFilePath.c_str());
    if (FileUtil::CopyFile(dbFilePath, uploadFilePath) != 0) {
        HIVIEW_LOGW("failed to copy db file");
        return;
    }
    if (!FileUtil::RemoveFile(dbFilePath)) {
        HIVIEW_LOGW("failed to delete db file=%{public}s", dbFilePath.c_str());
    }
}

void CpuStorage::TryToAgeUploadDbFiles()
{
    std::vector<std::string> dbFiles;
    FileUtil::GetDirFiles(dbStoreUploadPath_, dbFiles);
    if (!NeedCleanDbFiles(dbFiles)) {
        return;
    }
    HIVIEW_LOGI("start to clean db files, size=%{public}zu", dbFiles.size());
    std::sort(dbFiles.begin(), dbFiles.end());
    ClearDbFilesByTimestampOrder(dbFiles);
}

void CpuStorage::ReportCpuCollectionEvent()
{
    int32_t ret = HiSysEventWrite(HiSysEvent::Domain::HIVIEWDFX, "CPU_COLLECTION", HiSysEvent::EventType::FAULT);
    if (ret != 0) {
        HIVIEW_LOGW("failed to report cpu collection event, ret=%{public}d", ret);
    }
}

void CpuStorage::PrepareNewDbFilesAfterReport()
{
    InitDbStorePath();
    InitDbStore();
}
}  // namespace HiviewDFX
}  // namespace OHOS
