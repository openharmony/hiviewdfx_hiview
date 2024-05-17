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

#include "sys_event_repeat_db.h"

#include "hiview_logger.h"
#include "rdb_helper.h"
#include "sql_util.h"
#include "sys_event_database.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-SysEvent-Repeat-Db");
namespace {
    const std::string DB_FILE_NAME = "sys_event_history.db";
    const std::string TABLE_NAME = "sys_event_history";
    constexpr int32_t DB_VERSION = 1;

    const std::string COLUMN_DOMAIN = "domain";
    const std::string COLUMN_NAME = "name";
    const std::string COLUMN_EVENT_HASH = "eventHash";
    const std::string COLUMN_HAPPENTIME = "happentime";
    constexpr int32_t MAX_DB_COUNT = 10000;

class SysEventRepeatDbCallback : public NativeRdb::RdbOpenCallback {
public:
    int OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
};

int SysEventRepeatDbCallback::OnCreate(NativeRdb::RdbStore& rdbStore)
{
    HIVIEW_LOGI("create dbStore");
    return NativeRdb::E_OK;
}

int SysEventRepeatDbCallback::OnUpgrade(NativeRdb::RdbStore& rdbStore, int oldVersion, int newVersion)
{
    HIVIEW_LOGI("oldVersion=%{public}d, newVersion=%{public}d", oldVersion, newVersion);
    return NativeRdb::E_OK;
}
}

SysEventRepeatDb::SysEventRepeatDb()
{
    InitDbStore();
    RefreshDbCount();
}

void SysEventRepeatDb::InitDbStore()
{
    auto dbFullName = EventStore::SysEventDatabase::GetInstance().GetDatabaseDir() + DB_FILE_NAME;
    NativeRdb::RdbStoreConfig config(dbFullName);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    SysEventRepeatDbCallback callback;
    auto ret = NativeRdb::E_OK;
    dbStore_ = NativeRdb::RdbHelper::GetRdbStore(config, DB_VERSION, callback, ret);
    if (ret != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to init db store, db store path=%{public}s", dbFullName.c_str());
        dbStore_ = nullptr;
        return;
    }
    CreateTable();
}

void SysEventRepeatDb::CreateTable()
{
    if (dbStore_ == nullptr) {
        return;
    }
    /**
     * table: sys_event_history
     *
     * describe: store data that app task
     * |---------|------|-----------|------------|
     * |  domain | name | eventHash | happentime |
     * |---------|------|-----------|------------|
     * |  TEXT   | TEXT |   TEXT    |    INT64   |
     * |---------|------|-----------|------------|
     */
    const std::vector<std::pair<std::string, std::string>> fields = {
        {COLUMN_DOMAIN, SqlUtil::COLUMN_TYPE_STR},
        {COLUMN_NAME, SqlUtil::COLUMN_TYPE_STR},
        {COLUMN_EVENT_HASH, SqlUtil::COLUMN_TYPE_STR},
        {COLUMN_HAPPENTIME, SqlUtil::COLUMN_TYPE_INT},
    };
    std::string sql = SqlUtil::GenerateCreateSql(TABLE_NAME, fields);
    if (dbStore_->ExecuteSql(sql) != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to create table, sql=%{public}s", sql.c_str());
        return;
    }

    std::string indexSql = "create index sys_event_his_index1 on ";
    indexSql.append(TABLE_NAME).append("(").append(COLUMN_DOMAIN).append(", ")
        .append(COLUMN_NAME).append(",").append(COLUMN_EVENT_HASH).append(");");
    if (dbStore_->ExecuteSql(indexSql) != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to create index1, sql=%{public}s", indexSql.c_str());
        return;
    }

    std::string indexHappentimeSql = "create index sys_event_his_index2 on ";
    indexHappentimeSql.append(TABLE_NAME).append("(").append(COLUMN_HAPPENTIME).append(");");
    if (dbStore_->ExecuteSql(indexHappentimeSql) != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to create index2, sql=%{public}s", indexHappentimeSql.c_str());
        return;
    }
}

