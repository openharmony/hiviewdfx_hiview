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

#include "fold_app_usage_db_helper.h"

#include "event_db_helper.h"
#include "file_util.h"
#include "hiview_logger.h"
#include "rdb_helper.h"
#include "sql_util.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("FoldAppUsage");
namespace {
const int APP_FOREGROUND_EVENT = 1101;
const int SCREEN_STAT_CHANGE_EVENT = 1103;
const int USAGE_STATISTICS_EVENT = 1104;
const int SCREEN_EXPAND_HORIZONAL_STAT = 11;
const int SCREEN_EXPAND_VERTICAL_STAT = 12;
const int SCREEN_FOLD_HORIZONAL_STAT = 21;
const int SCREEN_FOLD_VERTICAL_STAT = 22;
const std::string DB_TABLE_NAME = "app_events";
const std::string EVENT_RAWID_COL_NAME = "rawid";
const std::string SEQ_ID_COL_NAME = "_ID";
const std::string SCREEN_STATUS_COL_NAME = "long1";
const std::string FORMER_SCREEN_STATUS_COL_NAME = "long2";
const std::string APP_VERION_COL_NAME = "long3";
const std::string HAPPEN_TIME_COL_NAME = "long4";
const std::string FOLD_VER_COL_NAME = "long5";
const std::string FOLD_HOR_COL_NAME = "long6";
const std::string EXPD_VER_COL_NAME = "long7";
const std::string EXPD_HOR_COL_NAME = "long8";
const std::string APP_NAME_COL_NAME = "str1";
const std::string BOOT_TIME_COL_NAME = "ts";

void UpdateScreenStatInfo(FoldAppUsageInfo &info, uint32_t time, int screenStatus)
{
    switch (screenStatus) {
        case SCREEN_EXPAND_HORIZONAL_STAT:
            info.expdHor += time;
            break;
        case SCREEN_EXPAND_VERTICAL_STAT:
            info.expdVer += time;
            break;
        case SCREEN_FOLD_HORIZONAL_STAT:
            info.foldHor += time;
            break;
        case SCREEN_FOLD_VERTICAL_STAT:
            info.foldVer += time;
            break;
        default:
            return;
    }
}

bool GetStringFromResultSet(std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet,
    const std::string& colName, std::string &value)
{
    int colIndex = 0;
    if (resultSet->GetColumnIndex(colName, colIndex) != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to get column index, column = %{public}s", colName.c_str());
        return false;
    }
    if (resultSet->GetString(colIndex, value) != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to get string value, column = %{public}s", colName.c_str());
        return false;
    }
    return true;
}

bool GetIntFromResultSet(std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet,
    const std::string& colName, int &value)
{
    int colIndex = 0;
    if (resultSet->GetColumnIndex(colName, colIndex) != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to get column index, column = %{public}s", colName.c_str());
        return false;
    }
    if (resultSet->GetInt(colIndex, value) != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to get int value, column = %{public}s", colName.c_str());
        return false;
    }
    return true;
}

bool GetLongFromResultSet(std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet,
    const std::string& colName, int64_t &value)
{
    int colIndex = 0;
    if (resultSet->GetColumnIndex(colName, colIndex) != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to get column index, column = %{public}s", colName.c_str());
        return false;
    }
    if (resultSet->GetLong(colIndex, value) != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to get long value, column = %{public}s", colName.c_str());
        return false;
    }
    return true;
}
}

FoldAppUsageDbHelper::FoldAppUsageDbHelper(std::string dbPath)
{
    if (!FileUtil::FileExists(dbPath) && !FileUtil::ForceCreateDirectory(dbPath)) {
        HIVIEW_LOGE("failed to create db dir");
        return;
    }
    NativeRdb::RdbStoreConfig config(dbPath + "log.db");
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    EventDbStoreCallback callback;
    int ret = NativeRdb::E_OK;
    rdbStore_ = NativeRdb::RdbHelper::GetRdbStore(config, 1, callback, ret);
    if (ret != NativeRdb::E_OK) {
        HIVIEW_LOGE("get rdb store failed");
    }
}

FoldAppUsageDbHelper::~FoldAppUsageDbHelper()
{}

