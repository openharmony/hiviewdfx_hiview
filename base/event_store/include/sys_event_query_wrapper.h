/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef HIVIEW_BASE_EVENT_STORE_INCLUDE_SYS_EVENT_QUERY_WRAPPER_H
#define HIVIEW_BASE_EVENT_STORE_INCLUDE_SYS_EVENT_QUERY_WRAPPER_H

#include <atomic>
#include <ctime>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "sys_event_query.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
using ConcurrentQueries = std::pair<int, int>;
using LastQueries = std::unordered_map<pid_t, time_t>;

class QueryStatusLogUtil {
public:
    static void LogTooManyQueryRules(const std::string sql);
    static void LogTooManyConcurrentQueries(const int limit, bool innerQuery = true);
    static void LogQueryOverTime(time_t costTime, const std::string sql, bool innerQuery = true);
    static void LogQueryCountOverLimit(const int32_t queryCount, const std::string& sql,
        bool innerQuery = true);
    static void LogQueryTooFrequently(const std::string& sql, const std::string& processName = std::string(""),
        bool innerQuery = true);

private:
    static void Logging(const std::string& detail);
};

class SysEventQueryWrapper : public SysEventQuery {
public:
    SysEventQueryWrapper(const std::string& domain, const std::vector<std::string>& names)
        : SysEventQuery(domain, names) {}
    SysEventQueryWrapper(const std::string& domain, const std::vector<std::string>& names,
        uint32_t type, int64_t toSeq) : SysEventQuery(domain, names, type, toSeq) {}
    ~SysEventQueryWrapper() {}

public:
    virtual ResultSet Execute(int limit, DbQueryTag tag, QueryProcessInfo callerInfo,
        DbQueryCallback queryCallback) override;

private:
    bool IsConditionCntValid(const DbQueryTag& tag);
    bool IsQueryCntLimitValid(const DbQueryTag& tag, const int limit, const DbQueryCallback& callback);
    bool IsQueryCostTimeValid(const DbQueryTag& tag, const time_t before, const time_t after,
        const DbQueryCallback& callback);
    bool IsConcurrentQueryCntValid(const DbQueryTag& tag, const DbQueryCallback& callback);
    bool IsQueryFrequenceValid(const DbQueryTag& tag, const QueryProcessInfo& processInfo,
        const DbQueryCallback& callback);
    void IncreaseConcurrentCnt(const DbQueryTag& tag);
    void DecreaseConcurrentCnt(const DbQueryTag& tag);

private:
    static ConcurrentQueries concurrentQueries_;
    static LastQueries lastQueries_;
    static std::mutex concurrentQueriesMutex_;
    static std::mutex lastQueriesMutex_;
};
} // namespace EventStore
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIVIEW_BASE_EVENT_STORE_INCLUDE_SYS_EVENT_QUERY_WRAPPER_H