void SysEventRepeatDb::CheckAndClearDb(const int64_t happentime)
{
    if (dbCount_ <= MAX_DB_COUNT) {
        return;
    }
    ClearHistory(happentime);
    RefreshDbCount();
}

bool SysEventRepeatDb::Insert(const SysEventHashRecord &sysEventHashRecord)
{
    if (dbStore_ == nullptr) {
        return false;
    }
    NativeRdb::ValuesBucket bucket;
    bucket.PutString(COLUMN_DOMAIN, sysEventHashRecord.domain);
    bucket.PutString(COLUMN_NAME, sysEventHashRecord.name);
    bucket.PutString(COLUMN_EVENT_HASH, sysEventHashRecord.eventHash);
    bucket.PutLong(COLUMN_HAPPENTIME, sysEventHashRecord.happentime);
    int64_t seq = 0;
    int ret = dbStore_->Insert(seq, TABLE_NAME, bucket);
    if (ret != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to insert app event task, ret=%{public}d", ret);
        return false;
    }
    ++dbCount_;
    return true;
}

bool SysEventRepeatDb::Update(const SysEventHashRecord &sysEventHashRecord)
{
    if (dbStore_ == nullptr) {
        return false;
    }
    NativeRdb::AbsRdbPredicates predicates(TABLE_NAME);
    predicates.EqualTo(COLUMN_DOMAIN, sysEventHashRecord.domain);
    predicates.EqualTo(COLUMN_NAME, sysEventHashRecord.name);
    predicates.EqualTo(COLUMN_EVENT_HASH, sysEventHashRecord.eventHash);
    NativeRdb::ValuesBucket bucket;
    bucket.PutLong(COLUMN_HAPPENTIME, sysEventHashRecord.happentime);
    int updateRowNum = 0;
    if (dbStore_->Update(updateRowNum, bucket, predicates) != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to update table.");
        return false;
    }
    return true;
}

void SysEventRepeatDb::ClearHistory(const int64_t happentime)
{
    if (dbStore_ == nullptr) {
        return;
    }
    std::string whereClause = COLUMN_HAPPENTIME;
        whereClause.append(" < ").append(std::to_string(happentime));
    int deleteRows = 0;
    if (dbStore_->Delete(deleteRows, TABLE_NAME, whereClause) != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to delete, whereClause=%{public}s", whereClause.c_str());
        return;
    }
}

void SysEventRepeatDb::RefreshDbCount()
{
    if (dbStore_ == nullptr) {
        return;
    }
    std::string sql;
    sql.append("SELECT count(*) from ").append(TABLE_NAME).append(";");
    std::shared_ptr<NativeRdb::ResultSet> resultSet = dbStore_->QuerySql(sql, std::vector<std::string> {});
    if (resultSet == nullptr) {
        HIVIEW_LOGE("failed to query from table %{public}s, db is null", TABLE_NAME.c_str());
        return;
    }
    if (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        resultSet->GetLong(0, dbCount_);
        resultSet->Close();
        return;
    }
    resultSet->Close();
}

int64_t SysEventRepeatDb::QueryHappentime(SysEventHashRecord &sysEventHashRecord)
{
    if (dbStore_ == nullptr) {
        return 0;
    }
    NativeRdb::AbsRdbPredicates predicates(TABLE_NAME);
    predicates.EqualTo(COLUMN_DOMAIN, sysEventHashRecord.domain);
    predicates.EqualTo(COLUMN_NAME, sysEventHashRecord.name);
    predicates.EqualTo(COLUMN_EVENT_HASH, sysEventHashRecord.eventHash);
    auto resultSet = dbStore_->Query(predicates, {COLUMN_HAPPENTIME});
    if (resultSet == nullptr) {
        HIVIEW_LOGE("failed to query from table %{public}s, db is null", TABLE_NAME.c_str());
        return 0;
    }

    if (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        int64_t happentime = 0;
        resultSet->GetLong(0, happentime);   // 0 is result of happentime
        resultSet->Close();
        return happentime;
    }
    resultSet->Close();
    return 0;
}

} // HiviewDFX
} // OHOS