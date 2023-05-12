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

#include "event_query_wrapper_builder.h"

#include <algorithm>
#include <cinttypes>

#include "common_utils.h"
#include "hilog/log.h"
#include "ipc_skeleton.h"
#include "ret_code.h"
#include "string_ex.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D10, "HiView-SysEventQueryBuilder" };
constexpr char LOGIC_AND_COND[] = "and";
constexpr char LOGIC_OR_COND[] = "or";
constexpr int64_t INVALID_SEQ = -1;
constexpr int64_t TRANS_DEFAULT_CNT = 0;
constexpr int32_t IGNORED_DEFAULT_CNT = 0;
constexpr int MAX_QUERY_EVENTS = 1000; // The maximum number of queries is 1000 at one time
constexpr int MAX_TRANS_BUF = 1024 * 768;  // Maximum transmission 768K at one time
const std::vector<int> EVENT_TYPES = {1, 2, 3, 4}; // FAULT = 1, STATISTIC = 2 SECURITY = 3, BEHAVIOR = 4

EventStore::QueryProcessInfo GetCallingProcessInfo()
{
    std::string processName = CommonUtils::GetProcNameByPid(IPCSkeleton::GetCallingPid());
    processName = processName.empty() ? "unknown" : processName;
    return std::make_pair(IPCSkeleton::GetCallingPid(), processName);
}
}

bool ConditionParser::ParseCondition(const std::string& condStr, EventStore::Cond& condition)
{
    if (extraInfoCondCache.empty() || extraInfoCondCache.find(condStr) == extraInfoCondCache.end()) {
        EventStore::Cond cond;
        if (ParseQueryCondition(condStr, cond)) {
            extraInfoCondCache[condStr] = cond;
        }
    }
    auto iter = extraInfoCondCache.find(condStr);
    if (iter != extraInfoCondCache.end()) {
        condition = iter->second;
        return true;
    }
    return false;
}

bool ConditionParser::ParseJsonString(const Json::Value& root, const std::string& key, std::string& value)
{
    if (!root.isMember(key.c_str()) || !root[key.c_str()].isString()) {
        return false;
    }
    value = root[key].asString();
    return true;
}

EventStore::Op ConditionParser::GetOpEnum(const std::string& op)
{
    const std::unordered_map<std::string, EventStore::Op> opMap = {
        { "=", EventStore::Op::EQ },
        { "<", EventStore::Op::LT },
        { ">", EventStore::Op::GT },
        { "<=", EventStore::Op::LE },
        { ">=", EventStore::Op::GE },
    };
    return opMap.find(op) == opMap.end() ? EventStore::Op::NONE : opMap.at(op);
}

void ConditionParser::SpliceConditionByLogic(EventStore::Cond& condition, const EventStore::Cond& subCond,
    const std::string& logic)
{
    if (logic == LOGIC_AND_COND) {
        condition.And(subCond);
    }
}

bool ConditionParser::ParseLogicCondition(const Json::Value& root, const std::string& logic,
    EventStore::Cond& condition)
{
    if (!root.isMember(logic) || !root[logic].isArray()) {
        HiLog::Error(LABEL, "ParseLogicCondition err1.");
        return false;
    }

    EventStore::Cond subCondition;
    for (size_t i = 0; i < root[logic].size(); ++i) {
        auto cond = root[logic][static_cast<int>(i)];
        std::string param;
        if (!ParseJsonString(cond, "param", param) || param.empty()) {
            return false;
        }
        std::string op;
        if (!ParseJsonString(cond, "op", op) || GetOpEnum(op) == EventStore::Op::NONE) {
            return false;
        }
        const char valueKey[] = "value";
        if (!cond.isMember(valueKey)) {
            return false;
        }
        if (cond[valueKey].isString()) {
            std::string value = cond[valueKey].asString();
            SpliceConditionByLogic(subCondition, EventStore::Cond(param, GetOpEnum(op), value), logic);
        } else if (cond[valueKey].isInt64()) {
            int64_t value = cond[valueKey].asInt64();
            SpliceConditionByLogic(subCondition, EventStore::Cond(param, GetOpEnum(op), value), logic);
        } else {
            return false;
        }
    }
    condition.And(subCondition);
    return true;
}

bool ConditionParser::ParseOrCondition(const Json::Value& root, EventStore::Cond& condition)
{
    return ParseLogicCondition(root, LOGIC_OR_COND, condition);
}

bool ConditionParser::ParseAndCondition(const Json::Value& root, EventStore::Cond& condition)
{
    return ParseLogicCondition(root, LOGIC_AND_COND, condition);
}