int FoldAppUsageDbHelper::QueryFinalScreenStatus(uint64_t endTime)
{
    if (rdbStore_ == nullptr) {
        HIVIEW_LOGE("db is nullptr");
        return 0;
    }
    NativeRdb::AbsRdbPredicates predicates(DB_TABLE_NAME);
    predicates.Between(HAPPEN_TIME_COL_NAME, 0, static_cast<int64_t>(endTime));
    predicates.OrderByDesc(SEQ_ID_COL_NAME);
    predicates.Limit(1);
    auto resultSet = rdbStore_->Query(predicates, {SEQ_ID_COL_NAME, SCREEN_STATUS_COL_NAME});
    if (resultSet == nullptr) {
        HIVIEW_LOGE("resultSet is nullptr");
        return 0;
    }
    int id = 0;
    int status = 0;
    if (resultSet->GoToNextRow() == NativeRdb::E_OK &&
        GetIntFromResultSet(resultSet, SEQ_ID_COL_NAME, id) &&
        GetIntFromResultSet(resultSet, SCREEN_STATUS_COL_NAME, status) == NativeRdb::E_OK) {
        HIVIEW_LOGI("get handle seq: %{public}d, screen stat: %{public}d", id, status);
    } else {
        HIVIEW_LOGE("get handle seq and screen stat failed");
    }
    return status;
}

void FoldAppUsageDbHelper::QueryStatisticEventsInPeriod(uint64_t startTime, uint64_t endTime,
    std::vector<FoldAppUsageInfo> &infos)
{
    if (rdbStore_ == nullptr) {
        HIVIEW_LOGE("db is nullptr");
        return;
    }
    NativeRdb::AbsRdbPredicates predicates(DB_TABLE_NAME);
    predicates.EqualTo(EVENT_RAWID_COL_NAME, USAGE_STATISTICS_EVENT);
    predicates.Between(HAPPEN_TIME_COL_NAME, static_cast<int64_t>(startTime), static_cast<int64_t>(endTime));
    predicates.OrderByAsc(SEQ_ID_COL_NAME);
    auto resultSet = rdbStore_->Query(predicates, {APP_NAME_COL_NAME, APP_VERION_COL_NAME, FOLD_VER_COL_NAME,
        FOLD_HOR_COL_NAME, EXPD_VER_COL_NAME, EXPD_HOR_COL_NAME});
    if (resultSet == nullptr) {
        HIVIEW_LOGE("resultSet is nullptr");
        return;
    }
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        FoldAppUsageInfo usageInfo("", 0, 0, 0, 0, 0);
        if (GetStringFromResultSet(resultSet, APP_NAME_COL_NAME, usageInfo.package) &&
            GetLongFromResultSet(resultSet, APP_VERION_COL_NAME, usageInfo.version) &&
            GetIntFromResultSet(resultSet, FOLD_VER_COL_NAME, usageInfo.foldVer) &&
            GetIntFromResultSet(resultSet, FOLD_HOR_COL_NAME, usageInfo.foldHor) &&
            GetIntFromResultSet(resultSet, EXPD_VER_COL_NAME, usageInfo.expdVer) &&
            GetIntFromResultSet(resultSet, EXPD_HOR_COL_NAME, usageInfo.expdHor)) {
            infos.emplace_back(usageInfo);
        } else {
            HIVIEW_LOGE("fail to get appusage info!");
        }
    }
}

