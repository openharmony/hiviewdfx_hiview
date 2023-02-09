/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "sys_event_query_wrapper.h"

#include "data_query.h"
#include "running_status_logger.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr char SPACE_CONCAT[] = " ";
constexpr int DEFAULT_CONCURRENT_CNT = 0;
time_t GetCurTime()
{
    time_t current;
    (void)time(&current);
    return current;
}

int GetSubStrCount(const std::string& content, const std::string& sub)
{
    int cnt = 0;
    if (content.empty() || sub.empty()) {
        return cnt;
    }
    size_t start = 0;
    while ((start = content.find(sub, start)) != std::string::npos) {
        start += sub.size();
        cnt++;
    }
    return cnt;
}
}

namespace EventStore {
void QueryStatusLogUtil::LogTooManyQueryRules(const std::string sql)
{
    std::string info { "PLUGIN TOOMANYQUERYCONDITION SQL=" };
    info.append(sql);
    Logging(info);
}

void QueryStatusLogUtil::LogTooManyConcurrentQueries(const int limit, bool innerQuery)
{
    std::string info { innerQuery ? "HIVIEW TOOMANYCOCURRENTQUERIES " : "TOOMANYCOCURRENTQUERIES " };
    info.append("COUNT > ");
    info.append(std::to_string(limit));
    Logging(info);
}

void QueryStatusLogUtil::LogQueryOverTime(time_t costTime, const std::string sql, bool innerQuery)
{
    std::string info { innerQuery ? "PLUGIN QUERYOVERTIME " : "QUERYOVERTIME " };
    info.append(std::to_string(costTime)).append(SPACE_CONCAT).append("SQL=").append(sql);
    Logging(info);
}

void QueryStatusLogUtil::LogQueryCountOverLimit(const int32_t queryCount, const std::string& sql,
    bool innerQuery)
{
    std::string info { innerQuery ? "PLUGIN QUERYCOUNTOVERLIMIT " : "QUERYCOUNTOVERLIMIT " };
    info.append(std::to_string(queryCount)).append(SPACE_CONCAT).append("SQL=").append(sql);
    Logging(info);
}

void QueryStatusLogUtil::LogQueryTooFrequently(const std::string& sql, const std::string& processName,
    bool innerQuery)
{
    std::string info { innerQuery ? "HIVIEW QUERYTOOFREQUENTLY " : "QUERYTOOFREQUENTLY " };
    if (!processName.empty()) {
        info.append(processName).append(SPACE_CONCAT);
    }
    info.append("SQL=").append(sql);
    Logging(info);
}

void QueryStatusLogUtil::Logging(const std::string& detail)
{
    std::string info = RunningStatusLogger::GetInstance().FormatTimeStamp();
    info.append(SPACE_CONCAT).append(detail);
    RunningStatusLogger::GetInstance().Log(info);
}

ConcurrentQueries SysEventQueryWrapper::concurrentQueries_;
LastQueries SysEventQueryWrapper::lastQueries_;
std::mutex SysEventQueryWrapper::lastQueriesMutex_;
std::mutex SysEventQueryWrapper::concurrentQueriesMutex_;
ResultSet SysEventQueryWrapper::Execute(int limit, DbQueryTag tag, QueryProcessInfo callerInfo,
    DbQueryCallback queryCallback)
{
    DataQuery dataQuery;
    SysEventQuery::BuildDataQuery(dataQuery, limit);
    ResultSet resultSet;
    int queryErrorCode = -1;
    (void)IsConditionCntValid(dataQuery, tag);
    if (!IsQueryCntLimitValid(dataQuery, tag, limit, queryCallback) ||
        !IsConcurrentQueryCntValid(GetDbFile(), tag, queryCallback) ||
        !IsQueryFrequenceValid(dataQuery, tag, GetDbFile(), callerInfo, queryCallback)) {
        resultSet.Set(queryErrorCode, false);
        return resultSet;
    }
    time_t beforeExecute = GetCurTime();
    IncreaseConcurrentCnt(GetDbFile(), tag);
    resultSet = SysEventQuery::Execute(limit, tag, callerInfo, queryCallback);
    DecreaseConcurrentCnt(GetDbFile(), tag);
    time_t afterExecute = GetCurTime();
    if (!IsQueryCostTimeValid(dataQuery, tag, beforeExecute, afterExecute, queryCallback)) {
        resultSet.Set(queryErrorCode, false);
        return resultSet;
    }
    if (queryCallback != nullptr) {
        queryCallback(DbQueryStatus::SUCCEED);
    }
    return resultSet;
}

bool SysEventQueryWrapper::IsConditionCntValid(const DataQuery& query, const DbQueryTag& tag)
{
    int conditionCntLimit = 8;
    if (tag.isInnerQuery && GetSubStrCount(query.ToString(), "domain_=") > conditionCntLimit) {
        QueryStatusLogUtil::LogTooManyQueryRules(query.ToString());
        return false;
    }
    return true;
}

bool SysEventQueryWrapper::IsQueryCntLimitValid(const DataQuery& query, const DbQueryTag& tag, const int limit,
    const DbQueryCallback& callback)
{
    int queryLimit = tag.isInnerQuery ? 50 : 1000;
    if (limit > queryLimit) {
        QueryStatusLogUtil::LogQueryCountOverLimit(limit, query.ToString(), tag.isInnerQuery);
        if (callback != nullptr) {
            callback(DbQueryStatus::OVER_LIMIT);
        }
        return tag.isInnerQuery;
    }
    return true;
}

bool SysEventQueryWrapper::IsQueryCostTimeValid(const DataQuery& query, const DbQueryTag& tag, const time_t before,
    const time_t after, const DbQueryCallback& callback)
{
    time_t maxQueryTime = 20;
    time_t duration = after - before;
    if (duration < maxQueryTime) {
        return true;
    }
    QueryStatusLogUtil::LogQueryOverTime(duration, query.ToString(), tag.isInnerQuery);
    if (callback != nullptr) {
        callback(DbQueryStatus::OVER_TIME);
    }
    return tag.isInnerQuery;
}

bool SysEventQueryWrapper::IsConcurrentQueryCntValid(const std::string& dbFile, const DbQueryTag& tag,
    const DbQueryCallback& callback)
{
    std::lock_guard<std::mutex> lock(concurrentQueriesMutex_);
    auto iter = concurrentQueries_.find(dbFile);
    if (iter != concurrentQueries_.end()) {
        auto& concurrentQueryCnt = tag.isInnerQuery ? iter->second.first : iter->second.second;
        int conCurrentQueryCntLimit = 4;
        if (concurrentQueryCnt < conCurrentQueryCntLimit) {
            return true;
        }
        QueryStatusLogUtil::LogTooManyConcurrentQueries(conCurrentQueryCntLimit, tag.isInnerQuery);
        if (callback != nullptr) {
            callback(DbQueryStatus::CONCURRENT);
        }
        return tag.isInnerQuery;
    }
    concurrentQueries_[dbFile] = { DEFAULT_CONCURRENT_CNT, DEFAULT_CONCURRENT_CNT };
    return true;
}

bool SysEventQueryWrapper::IsQueryFrequenceValid(const DataQuery& query, const DbQueryTag& tag,
    const std::string& dbFile, const QueryProcessInfo& processInfo, const DbQueryCallback& callback)
{
    std::lock_guard<std::mutex> lock(lastQueriesMutex_);
    if (!tag.needFrequenceCheck) {
        return true;
    }
    time_t current;
    (void)time(&current);
    auto execIter = lastQueries_.find(dbFile);
    auto queryProcessId = processInfo.first;
    if (execIter == lastQueries_.end()) {
        lastQueries_[dbFile].insert(std::make_pair(queryProcessId, current));
        return true;
    }
    auto processIter = execIter->second.find(queryProcessId);
    if (processIter == execIter->second.end()) {
        execIter->second[queryProcessId] = current;
        return true;
    }
    time_t queryFrequent = 1;
    if (abs(current - processIter->second) > queryFrequent) {
        execIter->second[queryProcessId] = current;
        return true;
    }
    QueryStatusLogUtil::LogQueryTooFrequently(query.ToString(), processInfo.second, tag.isInnerQuery);
    if (callback != nullptr) {
        callback(DbQueryStatus::TOO_FREQENTLY);
    }
    return tag.isInnerQuery;
}

void SysEventQueryWrapper::IncreaseConcurrentCnt(const std::string& dbFile, const DbQueryTag& tag)
{
    std::lock_guard<std::mutex> lock(concurrentQueriesMutex_);
    auto iter = concurrentQueries_.find(dbFile);
    if (iter != concurrentQueries_.end()) {
        auto& concurrentQueryCnt = tag.isInnerQuery ? iter->second.first : iter->second.second;
        concurrentQueryCnt++;
        return;
    }
    concurrentQueries_[dbFile] = std::make_pair(DEFAULT_CONCURRENT_CNT, DEFAULT_CONCURRENT_CNT);
}

void SysEventQueryWrapper::DecreaseConcurrentCnt(const std::string& dbFile, const DbQueryTag& tag)
{
    std::lock_guard<std::mutex> lock(concurrentQueriesMutex_);
    auto iter = concurrentQueries_.find(dbFile);
    if (iter != concurrentQueries_.end()) {
        auto& concurrentQueryCnt = tag.isInnerQuery ? iter->second.first : iter->second.second;
        concurrentQueryCnt--;
    }
}
} // namespace EventStore
} // namespace HiviewDFX
} // namespace OHOS