bool ConditionParser::ParseQueryConditionJson(const Json::Value& root, EventStore::Cond& condition)
{
    const char condKey[] = "condition";
    if (!root.isMember(condKey) || !root[condKey].isObject()) {
        return false;
    }
    bool res = false;
    if (ParseOrCondition(root[condKey], condition)) {
        res = true;
    }
    if (ParseAndCondition(root[condKey], condition)) {
        res = true;
    }
    return res;
}

bool ConditionParser::ParseQueryCondition(const std::string& condStr, EventStore::Cond& condition)
{
    if (condStr.empty()) {
        return false;
    }
    Json::Value root;
    Json::CharReaderBuilder jsonRBuilder;
    Json::CharReaderBuilder::strictMode(&jsonRBuilder.settings_);
    std::unique_ptr<Json::CharReader> const reader(jsonRBuilder.newCharReader());
    JSONCPP_STRING errs;
    if (!reader->parse(condStr.data(), condStr.data() + condStr.size(), &root, &errs)) {
        HiLog::Error(LABEL, "failed to parse condition string: %{public}s.", condStr.c_str());
        return false;
    }
    std::string version;
    if (!ParseJsonString(root, "version", version)) {
        HiLog::Error(LABEL, "failed to parser version.");
        return false;
    }
    const std::set<std::string> versionSet = { "V1" }; // set is used for future expansion
    if (versionSet.find(version) == versionSet.end()) {
        HiLog::Error(LABEL, "version is invalid.");
        return false;
    }
    if (!ParseQueryConditionJson(root, condition)) {
        HiLog::Error(LABEL, "condition is invalid.");
        return false;
    }
    return true;
}

void BaseEventQueryWrapper::HandleCurrentQueryDone(OHOS::sptr<OHOS::HiviewDFX::IQuerySysEventCallback> callback,
    int32_t& queryResult)
{
    if (callback == nullptr) {
        return;
    }
    if (IsQueryComplete()) { // all queries have finished, call OnComplete directly
        callback->OnComplete(queryResult, totalEventCnt, maxSeq);
        return;
    }
    if (!queryRules.empty() && !NeedStartNextQuery()) { // keep current query
        Query(callback, queryResult);
        return;
    }
    callback->OnComplete(queryResult, totalEventCnt, maxSeq);
}

void BaseEventQueryWrapper::Query(OHOS::sptr<OHOS::HiviewDFX::IQuerySysEventCallback> eventQueryCallback,
    int32_t& queryResult)
{
    if (eventQueryCallback == nullptr) {
        queryResult = ERR_LISTENER_NOT_EXIST;
        return;
    }
    if (queryRules.empty()) {
        HandleCurrentQueryDone(eventQueryCallback, queryResult);
        return;
    }
    BuildQuery();
    HiLog::Debug(LABEL, "execute query: beginTime=%{public}" PRId64
        ", endTime=%{public}" PRId64 ", maxEvents=%{public}d, fromSeq=%{public}" PRId64
        ", toSeq=%{public}" PRId64 ", queryLimit=%{public}d.", argument.beginTime, argument.endTime,
        argument.maxEvents, argument.fromSeq, argument.toSeq, queryLimit);
    if (isFirstPartialQuery) {
        HiLog::Debug(LABEL, "current query is first partial query.");
    }
    auto resultSet = query->Execute(queryLimit, { false, isFirstPartialQuery }, GetCallingProcessInfo(),
        [&queryResult] (EventStore::DbQueryStatus status) {
            std::unordered_map<EventStore::DbQueryStatus, int32_t> statusToCode {
                { EventStore::DbQueryStatus::CONCURRENT, ERR_TOO_MANY_CONCURRENT_QUERIES },
                { EventStore::DbQueryStatus::OVER_TIME, ERR_QUERY_OVER_TIME },
                { EventStore::DbQueryStatus::OVER_LIMIT, ERR_QUERY_OVER_LIMIT },
                { EventStore::DbQueryStatus::TOO_FREQENTLY, ERR_QUERY_TOO_FREQUENTLY },
            };
            queryResult = statusToCode[status];
        });
    if (queryResult != IPC_CALL_SUCCEED) {
        eventQueryCallback->OnComplete(queryResult, totalEventCnt, maxSeq);
        return;
    }
    auto details = std::make_pair(TRANS_DEFAULT_CNT, IGNORED_DEFAULT_CNT);
    TransportSysEvent(resultSet, eventQueryCallback, details);
    transportedEventCnt = details.first;
    totalEventCnt += transportedEventCnt;
    ignoredEventCnt += details.second;
    SetIsFirstPartialQuery(false);
    argument.maxEvents -= transportedEventCnt;
    HandleCurrentQueryDone(eventQueryCallback, queryResult);
}