void FoldAppUsageDbHelper::QueryForegroundAppsInfo(uint64_t startTime, uint64_t endTime, int screenStatus,
    FoldAppUsageInfo &info)
{
    if (rdbStore_ == nullptr) {
        HIVIEW_LOGE("db is nullptr");
        return;
    }
    NativeRdb::AbsRdbPredicates predicates(DB_TABLE_NAME);
    predicates.EqualTo(APP_NAME_COL_NAME, info.package);
    predicates.Between(HAPPEN_TIME_COL_NAME, static_cast<int64_t>(startTime), static_cast<int64_t>(endTime));
    predicates.OrderByDesc(SEQ_ID_COL_NAME);
    auto resultSet = rdbStore_->Query(predicates, {SEQ_ID_COL_NAME, EVENT_RAWID_COL_NAME, APP_NAME_COL_NAME,
        APP_VERION_COL_NAME, HAPPEN_TIME_COL_NAME, SCREEN_STATUS_COL_NAME, FORMER_SCREEN_STATUS_COL_NAME,
        BOOT_TIME_COL_NAME});
    if (resultSet == nullptr) {
        HIVIEW_LOGE("resultSet is nullptr");
        return;
    }
    std::vector<FoldAppUsageRawEvent> events;
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        FoldAppUsageRawEvent event;
        if (!GetLongFromResultSet(resultSet, SEQ_ID_COL_NAME, event.id) ||
            !GetIntFromResultSet(resultSet, EVENT_RAWID_COL_NAME, event.rawId) ||
            !GetStringFromResultSet(resultSet, APP_NAME_COL_NAME, event.package) ||
            !GetLongFromResultSet(resultSet, APP_VERION_COL_NAME, event.version) ||
            !GetLongFromResultSet(resultSet, HAPPEN_TIME_COL_NAME, event.happenTime) ||
            !GetIntFromResultSet(resultSet, SCREEN_STATUS_COL_NAME, event.screenStatusAfter) ||
            !GetIntFromResultSet(resultSet, FORMER_SCREEN_STATUS_COL_NAME, event.screenStatusBefore) ||
            !GetLongFromResultSet(resultSet, BOOT_TIME_COL_NAME, event.ts)) {
            HIVIEW_LOGE("fail to get db event!");
            return;
        }
        if (event.rawId != APP_FOREGROUND_EVENT && event.rawId != SCREEN_STAT_CHANGE_EVENT) {
            HIVIEW_LOGE("can not find foregroud event, latest raw id: %{public}d", event.rawId);
            return;
        }
        events.emplace_back(event);
        if (event.rawId == APP_FOREGROUND_EVENT) {
            break;
        }
    }
    info = CaculateForegroundAppUsage(events, startTime, endTime, info.package, screenStatus);
}

int FoldAppUsageDbHelper::DeleteEventsByTime(uint64_t clearDataTime)
{
    if (rdbStore_ == nullptr) {
        HIVIEW_LOGE("db is nullptr");
        return 0;
    }
    NativeRdb::AbsRdbPredicates predicates(DB_TABLE_NAME);
    predicates.Between(HAPPEN_TIME_COL_NAME, 0, static_cast<int64_t>(clearDataTime));
    int seq = 0;
    int ret = rdbStore_->Delete(seq, predicates);
    HIVIEW_LOGI("rows are deleted: %{public}d, ret: %{public}d", seq, ret);
    return seq;
}

FoldAppUsageInfo FoldAppUsageDbHelper::CaculateForegroundAppUsage(const std::vector<FoldAppUsageRawEvent> &events,
    uint64_t startTime, uint64_t endTime, const std::string &appName, int screenStatus)
{
    FoldAppUsageInfo info = FoldAppUsageInfo(appName, 0, 0, 0, 0, 0);
    // no event means: app is foregroud for whole day.
    if (events.size() == 0) {
        UpdateScreenStatInfo(info, static_cast<uint32_t>(endTime - startTime), screenStatus);
        return info;
    }
    uint32_t size = events.size();
    // first event is screen changed, means app is started befor statistic period.
    if (events[size - 1].rawId == SCREEN_STAT_CHANGE_EVENT) {
        UpdateScreenStatInfo(info, static_cast<uint32_t>(events[size - 1].happenTime - startTime),
            events[size - 1].screenStatusBefore);
    }
    // caculate all period between screen status changed events, till endTime.
    for (uint32_t i = size - 1; i > 0; --i) {
        if (events[i - 1].ts > events[i].ts) {
            UpdateScreenStatInfo(info, static_cast<uint32_t>(events[i - 1].ts - events[i].ts),
                events[i].screenStatusAfter);
        }
    }
    UpdateScreenStatInfo(info, static_cast<uint32_t>(endTime - events[0].happenTime), events[0].screenStatusAfter);
    return info;
}
} // namespace HiviewDFX
} // namespace OHOS