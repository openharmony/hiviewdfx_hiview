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
#include <algorithm>
#include <cmath>

#include "file_util.h"
#include "hisysevent.h"
#include "logger.h"
#include "process_status.h"
#include "rdb_helper.h"
#include "sql_util.h"
#include "string_util.h"
#include "trace_storage.h"

using namespace OHOS::HiviewDFX::UCollectUtil;

namespace OHOS {
namespace HiviewDFX {
namespace {
DEFINE_LOG_TAG("UCollectUtil-TraceCollector");
constexpr int32_t DB_VERSION = 1;
const std::string TABLE_NAME = "trace_flow_control";
const std::string COLUMN_SYSTEM_TIME = "system_time";
const std::string COLUMN_XPERF_SIZE = "xperf_size";
const std::string COLUMN_XPOWER_SIZE = "xpower_size";
const std::string COLUMN_RELIABILITY_SIZE = "reliability_size";
const std::string DB_PATH = "/data/log/hiview/unified_collection/trace/";
}

class TraceDbStoreCallback : public NativeRdb::RdbOpenCallback {
public:
    int OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
};

int TraceDbStoreCallback::OnCreate(NativeRdb::RdbStore& rdbStore)
{
    HIVIEW_LOGD("create dbStore");
    return NativeRdb::E_OK;
}

int TraceDbStoreCallback::OnUpgrade(NativeRdb::RdbStore& rdbStore, int oldVersion, int newVersion)
{
    HIVIEW_LOGD("oldVersion=%{public}d, newVersion=%{public}d", oldVersion, newVersion);
    return NativeRdb::E_OK;
}

TraceStorage::TraceStorage()
{
    InitDbStore();
}

void TraceStorage::InitDbStore()
{
    dbStorePath_ = DB_PATH;
    dbStorePath_.append(TABLE_NAME).append(".db");   // trace_flow_control.db
    NativeRdb::RdbStoreConfig config(dbStorePath_);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    TraceDbStoreCallback callback;
    auto ret = NativeRdb::E_OK;
    dbStore_ = NativeRdb::RdbHelper::GetRdbStore(config, DB_VERSION, callback, ret);
    if (ret != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to init db store, db store path=%{public}s", dbStorePath_.c_str());
        dbStore_ = nullptr;
        return;
    }

    std::string sql = SqlUtil::GenerateExistSql(TABLE_NAME);
    auto retSql = dbStore_->ExecuteSql(sql);
    HIVIEW_LOGI("InitDbStore, retSql= %{public}d, E_OK= %{public}d.", retSql, NativeRdb::E_OK);
    if (retSql < NativeRdb::E_OK) {
        HIVIEW_LOGE("table %{public}s not exists", TABLE_NAME.c_str());
        struct UcollectionTraceStorage traceStorage;
        traceStorage.systemTime = "";
        traceStorage.xperfSize = 0;
        traceStorage.xpowerSize = 0;
        traceStorage.reliabilitySize = 0;
        InsertTable(traceStorage);
        return;
    }
}

void TraceStorage::Store(const UcollectionTraceStorage& traceStorage)
{
    if (dbStore_ == nullptr) {
        HIVIEW_LOGE("db store is null, path=%{public}s", dbStorePath_.c_str());
        return;
    }
    InsertTable(traceStorage);
}

void TraceStorage::InsertTable(const UcollectionTraceStorage& traceStorage)
{
    if (CreateTable() != 0) {
        return;
    }

    int64_t id;
    NativeRdb::ValuesBucket bucket;
    bucket.PutInt("id", 1);
    bucket.PutString(COLUMN_SYSTEM_TIME, traceStorage.systemTime);
    bucket.PutLong(COLUMN_XPERF_SIZE, traceStorage.xperfSize);
    bucket.PutLong(COLUMN_XPOWER_SIZE, traceStorage.xpowerSize);
    bucket.PutLong(COLUMN_RELIABILITY_SIZE, traceStorage.reliabilitySize);
    int ret = dbStore_->InsertWithConflictResolution(id, TABLE_NAME, bucket,
        NativeRdb::ConflictResolution::ON_CONFLICT_REPLACE);
    if (ret != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to insert trace data to db store, ret=%{public}d", ret);
    }
}

void TraceStorage::Query(UcollectionTraceStorage &traceStorage)
{
    if (dbStore_ == nullptr) {
        HIVIEW_LOGE("db store is null, path=%{public}s", dbStorePath_.c_str());
        return;
    }
    GetResultItems(traceStorage);
}

void TraceStorage::GetResultItems(UcollectionTraceStorage &traceStorage)
{
    std::string sql;
    sql.append("SELECT ")
        .append(COLUMN_SYSTEM_TIME).append(", ")
        .append(COLUMN_XPERF_SIZE).append(", ")
        .append(COLUMN_XPOWER_SIZE).append(", ")
        .append(COLUMN_RELIABILITY_SIZE)
        .append(" FROM ").append(TABLE_NAME);
    std::shared_ptr<NativeRdb::ResultSet> resultSet = dbStore_->QuerySql(sql, std::vector<std::string> {});
    if (resultSet == nullptr) {
        HIVIEW_LOGE("failed to query from table %{public}s, db is null", TABLE_NAME.c_str());
        return;
    }

    if (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        HIVIEW_LOGI("start to GoToNextRow.");
        resultSet->GetString(0, traceStorage.systemTime);    // 0 means system_time field
        resultSet->GetLong(1, traceStorage.xperfSize);       // 1 means xperf_size field
        resultSet->GetLong(2, traceStorage.xpowerSize);      // 2 means xpower_size field
        resultSet->GetLong(3, traceStorage.reliabilitySize); // 3 means reliability_size field
        HIVIEW_LOGI("systemTime:%{public}s, xperfSize:%{public}d, xpowerSize:%{public}d, reliabilitySize:%{public}d",
            traceStorage.systemTime.c_str(), traceStorage.xperfSize,
            traceStorage.xpowerSize, traceStorage.reliabilitySize);
    }
}

int32_t TraceStorage::CreateTable()
{
    /**
     * table: unified_collection_cpu
     *
     * describe: store data that has been spent
     * |-----|-------------|------------|-------------|------------------|
     * |  id | system_time | xperf_size | xpower_size | reliability_size |
     * |-----|-------------|------------|-------------|------------------|
     * | INT |     INT64   |    INT64   |    INT64    |       INT64      |
     * |-----|-------------|------------|-------------|------------------|
     */
    const std::vector<std::pair<std::string, std::string>> fields = {
        {COLUMN_SYSTEM_TIME, SqlUtil::COLUMN_TYPE_STR},
        {COLUMN_XPERF_SIZE, SqlUtil::COLUMN_TYPE_INT},
        {COLUMN_XPOWER_SIZE, SqlUtil::COLUMN_TYPE_INT},
        {COLUMN_RELIABILITY_SIZE, SqlUtil::COLUMN_TYPE_INT},
    };
    std::string sql = SqlUtil::GenerateCreateSql(TABLE_NAME, fields);
    if (dbStore_->ExecuteSql(sql) != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to create table, sql=%{public}s", sql.c_str());
        return -1;
    }
    return 0;
}
}  // namespace HiviewDFX
}  // namespace OHOS