void BaseEventQueryWrapper::TransportSysEvent(OHOS::HiviewDFX::EventStore::ResultSet& result,
    const OHOS::sptr<OHOS::HiviewDFX::IQuerySysEventCallback> callback, std::pair<int64_t, int32_t>& details)
{
    std::vector<std::u16string> events;
    std::vector<int64_t> seqs;
    OHOS::HiviewDFX::EventStore::ResultSet::RecordIter iter;
    int32_t transTotalJsonSize = 0;
    while (result.HasNext()) {
        iter = result.Next();
        auto eventJsonStr = iter->AsJsonStr();
        if (eventJsonStr.empty()) {
            continue;
        }
        std::u16string curJson = Str8ToStr16(eventJsonStr);
        int32_t eventJsonSize = static_cast<int32_t>((curJson.size() + 1) * sizeof(std::u16string));
        if (eventJsonSize > MAX_TRANS_BUF) { // too large events, drop
            details.second++;
            continue;
        }
        if (eventJsonSize + transTotalJsonSize > MAX_TRANS_BUF) {
            callback->OnQuery(events, seqs);
            events.clear();
            seqs.clear();
            transTotalJsonSize = 0;
        }
        events.push_back(curJson);
        seqs.push_back(iter->GetSeq());
        details.first++;
        transTotalJsonSize += eventJsonSize;
        SyncQueryArgument(iter);
    }
    if (!events.empty()) {
        callback->OnQuery(events, seqs);
    }
}

void BaseEventQueryWrapper::BuildConditon(const std::string& condition)
{
    if (condition.empty()) {
        return;
    }
    EventStore::Cond extraCond;
    if (this->parser.ParseCondition(condition, extraCond)) {
        query->And(extraCond);
    } else {
        HiLog::Info(LABEL, "invalid query condition=%{public}s", condition.c_str());
    }
}

void BaseEventQueryWrapper::SetQueryArgument(QueryArgument argument)
{
    HiLog::Debug(LABEL, "set argument: beginTime=%{public} " PRId64
        ", endTime=%{public} " PRId64 ", maxEvents=%{public}d, fromSeq=%{public} " PRId64
        ", toSeq=%{public} " PRId64 ".", argument.beginTime, argument.endTime,
        argument.maxEvents, argument.fromSeq, argument.toSeq);
    this->argument = argument;
}

QueryArgument& BaseEventQueryWrapper::GetQueryArgument()
{
    return argument;
}

void BaseEventQueryWrapper::SetIsFirstPartialQuery(bool isFirstPartialQuery)
{
    this->isFirstPartialQuery = isFirstPartialQuery;
}

std::vector<SysEventQueryRule>& BaseEventQueryWrapper::GetSysEventQueryRules()
{
    return queryRules;
}

int64_t BaseEventQueryWrapper::GetMaxSequence() const
{
    return maxSeq;
}

int64_t BaseEventQueryWrapper::GetEventTotalCount() const
{
    return totalEventCnt;
}

bool BaseEventQueryWrapper::IsValid() const
{
    return !queryRules.empty();
}

bool BaseEventQueryWrapper::IsQueryComplete() const
{
    return argument.maxEvents <= 0;
}

void BaseEventQueryWrapper::SetEventTotalCount(int64_t totalCount)
{
    HiLog::Debug(LABEL, "SetEventTotalCount: %{public}" PRId64 ".", totalCount);
    totalEventCnt = totalCount;
}

void TimeStampEventQueryWrapper::BuildQuery()
{
    argument.beginTime = argument.beginTime < 0 ? 0 : argument.beginTime;
    argument.endTime = argument.endTime < 0 ? std::numeric_limits<int64_t>::max() : argument.endTime;
    argument.maxEvents = argument.maxEvents < 0 ? std::numeric_limits<int32_t>::max() : argument.maxEvents;
    queryLimit = argument.maxEvents < MAX_QUERY_EVENTS ? argument.maxEvents : MAX_QUERY_EVENTS;
    EventStore::Cond whereCond;
    whereCond.And(EventStore::EventCol::TS, EventStore::Op::GE, argument.beginTime)
        .And(EventStore::EventCol::TS, EventStore::Op::LT, argument.endTime);
    auto queryRule = queryRules.front();
    query = EventStore::SysEventDao::BuildQuery(queryRule.domain, queryRule.eventList, queryRule.eventType, INVALID_SEQ);
    query->Where(whereCond);
    BuildConditon(queryRule.condition);
    Order();
    queryRules.erase(queryRules.begin());
}

void TimeStampEventQueryWrapper::SyncQueryArgument(const EventStore::ResultSet::RecordIter iter)
{
    argument.beginTime = static_cast<int64_t>(iter->happenTime_);
}

void TimeStampEventQueryWrapper::SetMaxSequence(int64_t maxSeq)
{
    this->maxSeq = maxSeq;
}

