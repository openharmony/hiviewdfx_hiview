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

#include "data_share_dao.h"

#include <map>
#include <string>
#include <vector>

#include "hiview_logger.h"
#include "rdb_errno.h"
#include "rdb_store.h"
#include "value_object.h"
#include "values_bucket.h"

#include "data_share_common.h"
#include "data_share_util.h"
#include "data_share_store.h"
#include "string_util.h"
#include "time_util.h"

using namespace OHOS::HiviewDFX::SubscribeStore;
namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-DataShareDao");
DataShareDao::DataShareDao(std::shared_ptr<DataShareStore> store) : store_(store)
{
    eventTable_ = EventTable::TABLE;
}

bool DataShareDao::IsUidExists(int32_t uid)
{
    auto dbStore = store_->GetDbStore();
    if (dbStore == nullptr) {
        HIVIEW_LOGE("DataShareDao::IsUidExists, dbStore is null.");
        return false;
    }
    std::string sql;
    sql.append("SELECT COUNT(*) FROM ").append(eventTable_)
        .append(" WHERE ").append(EventTable::FIELD_UID).append(" = ?");
    int64_t count = 0;
    std::vector<NativeRdb::ValueObject> objects = { NativeRdb::ValueObject(uid) };
    if (int ret = dbStore->ExecuteAndGetLong(count, sql, objects); ret != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to query uid info: %{public}d, ret=%{public}d", uid, ret);
        return false;
    }
    return count != 0;
}

int DataShareDao::SaveSubscriberInfo(int32_t uid, const std::string& events)
{
    auto dbStore = store_->GetDbStore();
    if (dbStore == nullptr) {
        HIVIEW_LOGE("DataShareDao::SaveSubscriberInfo, dbStore is null.");
        return DB_FAILED;
    }
    std::string bundleName = OHOS::HiviewDFX::DataShareUtil::GetBundleNameById(uid);
    if (this->IsUidExists(uid)) {
        int rows = 0;
        NativeRdb::ValuesBucket values;
        values.PutInt(EventTable::FIELD_UID, uid);
        values.PutString(EventTable::FIELD_BUNDLE_NAME, bundleName);
        values.PutLong(EventTable::FIELD_SUBSCRIBETIME, TimeUtil::GetMilliseconds());
        values.PutString(EventTable::FIELD_EVENTLIST, events);
        std::string whereClause = "uid = ?";
        std::vector<std::string> whereArgs = {std::to_string(uid)};
        if (int ret = dbStore->Update(rows, eventTable_, values, whereClause, whereArgs); ret != NativeRdb::E_OK) {
            HIVIEW_LOGE("failed to update uid %{public}d, ret=%{public}d", uid, ret);
            return DB_FAILED;
        }
        return DB_SUCC;
    }
    NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutInt(EventTable::FIELD_UID, uid);
    valuesBucket.PutString(EventTable::FIELD_BUNDLE_NAME, bundleName);
    valuesBucket.PutLong(EventTable::FIELD_SUBSCRIBETIME, TimeUtil::GetMilliseconds());
    valuesBucket.PutString(EventTable::FIELD_EVENTLIST, events);
    int64_t seq = 0;
    if (int ret = dbStore->Insert(seq, eventTable_, valuesBucket); ret != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to add uid %{public}d, ret=%{public}d", uid, ret);
        return DB_FAILED;
    }
    return DB_SUCC;
}

int DataShareDao::DeleteSubscriberInfo(int32_t uid)
{
    auto dbStore = store_->GetDbStore();
    if (dbStore == nullptr) {
        HIVIEW_LOGE("failed to delete subscriberInfo from table.");
        return DB_FAILED;
    }
    std::string cond;
    cond += EventTable::FIELD_UID;
    cond += " = ?";
    int delRow = 0;
    std::vector<std::string> fields = { std::to_string(uid) };
    if (int ret = dbStore->Delete(delRow, eventTable_, cond, fields); ret != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to delete subscriberInfo from table.");
        return DB_FAILED;
    }
    return DB_SUCC;
}

int DataShareDao::GetEventListByUid(int32_t uid, std::string& events)
{
    auto dbStore = store_->GetDbStore();
    if (dbStore == nullptr) {
        HIVIEW_LOGE("failed to query from table %{public}s, db is null", eventTable_.c_str());
        return DB_FAILED;
    }
    std::string sql;
    sql.append("SELECT ")
        .append(EventTable::FIELD_EVENTLIST)
        .append(" FROM ").append(eventTable_)
        .append(" WHERE uid")
        .append(" = ?");
    auto resultSet = dbStore->QuerySql(sql, std::vector<std::string> {std::to_string(uid)});
    if (resultSet == nullptr) {
        HIVIEW_LOGE("failed to get eventList");
        return DB_FAILED;
    }
    if (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        resultSet->GetString(0, events); // 0 means eventList
    }
    resultSet->Close();
    return DB_SUCC;
}

int DataShareDao::GetUidByBundleName(const std::string& bundleName, int32_t& uid)
{
    auto dbStore = store_->GetDbStore();
    if (dbStore == nullptr) {
        HIVIEW_LOGE("failed to query from table %{public}s, db is null", eventTable_.c_str());
        return DB_FAILED;
    }
    std::string sql;
    sql.append("SELECT ")
        .append(EventTable::FIELD_UID)
        .append(" FROM ").append(eventTable_)
        .append(" WHERE bundle_name")
        .append(" = ?");
    auto resultSet = dbStore->QuerySql(sql, std::vector<std::string> {bundleName});
    if (resultSet == nullptr) {
        HIVIEW_LOGE("failed to get eventList");
        return DB_FAILED;
    }
    if (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        resultSet->GetInt(0, uid); // 0 means eventList
    }
    resultSet->Close();
    return DB_SUCC;
}

int DataShareDao::GetTotalSubscriberInfo(std::map<int, std::string>& map)
{
    auto dbStore = store_->GetDbStore();
    if (dbStore == nullptr) {
        HIVIEW_LOGE("failed to query from table %{public}s, db is null", eventTable_.c_str());
        return DB_FAILED;
    }
    std::string sql;
    sql.append("SELECT ")
        .append(EventTable::FIELD_UID)
        .append(", ")
        .append(EventTable::FIELD_EVENTLIST)
        .append(" FROM ").append(eventTable_);
    auto resultSet = dbStore->QuerySql(sql, std::vector<std::string> {});
    if (resultSet == nullptr) {
        HIVIEW_LOGE("failed to get eventList");
        return DB_FAILED;
    }
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        int uid = 0;
        resultSet->GetInt(0, uid); // 0 means uid field
        std::string events;
        resultSet->GetString(1, events); // 1 means events field
        map.insert(std::make_pair(uid, events));
    }
    resultSet->Close();
    return DB_SUCC;
}

} // namespace HiviewDFX
} // namespace OHOS
