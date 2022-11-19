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

#include <cinttypes>

#include "hilog/log.h"
#include "ret_code.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D10, "HiView-SysEventQueryBuilder" };
constexpr char DOMAIN_[] = "domain_";
constexpr char NAME_[] = "name_";
constexpr char SEQ_[] = "seq_";
constexpr char LOGIC_AND_COND[] = "and";
constexpr char LOGIC_OR_COND[] = "or";
constexpr int64_t INVALID_SEQ = -1;
constexpr int MAX_QUERY_EVENTS = 1000; // The maximum number of queries is 1000 at one time
const std::vector<int> EVENT_TYPES = {1, 2, 3, 4}; // FAULT = 1, STATISTIC = 2 SECURITY = 3, BEHAVIOR = 4
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
    if (logic == LOGIC_OR_COND) {
        condition.Or(subCond);
    } else {
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

bool BaseEventQueryWrapper::Query(EventStore::QueryProcessInfo callerInfo, EventStore::DbQueryCallback queryCallback,
    EventStore::ResultSet& resultSet)
{
    if (query == nullptr) {
        HiLog::Debug(LABEL, "current query is null.");
        return false;
    }
    BuildQuery(query);
    EventStore::Cond domainNameConds;
    if (BuildConditonByDomainNameExtraInfo(domainNameConds)) {
        query->And(domainNameConds);
    }
    Order();
    HiLog::Debug(LABEL, "eventType[%{public}u] execute query: beginTime=%{public}" PRId64
        ", endTime=%{public}" PRId64 ", maxEvents=%{public}d, fromSeq=%{public}" PRId64
        ", toSeq=%{public}" PRId64 ", queryLimit=%{public}d.", eventType, argument.beginTime, argument.endTime,
        argument.maxEvents, argument.fromSeq, argument.toSeq, queryLimit);
    if (isFirstPartialQuery) {
        HiLog::Debug(LABEL, "current query is first partial query.");
    }
    resultSet = query->Execute(queryLimit, { false, isFirstPartialQuery }, callerInfo, queryCallback);
    SetIsFirstPartialQuery(false);
    if (HasNext()) {
        Next()->SetIsFirstPartialQuery(false);
    }
    return true;
}

bool BaseEventQueryWrapper::BuildConditonByDomainNameExtraInfo(EventStore::Cond& domainNameConds)
{
    auto hasDomainNameCond = false;
    for_each(domainNameExtraInfos.cbegin(), domainNameExtraInfos.cend(),
        [this, &hasDomainNameCond, &domainNameConds] (const auto& item) {
            if (this->HasDomainNameExtraConditon(domainNameConds, item)) {
                hasDomainNameCond = true;
            }
        });
    return hasDomainNameCond;
}

bool BaseEventQueryWrapper::HasDomainNameExtraConditon(EventStore::Cond& domainNameConds,
    const DomainNameExtraInfoMap::value_type& domainNameExtraInfo)
{
    auto isDomainCondsEmpty = true;
    auto isNameCondsEmpty = true;
    EventStore::Cond domainConds;
    if (!domainNameExtraInfo.first.empty()) {
        isDomainCondsEmpty = false;
        domainConds = EventStore::Cond(DOMAIN_, EventStore::Op::EQ, domainNameExtraInfo.first);
    }
    EventStore::Cond nameConds;
    for_each(domainNameExtraInfo.second.cbegin(), domainNameExtraInfo.second.cend(),
        [this, &nameConds, &isNameCondsEmpty] (const auto& item) {
            if (item.first.empty() && item.second.empty()) {
                return;
            }
            EventStore::Cond nameCond(NAME_, EventStore::Op::EQ, item.first);
            EventStore::Cond extraCond;
            if (!item.second.empty() &&
                (this->parser).ParseCondition(item.second, extraCond)) {
                nameCond.And(extraCond);
            }
            nameConds.Or(nameCond);
            isNameCondsEmpty = false;
        });
    if (isDomainCondsEmpty && isNameCondsEmpty) {
        return false;
    }
    if (!isDomainCondsEmpty && !isNameCondsEmpty) {
        domainConds.And(nameConds);
    } else if (isDomainCondsEmpty) {
        domainConds = nameConds;
    }
    domainNameConds.Or(domainConds);
    return true;
}

void BaseEventQueryWrapper::SetQueryArgument(QueryArgument argument)
{
    HiLog::Debug(LABEL, "eventType[%{public}u] set argument: beginTime=%{public} " PRId64
        ", endTime=%{public} " PRId64 ", maxEvents=%{public}d, fromSeq=%{public} " PRId64
        ", toSeq=%{public} " PRId64 ".", eventType, argument.beginTime, argument.endTime,
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

DomainNameExtraInfoMap& BaseEventQueryWrapper::GetDomainNameExtraInfoMap()
{
    return domainNameExtraInfos;
}

bool BaseEventQueryWrapper::IsValid() const
{
    return query != nullptr;
}

void BaseEventQueryWrapper::SetSysEventQuery(std::shared_ptr<EventStore::SysEventQuery> query)
{
    this->query = query;
}

void BaseEventQueryWrapper::SetNext(std::shared_ptr<BaseEventQueryWrapper> next)
{
    this->next = next;
}

bool BaseEventQueryWrapper::HasNext() const
{
    return next != nullptr;
}

std::shared_ptr<BaseEventQueryWrapper> BaseEventQueryWrapper::Next() const
{
    return next;
}

uint32_t BaseEventQueryWrapper::GetEventType() const
{
    return eventType;
}

bool BaseEventQueryWrapper::IsQueryComplete() const
{
    return argument.maxEvents <= 0;
}

void TimeStampEventQueryWrapper::BuildQuery(std::shared_ptr<EventStore::SysEventQuery> query)
{
    if (query == nullptr) {
        HiLog::Debug(LABEL, "query is null");
        return;
    }
    argument.beginTime = argument.beginTime < 0 ? 0 : argument.beginTime;
    argument.endTime = argument.endTime < 0 ? std::numeric_limits<int64_t>::max() : argument.endTime;
    argument.maxEvents = argument.maxEvents < 0 ? std::numeric_limits<int32_t>::max() : argument.maxEvents;
    queryLimit = argument.maxEvents < MAX_QUERY_EVENTS ? argument.maxEvents : MAX_QUERY_EVENTS;
    EventStore::Cond whereCond;
    whereCond.And(EventStore::EventCol::TS, EventStore::Op::GE, argument.beginTime)
        .And(EventStore::EventCol::TS, EventStore::Op::LT, argument.endTime);
    query->Where(whereCond);
}

void TimeStampEventQueryWrapper::SyncQueryArgument(const EventStore::ResultSet::RecordIter iter)
{
    argument.beginTime = static_cast<int64_t>(iter->happenTime_);
}

bool TimeStampEventQueryWrapper::NeedStartNextQuery(int32_t transEventTotalCnt, int32_t ignoredEventCnt)
{
    argument.maxEvents -= transEventTotalCnt;
    argument.beginTime++;
    if (((transEventTotalCnt + ignoredEventCnt) < queryLimit || argument.beginTime >= argument.endTime)) {
        if (HasNext()) {
            argument.beginTime = Next()->GetQueryArgument().beginTime;
            argument.endTime = Next()->GetQueryArgument().endTime;
            Next()->SetQueryArgument(argument);
        }
        return true;
    }
    return false;
}

void TimeStampEventQueryWrapper::Order()
{
    query->Order(EventStore::EventCol::TS, true);
}

void SeqEventQueryWrapper::BuildQuery(std::shared_ptr<EventStore::SysEventQuery> query)
{
    if (query == nullptr) {
        HiLog::Debug(LABEL, "query is null");
        return;
    }
    auto offset = static_cast<int32_t>(argument.toSeq - argument.fromSeq);
    queryLimit = offset < MAX_QUERY_EVENTS ? offset : MAX_QUERY_EVENTS;
    EventStore::Cond whereCond;
    whereCond.And(SEQ_, EventStore::Op::GE, argument.fromSeq)
            .And(SEQ_, EventStore::Op::LT, argument.toSeq);
    query->Where(whereCond);
}

void SeqEventQueryWrapper::SyncQueryArgument(const EventStore::ResultSet::RecordIter iter)
{
    argument.fromSeq = iter->GetEventSeq();
}

bool SeqEventQueryWrapper::NeedStartNextQuery(int32_t transEventTotalCnt, int32_t ignoredEventCnt)
{
    argument.maxEvents -= transEventTotalCnt;
    argument.fromSeq++;
    if (((transEventTotalCnt + ignoredEventCnt) < queryLimit || argument.fromSeq >= argument.toSeq)) {
        if (HasNext()) {
            argument.fromSeq = Next()->GetQueryArgument().fromSeq;
            argument.toSeq = Next()->GetQueryArgument().toSeq;
            Next()->SetQueryArgument(argument);
        }
        return true;
    }
    return false;
}

void SeqEventQueryWrapper::Order()
{
    query->Order(SEQ_, true);
}

EventQueryWrapperBuilder& EventQueryWrapperBuilder::Append(uint32_t eventType)
{
    HiLog::Debug(LABEL, "append eventType: %{public}u.", eventType);
    auto queryWrapper = Find(eventType);
    if (queryWrapper != nullptr && !queryWrapper->IsValid()) {
        queryWrapper->SetSysEventQuery(
            EventStore::SysEventDao::BuildQuery(static_cast<EventStore::StoreType>(eventType)));
    }
    return *shared_from_this();
}

EventQueryWrapperBuilder& EventQueryWrapperBuilder::Append(const std::string& domain, const std::string& eventName,
    uint32_t eventType, const std::string& extraInfo)
{
    HiLog::Debug(LABEL, "builder append domain=%{public}s, event name=%{public}s, event type=%{public}u,"
        "extra condition=%{public}s.", domain.c_str(), eventName.c_str(), eventType, extraInfo.c_str());
    auto queryWrapper = Find(eventType);
    if (queryWrapper == nullptr) {
        return *shared_from_this();
    }
    if (!queryWrapper->IsValid()) {
        queryWrapper->SetSysEventQuery(
            EventStore::SysEventDao::BuildQuery(static_cast<EventStore::StoreType>(eventType)));
    }
    auto eventNameWithExtraInfo = std::make_pair(eventName, extraInfo);
    auto& domainNameExtraInfos = queryWrapper->GetDomainNameExtraInfoMap();
    if (domainNameExtraInfos.empty()) {
        domainNameExtraInfos[domain] = { eventNameWithExtraInfo };
        return *shared_from_this();
    }
    auto domainNameExtraInfosIter = domainNameExtraInfos.find(domain);
    if (domainNameExtraInfosIter == domainNameExtraInfos.end()) {
        domainNameExtraInfos[domain] = { eventNameWithExtraInfo };
        return *shared_from_this();
    }
    auto& nameExtraInfos = domainNameExtraInfosIter->second;
    if (any_of(nameExtraInfos.begin(), nameExtraInfos.end(), [] (auto& nameExtraInfo) {
        return nameExtraInfo.first.empty() && nameExtraInfo.second.empty();
    })) {
        return *shared_from_this();
    }
    if (eventName.empty() && extraInfo.empty()) {
        nameExtraInfos.clear();
        nameExtraInfos.emplace(eventName, extraInfo);
        return *shared_from_this();
    }
    if (nameExtraInfos.find(eventName) == nameExtraInfos.end()) {
        nameExtraInfos.emplace(eventName, extraInfo);
        return *shared_from_this();
    }
    nameExtraInfos[eventName] = extraInfo;
    return *shared_from_this();
}

bool EventQueryWrapperBuilder::IsValid() const
{
    auto queryWrapper = Build();
    if (queryWrapper == nullptr) {
        return false;
    }
    while (queryWrapper != nullptr) {
        if (queryWrapper->IsValid()) {
            return true;
        }
        queryWrapper = queryWrapper->Next();
    }
    return false;
}

std::shared_ptr<BaseEventQueryWrapper> EventQueryWrapperBuilder::Build() const
{
    return queryWrapper;
}

std::shared_ptr<BaseEventQueryWrapper> EventQueryWrapperBuilder::CreateQueryWrapperByArgument(
    const QueryArgument& argument, uint32_t eventType, std::shared_ptr<EventStore::SysEventQuery> query)
{
    if (argument.fromSeq != INVALID_SEQ && argument.toSeq != INVALID_SEQ && argument.fromSeq < argument.toSeq) {
        return std::make_shared<SeqEventQueryWrapper>(eventType, query);
    }
    return std::make_shared<TimeStampEventQueryWrapper>(eventType, query);
}

void EventQueryWrapperBuilder::InitQueryWrappers(const QueryArgument& argument)
{
    HiLog::Debug(LABEL, "init link list of query wrapper with argument: beginTime=%{public} " PRId64
        ", endTime=%{public} " PRId64 ", maxEvents=%{public}d, fromSeq=%{public} " PRId64
        ", toSeq=%{public} " PRId64 ".", argument.beginTime, argument.endTime,
        argument.maxEvents, argument.fromSeq, argument.toSeq);
    std::shared_ptr<BaseEventQueryWrapper> queryWrapper = nullptr;
    for (auto& eventType : EVENT_TYPES) {
        if (queryWrapper == nullptr) {
            queryWrapper = CreateQueryWrapperByArgument(argument, eventType, nullptr);
            queryWrapper->SetQueryArgument(argument);
            this->queryWrapper = queryWrapper;
            continue;
        }
        queryWrapper->SetNext(CreateQueryWrapperByArgument(argument, eventType, nullptr));
        queryWrapper = queryWrapper->Next();
        queryWrapper->SetQueryArgument(argument);
    }
}

std::shared_ptr<BaseEventQueryWrapper> EventQueryWrapperBuilder::Find(uint32_t eventType)
{
    auto queryWrapper = Build();
    if (eventType == 0) {
        return queryWrapper;
    }
    while (queryWrapper != nullptr && queryWrapper->GetEventType() != eventType) {
        queryWrapper = queryWrapper->Next();
    }
    if (queryWrapper != nullptr && queryWrapper->GetEventType() == eventType) {
        return queryWrapper;
    }
    return nullptr;
}
} // namespace HiviewDFX
} // namespace OHOS