bool TimeStampEventQueryWrapper::NeedStartNextQuery()
{
    argument.beginTime++;
    return (transportedEventCnt + ignoredEventCnt) < queryLimit;
}

void TimeStampEventQueryWrapper::Order()
{
    if (query == nullptr) {
        return;
    }
    query->Order(EventStore::EventCol::TS, true);
}

void SeqEventQueryWrapper::BuildQuery()
{
    auto offset = static_cast<int32_t>(argument.toSeq - argument.fromSeq);
    queryLimit = offset < MAX_QUERY_EVENTS ? offset : MAX_QUERY_EVENTS;
    EventStore::Cond whereCond;
    whereCond.And(EventStore::EventCol::SEQ, EventStore::Op::GE, argument.fromSeq)
            .And(EventStore::EventCol::SEQ, EventStore::Op::LT, argument.toSeq);
    auto queryRule = queryRules.front();
    query = EventStore::SysEventDao::BuildQuery(queryRule.domain, queryRule.eventList, queryRule.eventType, argument.toSeq);
    query->Where(whereCond);
    BuildConditon(queryRule.condition);
    Order();
    queryRules.erase(queryRules.begin());
}

void SeqEventQueryWrapper::SyncQueryArgument(const EventStore::ResultSet::RecordIter iter)
{
    argument.fromSeq = iter->GetEventSeq();
}

void SeqEventQueryWrapper::SetMaxSequence(int64_t maxSeq)
{
    this->maxSeq = maxSeq;
    HiLog::Debug(LABEL, "argument.toSeq is %{public}" PRId64 ", maxSeq is %{public}" PRId64 ".",
        argument.toSeq, maxSeq);
    argument.toSeq = std::min(argument.toSeq, maxSeq);
}

bool SeqEventQueryWrapper::NeedStartNextQuery()
{
    argument.fromSeq++;
    return (transportedEventCnt + ignoredEventCnt) < queryLimit;
}

void SeqEventQueryWrapper::Order()
{
    if (query == nullptr) {
        return;
    }
    query->Order(EventStore::EventCol::SEQ, true);
}

EventQueryWrapperBuilder& EventQueryWrapperBuilder::Append(const std::string& domain, const std::string& eventName,
    uint32_t eventType, const std::string& extraInfo)
{
    HiLog::Debug(LABEL, "builder append domain=%{public}s, name=%{public}s, type=%{public}u, condition=%{public}s.",
        domain.c_str(), eventName.c_str(), eventType, extraInfo.c_str());
    auto& queryRules = this->queryWrapper->GetSysEventQueryRules();
    for (auto& rule : queryRules) {
        // if the query rules are the same group, combine them
        if (rule.domain == domain && eventType == rule.eventType && extraInfo == rule.condition) {
            auto& eventList = rule.eventList;
            if (eventName.empty()) {
                eventList.clear();
            } else {
                eventList.push_back(eventName);
            }
            return *shared_from_this();
        }
    }
    // otherwise, create a new query rule
    std::vector<std::string> eventList;
    if (!eventName.empty()) {
        eventList.push_back(eventName);
    }
    queryRules.push_back(SysEventQueryRule(domain, eventList, RuleType::WHOLE_WORD, eventType, extraInfo));
    return *shared_from_this();
}

bool EventQueryWrapperBuilder::IsValid() const
{
    return queryWrapper->IsValid();
}

std::shared_ptr<BaseEventQueryWrapper> EventQueryWrapperBuilder::Build() const
{
    return queryWrapper;
}

std::shared_ptr<BaseEventQueryWrapper> EventQueryWrapperBuilder::CreateQueryWrapperByArgument(
    const QueryArgument& argument, std::shared_ptr<EventStore::SysEventQuery> query)
{
    if (argument.fromSeq != INVALID_SEQ && argument.toSeq != INVALID_SEQ && argument.fromSeq < argument.toSeq) {
        return std::make_shared<SeqEventQueryWrapper>(query);
    }
    return std::make_shared<TimeStampEventQueryWrapper>(query);
}

void EventQueryWrapperBuilder::InitQueryWrapper(const QueryArgument& argument)
{
    HiLog::Debug(LABEL, "init link list of query wrapper with argument: beginTime=%{public} " PRId64
        ", endTime=%{public} " PRId64 ", maxEvents=%{public}d, fromSeq=%{public} " PRId64
        ", toSeq=%{public} " PRId64 ".", argument.beginTime, argument.endTime,
        argument.maxEvents, argument.fromSeq, argument.toSeq);
    this->queryWrapper = CreateQueryWrapperByArgument(argument, nullptr);
    this->queryWrapper->SetQueryArgument(argument);
}
} // namespace HiviewDFX
} // namespace OHOS
