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

#ifndef OHOS_HIVIEWDFX_EVENT_QUERY_WRAPPER_BUILDER_H
#define OHOS_HIVIEWDFX_EVENT_QUERY_WRAPPER_BUILDER_H

#include <unordered_map>

#include "iquery_sys_event_callback.h"
#include "json/json.h"
#include "query_argument.h"
#include "sys_event_dao.h"
#include "sys_event_query.h"
#include "sys_event_query_rule.h"

namespace OHOS {
namespace HiviewDFX {
using ExtraInfoConditionMap = std::unordered_map<std::string, EventStore::Cond>;
using EventNameExtraInfoMap = std::unordered_map<std::string, std::string>;
using DomainNameExtraInfoMap = std::unordered_map<std::string, EventNameExtraInfoMap>;

class ConditionParser {
public:
    bool ParseCondition(const std::string& condStr, EventStore::Cond& condition);

private:
    void SpliceConditionByLogic(EventStore::Cond& condition, const EventStore::Cond& subCond,
        const std::string& logic);
    bool ParseJsonString(const Json::Value& root, const std::string& key, std::string& value);
    bool ParseLogicCondition(const Json::Value& root, const std::string& logic, EventStore::Cond& condition);
    bool ParseOrCondition(const Json::Value& root, EventStore::Cond& condition);
    bool ParseAndCondition(const Json::Value& root, EventStore::Cond& condition);
    bool ParseQueryConditionJson(const Json::Value& root, EventStore::Cond& condition);
    bool ParseQueryCondition(const std::string& condStr, EventStore::Cond& condition);
    EventStore::Op GetOpEnum(const std::string& op);

private:
    ExtraInfoConditionMap extraInfoCondCache;
};

class BaseEventQueryWrapper {
public:
    BaseEventQueryWrapper(uint32_t eventType, std::shared_ptr<EventStore::SysEventQuery> query)
        : query(query), eventType(eventType) {}
    virtual ~BaseEventQueryWrapper() {}

public:
    void Query(OHOS::sptr<OHOS::HiviewDFX::IQuerySysEventCallback> eventQueryCallback, int32_t& queryResult);
    void SetQueryArgument(QueryArgument argument);
    void SetIsFirstPartialQuery(bool isFirstPartialQuery);
    void SetSysEventQuery(std::shared_ptr<EventStore::SysEventQuery> query);
    void SetNext(std::shared_ptr<BaseEventQueryWrapper> next);
    QueryArgument& GetQueryArgument();
    DomainNameExtraInfoMap& GetDomainNameExtraInfoMap();
    int64_t GetMaxSequence() const;
    int64_t GetEventTotalCount() const;
    void TransportSysEvent(OHOS::HiviewDFX::EventStore::ResultSet& result,
        const OHOS::sptr<OHOS::HiviewDFX::IQuerySysEventCallback> callback, std::pair<int64_t, int32_t>& details);

public:
    virtual void SetMaxSequence(int64_t maxSeq) = 0;
    void SetEventTotalCount(int64_t totalCount);

public:
    bool IsValid() const;
    bool HasNext() const;
    std::shared_ptr<BaseEventQueryWrapper> Next() const;
    uint32_t GetEventType() const;
    bool IsQueryComplete() const;

protected:
    virtual bool NeedStartNextQuery() = 0;
    virtual void SyncQueryArgument(const EventStore::ResultSet::RecordIter iter) = 0;
    virtual void BuildQuery(std::shared_ptr<EventStore::SysEventQuery> query) = 0;
    virtual void Order() = 0;

protected:
    QueryArgument argument;
    int32_t queryLimit = 0;
    int64_t maxSeq = 0;
    int64_t totalEventCnt = 0;
    int64_t transportedEventCnt = 0;
    int32_t ignoredEventCnt = 0;
    std::shared_ptr<EventStore::SysEventQuery> query = nullptr;

private:
    uint32_t eventType = -1; // invalid event type is -1
    bool isFirstPartialQuery = true;
    std::shared_ptr<BaseEventQueryWrapper> next = nullptr;
    ConditionParser parser;
    DomainNameExtraInfoMap domainNameExtraInfos;

private:
    bool BuildConditonByDomainNameExtraInfo(EventStore::Cond& domainNameConds);
    bool HasDomainNameExtraConditon(EventStore::Cond& domainNameConds,
        const DomainNameExtraInfoMap::value_type& domainNameExtraInfo);
    void HandleCurrentQueryDone(OHOS::sptr<OHOS::HiviewDFX::IQuerySysEventCallback> callback,
        int32_t& queryResult);
};

class TimeStampEventQueryWrapper final : public BaseEventQueryWrapper {
public:
    TimeStampEventQueryWrapper(uint32_t eventType, std::shared_ptr<EventStore::SysEventQuery> query)
        : BaseEventQueryWrapper(eventType, query) {}

public:
    virtual void SyncQueryArgument(const EventStore::ResultSet::RecordIter iter);
    virtual bool NeedStartNextQuery();
    virtual void SetMaxSequence(int64_t maxSeq);

private:
    virtual void BuildQuery(std::shared_ptr<EventStore::SysEventQuery> query);
    virtual void Order();
};

class SeqEventQueryWrapper final : public BaseEventQueryWrapper {
public:
    SeqEventQueryWrapper(uint32_t eventType, std::shared_ptr<EventStore::SysEventQuery> query)
        : BaseEventQueryWrapper(eventType, query) {}

public:
    virtual void SyncQueryArgument(const EventStore::ResultSet::RecordIter iter);
    virtual bool NeedStartNextQuery();
    virtual void SetMaxSequence(int64_t maxSeq);

private:
    virtual void BuildQuery(std::shared_ptr<EventStore::SysEventQuery> query);
    virtual void Order();
};

class EventQueryWrapperBuilder final : public std::enable_shared_from_this<EventQueryWrapperBuilder> {
public:
    EventQueryWrapperBuilder(const QueryArgument& queryArgument)
    {
        InitQueryWrappers(queryArgument);
    }

public:
    EventQueryWrapperBuilder& Append(uint32_t eventType);
    EventQueryWrapperBuilder& Append(const std::string& domain, const std::string& eventName, uint32_t eventType,
        const std::string& extraInfo);
    std::shared_ptr<BaseEventQueryWrapper> Build() const;
    bool IsValid() const;

private:
    std::shared_ptr<BaseEventQueryWrapper> CreateQueryWrapperByArgument(const QueryArgument& argument,
        uint32_t eventType, std::shared_ptr<EventStore::SysEventQuery> query);
    void InitQueryWrappers(const QueryArgument& argument);
    std::shared_ptr<BaseEventQueryWrapper> Find(uint32_t eventType = 0);

private:
   std::shared_ptr<BaseEventQueryWrapper> queryWrapper = nullptr;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_EVENT_QUERY_WRAPPER_BUILDER_H