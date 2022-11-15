/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "sys_event_service_ohos.h"

#include <codecvt>
#include <regex>
#include <set>

#include "accesstoken_kit.h"
#include "common_utils.h"
#include "hilog/log.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "json/json.h"
#include "ret_code.h"
#include "running_status_log_util.h"
#include "string_ex.h"
#include "system_ability_definition.h"

using namespace std;
using namespace OHOS::HiviewDFX::EventStore;

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D10, "HiView-SysEventService" };
constexpr int MAX_TRANS_BUF = 1024 * 768;  // Maximum transmission 768K at one time
constexpr int MAX_QUERY_EVENTS = 1000; // The maximum number of queries is 1000 at one time
constexpr int HID_ROOT = 0;
constexpr int HID_SHELL = 2000;
const std::vector<int> EVENT_TYPES = {1, 2, 3, 4}; // FAULT = 1, STATISTIC = 2 SECURITY = 3, BEHAVIOR = 4
constexpr uint32_t INVALID_EVENT_TYPE = 0;
const string READ_DFX_SYSEVENT_PERMISSION = "ohos.permission.READ_DFX_SYSEVENT";
const std::string LOGIC_AND_COND = "and";
const std::string LOGIC_OR_COND = "or";

bool MatchContent(int type, const string& rule, const string& match)
{
    if (match.empty()) {
        return false;
    }
    switch (type) {
        case RuleType::WHOLE_WORD:
            return rule.empty() || match.compare(rule) == 0;
        case RuleType::PREFIX:
            return rule.empty() || match.find(rule) == 0;
        case RuleType::REGULAR: {
                smatch result;
                const regex pattern(rule);
                return rule.empty() || regex_search(match, result, pattern);
            }
        default:
            HiLog::Error(LABEL, "invalid rule type %{public}d.", type);
            return false;
    }
}

bool MatchEventType(int rule, int match)
{
    return rule == INVALID_EVENT_TYPE || rule == match;
}

bool IsMatchedRule(const OHOS::HiviewDFX::SysEventRule& rule, const string& domain,
    const string& eventName, const string& tag, uint32_t eventType)
{
    if (rule.tag.empty()) {
        return MatchContent(rule.ruleType, rule.domain, domain)
            && MatchContent(rule.ruleType, rule.eventName, eventName)
            && MatchEventType(rule.eventType, eventType);
    }
    return MatchContent(rule.ruleType, rule.tag, tag)
        && MatchEventType(rule.eventType, eventType);
}

bool MatchRules(const SysEventRuleGroupOhos& rules, const string& domain, const string& eventName,
    const string& tag, uint32_t eventType)
{
    return any_of(rules.begin(), rules.end(), [domain, eventName, tag, eventType] (auto& rule) {
        if (IsMatchedRule(rule, domain, eventName, tag, eventType)) {
            string logFormat("rule type is %{public}d, domain is %{public}s, eventName is %{public}s, ");
            logFormat.append("tag is %{public}s, eventType is %{public}u for matched");
            HiLog::Debug(LABEL, logFormat.c_str(),
                rule.ruleType, rule.domain.empty() ? "empty" : rule.domain.c_str(),
                rule.eventName.empty() ? "empty" : rule.eventName.c_str(),
                rule.tag.empty() ? "empty" : rule.tag.c_str(), eventType);
            return true;
        }
        return false;
    });
}

int32_t CheckEventListenerAddingValidity(const std::vector<SysEventRule>& rules, RegisteredListeners& listeners)
{
    size_t watchRuleCntLimit = 20; // count of listener rule for each watcher is limited to 20.
    if (rules.size() > watchRuleCntLimit) {
        OHOS::HiviewDFX::RunningStatusLogUtil::LogTooManyWatchRules(rules);
        return ERR_TOO_MANY_WATCH_RULES;
    }
    size_t watcherTotalCntLimit = 30; // count of total watches is limited to 30.
    if (listeners.size() >= watcherTotalCntLimit) {
        OHOS::HiviewDFX::RunningStatusLogUtil::LogTooManyWatchers(watcherTotalCntLimit);
        return ERR_TOO_MANY_WATCHERS;
    }
    return IPC_CALL_SUCCEED;
}

int32_t CheckEventQueryingValidity(const SysEventQueryRuleGroupOhos& rules)
{
    size_t queryRuleCntLimit = 10; // count of query rule for each querier is limited to 10.
    if (rules.size() > queryRuleCntLimit) {
        OHOS::HiviewDFX::RunningStatusLogUtil::LogTooManyQueryRules(rules);
        return ERR_TOO_MANY_QUERY_RULES;
    }
    return IPC_CALL_SUCCEED;
}

bool ParseJsonString(const Json::Value& root, const std::string& key, std::string& value)
{
    if (!root.isMember(key.c_str()) || !root[key.c_str()].isString()) {
        return false;
    }
    value = root[key].asString();
    return true;
}

Op GetOpEnum(const std::string& op)
{
    const std::unordered_map<std::string, Op> opMap = {
        { "=", Op::EQ },
        { "<", Op::LT },
        { ">", Op::GT },
        { "<=", Op::LE },
        { ">=", Op::GE },
    };
    return opMap.find(op) == opMap.end() ? Op::NONE : opMap.at(op);
}

void SpliceConditionByLogic(Cond& condition, const Cond& subCond, const std::string& logic)
{
    if (logic == LOGIC_OR_COND) {
        condition.Or(subCond);
    } else {
        condition.And(subCond);
    }
}

bool ParseLogicCondition(const Json::Value& root, const std::string& logic, Cond& condition)
{
    if (!root.isMember(logic) || !root[logic].isArray()) {
        HiLog::Error(LABEL, "ParseLogicCondition err1.");
        return false;
    }

    Cond subCondition;
    for (size_t i = 0; i < root[logic].size(); ++i) {
        auto cond = root[logic][static_cast<int>(i)];
        std::string param;
        if (!ParseJsonString(cond, "param", param) || param.empty()) {
            return false;
        }
        std::string op;
        if (!ParseJsonString(cond, "op", op) || GetOpEnum(op) == Op::NONE) {
            return false;
        }
        const char valueKey[] = "value";
        if (!cond.isMember(valueKey)) {
            return false;
        }
        if (cond[valueKey].isString()) {
            std::string value = cond[valueKey].asString();
            SpliceConditionByLogic(subCondition, Cond(param, GetOpEnum(op), value), logic);
        } else if (cond[valueKey].isInt64()) {
            int64_t value = cond[valueKey].asInt64();
            SpliceConditionByLogic(subCondition, Cond(param, GetOpEnum(op), value), logic);
        } else {
            return false;
        }
    }
    condition.And(subCondition);
    return true;
}

bool ParseOrCondition(const Json::Value& root, Cond& condition)
{
    return ParseLogicCondition(root, LOGIC_OR_COND, condition);
}

bool ParseAndCondition(const Json::Value& root, Cond& condition)
{
    return ParseLogicCondition(root, LOGIC_AND_COND, condition);
}

bool ParseQueryConditionJson(const Json::Value& root, Cond& condition)
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

bool ParseQueryCondition(const std::string& condStr, Cond& condition)
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
}

OHOS::HiviewDFX::NotifySysEvent SysEventServiceOhos::gISysEventNotify_;
void SysEventServiceOhos::StartService(SysEventServiceBase *service,
    const OHOS::HiviewDFX::NotifySysEvent notify)
{
    gISysEventNotify_ = notify;
    GetSysEventService(service);
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        HiLog::Error(LABEL, "failed to find SystemAbilityManager.");
        return;
    }
    int ret = samgr->AddSystemAbility(DFX_SYS_EVENT_SERVICE_ABILITY_ID, &(SysEventServiceOhos::GetInstance()));
    if (ret != 0) {
        HiLog::Error(LABEL, "failed to add sys event service ability.");
    }
}

string SysEventServiceOhos::GetTagByDomainAndName(const string& eventDomain, const string& eventName)
{
    if (getTagFunc_ == nullptr) {
        return "";
    }
    return getTagFunc_(eventDomain, eventName);
}

uint32_t SysEventServiceOhos::GetTypeByDomainAndName(const string& eventDomain, const string& eventName)
{
    if (getTypeFunc_ == nullptr) {
        return INVALID_EVENT_TYPE;
    }
    return getTypeFunc_(eventDomain, eventName);
}

void SysEventServiceOhos::OnSysEvent(std::shared_ptr<OHOS::HiviewDFX::SysEvent>& event)
{
    lock_guard<mutex> lock(mutex_);
    for (auto listener = registeredListeners_.begin(); listener != registeredListeners_.end(); ++listener) {
        SysEventCallbackPtrOhos callback = iface_cast<ISysEventCallback>(listener->first);
        if (callback == nullptr) {
            HiLog::Error(LABEL, "interface is null, no need to match rules.");
            continue;
        }
        auto tag = GetTagByDomainAndName(event->domain_, event->eventName_);
        auto eventType = GetTypeByDomainAndName(event->domain_, event->eventName_);
        bool isMatched = MatchRules(listener->second.second, event->domain_, event->eventName_, tag, eventType);
        HiLog::Debug(LABEL, "pid %{public}d rules match %{public}s.", listener->second.first,
            isMatched ? "success" : "fail");
        if (isMatched) {
            callback->Handle(Str8ToStr16(event->domain_), Str8ToStr16(event->eventName_),
                static_cast<int>(event->what_), Str8ToStr16(event->jsonExtraInfo_));
        }
    }
}

void SysEventServiceOhos::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        HiLog::Error(LABEL, "remote is null");
        return;
    }
    auto remoteObject = remote.promote();
    if (remoteObject == nullptr) {
        HiLog::Error(LABEL, "object in remote is null.");
        return;
    }
    lock_guard<mutex> lock(mutex_);
    if (debugModeCallback_ != nullptr) {
        CallbackObjectOhos callbackObject = debugModeCallback_->AsObject();
        if (callbackObject == remoteObject && isDebugMode_) {
            HiLog::Error(LABEL, "quit debugmode.");
            auto event = std::make_shared<Event>("SysEventSource");
            event->messageType_ = Event::ENGINE_SYSEVENT_DEBUG_MODE;
            event->SetValue("DEBUGMODE", "false");
            gISysEventNotify_(event);
            isDebugMode_ = false;
        }
    }
    auto listener = registeredListeners_.find(remoteObject);
    if (listener != registeredListeners_.end()) {
        listener->first->RemoveDeathRecipient(deathRecipient_);
        HiLog::Error(LABEL, "pid %{public}d has died and remove listener.", listener->second.first);
        registeredListeners_.erase(listener);
    }
}

void SysEventServiceOhos::BindGetTagFunc(const GetTagByDomainNameFunc& getTagFunc)
{
    getTagFunc_ = getTagFunc;
}

void SysEventServiceOhos::BindGetTypeFunc(const GetTypeByDomainNameFunc& getTypeFunc)
{
    getTypeFunc_ = getTypeFunc;
}

SysEventServiceBase* SysEventServiceOhos::GetSysEventService(SysEventServiceBase* service)
{
    static SysEventServiceBase* ref = nullptr;
    if (service != nullptr) {
        ref = service;
    }
    return ref;
}

int32_t SysEventServiceOhos::AddListener(const std::vector<SysEventRule>& rules,
    const sptr<ISysEventCallback>& callback)
{
    if (!HasAccessPermission()) {
        HiLog::Error(LABEL, "access permission check failed");
        return ERR_NO_PERMISSION;
    }
    auto checkRet = CheckEventListenerAddingValidity(rules, registeredListeners_);
    if (checkRet != IPC_CALL_SUCCEED) {
        return checkRet;
    }
    auto service = GetSysEventService();
    if (service == nullptr) {
        HiLog::Error(LABEL, "subscribe fail, sys event service is null.");
        return ERR_REMOTE_SERVICE_IS_NULL;
    }
    if (callback == nullptr) {
        HiLog::Error(LABEL, "subscribe fail, callback is null.");
        return ERR_LISTENER_NOT_EXIST;
    }
    CallbackObjectOhos callbackObject = callback->AsObject();
    if (callbackObject == nullptr) {
        HiLog::Error(LABEL, "subscribe fail, object in callback is null.");
        return ERR_LISTENER_STATUS_INVALID;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t pid = IPCSkeleton::GetCallingPid();
    lock_guard<mutex> lock(mutex_);
    pair<int32_t, SysEventRuleGroupOhos> rulesPair(pid, rules);
    if (registeredListeners_.find(callbackObject) != registeredListeners_.end()) {
        registeredListeners_[callbackObject] = rulesPair;
        HiLog::Debug(LABEL, "uid %{public}d pid %{public}d listener has been added and update rules.", uid, pid);
        return IPC_CALL_SUCCEED;
    }
    if (!callbackObject->AddDeathRecipient(deathRecipient_)) {
        HiLog::Error(LABEL, "subscribe fail, can not add death recipient.");
        return ERR_ADD_DEATH_RECIPIENT;
    }
    registeredListeners_.insert(make_pair(callbackObject, rulesPair));
    HiLog::Debug(LABEL, "uid %{public}d pid %{public}d listener is added successfully, total is %{public}zu.",
        uid, pid, registeredListeners_.size());
    return IPC_CALL_SUCCEED;
}

int32_t SysEventServiceOhos::RemoveListener(const SysEventCallbackPtrOhos& callback)
{
    if (!HasAccessPermission()) {
        HiLog::Error(LABEL, "access permission check failed");
        return ERR_NO_PERMISSION;
    }
    auto service = GetSysEventService();
    if (service == nullptr) {
        HiLog::Error(LABEL, "sys event service is null.");
        return ERR_REMOTE_SERVICE_IS_NULL;
    }
    if (callback == nullptr) {
        HiLog::Error(LABEL, "callback is null.");
        return ERR_LISTENER_NOT_EXIST;
    }
    CallbackObjectOhos callbackObject = callback->AsObject();
    if (callbackObject == nullptr) {
        HiLog::Error(LABEL, "object in callback is null.");
        return ERR_LISTENER_STATUS_INVALID;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t pid = IPCSkeleton::GetCallingPid();
    lock_guard<mutex> lock(mutex_);
    if (registeredListeners_.empty()) {
        HiLog::Debug(LABEL, "has no any listeners.");
        return ERR_LISTENERS_EMPTY;
    }
    auto registeredListener = registeredListeners_.find(callbackObject);
    if (registeredListener != registeredListeners_.end()) {
        if (!callbackObject->RemoveDeathRecipient(deathRecipient_)) {
            HiLog::Error(LABEL, "uid %{public}d pid %{public}d listener can not remove death recipient.", uid, pid);
            return ERR_ADD_DEATH_RECIPIENT;
        }
        registeredListeners_.erase(registeredListener);
        HiLog::Debug(LABEL, "uid %{public}d pid %{public}d has found listener and removes it.", uid, pid);
        return IPC_CALL_SUCCEED;
    } else {
        HiLog::Debug(LABEL, "uid %{public}d pid %{public}d has not found listener.", uid, pid);
        return ERR_LISTENER_NOT_EXIST;
    }
}

int64_t SysEventServiceOhos::TransSysEvent(ResultSet& result, const QuerySysEventCallbackPtrOhos& callback,
    QueryTimeRange& queryTimeRange, int32_t& drops)
{
    std::vector<u16string> events;
    std::vector<int64_t> seqs;
    ResultSet::RecordIter iter;
    int32_t curTotal = 0;
    int32_t totalRecords = 0;
    while (result.HasNext()) {
        iter = result.Next();
        u16string curJson = Str8ToStr16(iter->jsonExtraInfo_);
        int32_t jsonSize = static_cast<int32_t>((curJson.size() + 1) * sizeof(u16string));
        if (jsonSize > MAX_TRANS_BUF) { // too large events, drop
            drops++;
            continue;
        }
        if (jsonSize + curTotal > MAX_TRANS_BUF) {
            callback->OnQuery(events, seqs);
            events.clear();
            seqs.clear();
            curTotal = 0;
        }
        events.push_back(curJson);
        seqs.push_back(iter->GetSeq());
        totalRecords++;
        curTotal += jsonSize;
        queryTimeRange.first = static_cast<int64_t>(iter->happenTime_);
    }

    if (events.size()) {
        callback->OnQuery(events, seqs);
    }

    return totalRecords;
}

void SysEventServiceOhos::BuildQueryArgs(QueryArgs& queryArgs, const std::string& domain,
    const std::string& eventName, uint32_t eventType) const
{
    HiLog::Debug(LABEL, "build query args with domain=%{public}s, event name=%{public}s, event type=%{public}u.",
        domain.c_str(), eventName.c_str(), eventType);
    auto typeDomainsIter = queryArgs.find(eventType);
    if (typeDomainsIter == queryArgs.end()) {
        EventNames names = {eventName};
        DomainsWithNames domainsWithNames = {{domain, names}};
        queryArgs.insert(std::make_pair(eventType, domainsWithNames));
        return;
    }
    auto& domains = typeDomainsIter->second;
    auto domainNamesIter = domains.find(domain);
    if (domainNamesIter == domains.end()) {
        EventNames names = { eventName };
        domains.insert(std::make_pair(domain, names));
        return;
    }
    auto& names = domainNamesIter->second;
    if (any_of(names.begin(), names.end(), [] (auto& name) {
        return name.empty();
    })) {
        return;
    }
    if (eventName.empty()) {
        names.clear();
        names.emplace_back(eventName);
        return;
    }
    if (find(names.cbegin(), names.cend(), eventName) == names.cend()) {
        names.emplace_back(eventName);
    }
}

void SysEventServiceOhos::ParseQueryArgs(const SysEventQueryRuleGroupOhos& rules, QueryArgs& queryArgs,
    QueryConds& extConds)
{
    if (rules.empty()) {
        for (auto eventType : EVENT_TYPES) {
            queryArgs.insert(std::make_pair(eventType, DomainsWithNames()));
        }
        return;
    }
    for (auto ruleIter = rules.cbegin(); ruleIter < rules.cend(); ++ruleIter) {
        Cond extCond;
        bool isCondValid = false;
        if (ParseQueryCondition(ruleIter->condition, extCond)) {
            isCondValid = true;
        }
        for (auto nameIter = ruleIter->eventList.cbegin(); nameIter < ruleIter->eventList.cend(); ++nameIter) {
            std::string eventName = *nameIter;
            auto eventType = GetTypeByDomainAndName(ruleIter->domain, eventName);
            if (eventType != INVALID_EVENT_TYPE && ruleIter->eventType != INVALID_EVENT_TYPE &&
                eventType != ruleIter->eventType) {
                HiLog::Warn(LABEL, "domain=%{public}s, event name=%{public}s: event type configured is %{public}u, "
                    "no match with query event type which is %{public}u.",
                    ruleIter->domain.c_str(), eventName.c_str(), eventType, ruleIter->eventType);
                continue;
            }
            if (isCondValid) {
                extConds.insert({std::make_pair(ruleIter->domain, eventName), extCond});
            }
            if (ruleIter->eventType != INVALID_EVENT_TYPE) { // for query by type
                BuildQueryArgs(queryArgs, ruleIter->domain, eventName, ruleIter->eventType);
                continue;
            }
            if (eventType != INVALID_EVENT_TYPE) { // for query by domain and name
                BuildQueryArgs(queryArgs, ruleIter->domain, eventName, eventType);
                continue;
            }
            for (auto type : EVENT_TYPES) {
                BuildQueryArgs(queryArgs, ruleIter->domain, eventName, type);
            }
        }
    }
}

bool SysEventServiceOhos::HasDomainNameConditon(EventStore::Cond& domainNameConds,
    const DomainsWithNames::value_type& domainNames, const QueryConds& extConds) const
{
    auto isDomainCondsEmpty = true;
    auto isNameCondsEmpty = true;
    Cond domainConds;
    if (!domainNames.first.empty()) {
        isDomainCondsEmpty = false;
        domainConds = Cond("domain_", Op::EQ, domainNames.first);
    }
    Cond nameConds;
    for_each(domainNames.second.cbegin(), domainNames.second.cend(),
        [&nameConds, &isNameCondsEmpty, &extConds, &domainNames] (const std::string& name) {
            if (name.empty()) {
                return;
            }
            Cond nameCond("name_", Op::EQ, name);
            if (auto pair = std::make_pair(domainNames.first, name); extConds.find(pair) != extConds.end()) {
                nameCond.And(extConds.at(pair));
            }
            nameConds.Or(nameCond);
            isNameCondsEmpty = false;
        });
    if (isDomainCondsEmpty && isNameCondsEmpty) {
        return false;
    }
    if (!isDomainCondsEmpty && !isNameCondsEmpty) {
        domainConds.And(nameConds);
    } else if (isDomainCondsEmpty && !isNameCondsEmpty) {
        domainConds = nameConds;
    }
    domainNameConds.Or(domainConds);
    return true;
}

std::shared_ptr<SysEventQuery> SysEventServiceOhos::BuildSysEventQuery(QueryArgs::const_iterator queryArgIter,
    const QueryConds& extConds, const QueryTimeRange& timeRange) const
{
    Cond timeCond;
    timeCond.And(EventCol::TS, Op::GE, timeRange.first).And(EventCol::TS, Op::LT, timeRange.second);
    auto sysEventQuery = SysEventDao::BuildQuery(static_cast<StoreType>(queryArgIter->first));
    Cond domainNameConds;
    bool hasDomainNameCond = false;
    for_each(queryArgIter->second.cbegin(), queryArgIter->second.cend(),
        [this, &domainNameConds, &hasDomainNameCond, &extConds] (const DomainsWithNames::value_type& domainNames) {
            if (this->HasDomainNameConditon(domainNameConds, domainNames, extConds)) {
                hasDomainNameCond = true;
            }
        });
    if (hasDomainNameCond) {
        (*sysEventQuery).Where(timeCond).And(domainNameConds).Order(EventCol::TS, true);
    } else {
        (*sysEventQuery).Where(timeCond).Order(EventCol::TS, true);
    }
    return sysEventQuery;
}

uint32_t SysEventServiceOhos::QuerySysEventMiddle(const std::shared_ptr<SysEventQuery>& sysEventQuery,
    int32_t maxEvents, bool isFirstPartialQuery, ResultSet& result)
{
    std::string processName = CommonUtils::GetProcNameByPid(IPCSkeleton::GetCallingPid());
    processName = processName.empty() ? "unknown" : processName;
    QueryProcessInfo callInfo = std::make_pair(IPCSkeleton::GetCallingPid(), processName);
    uint32_t queryRetCode = IPC_CALL_SUCCEED;
    result = sysEventQuery->Execute(maxEvents, { false, isFirstPartialQuery }, callInfo,
        [&queryRetCode] (DbQueryStatus status) {
            std::unordered_map<DbQueryStatus, uint32_t> statusToCode {
                { DbQueryStatus::CONCURRENT, ERR_TOO_MANY_CONCURRENT_QUERIES },
                { DbQueryStatus::OVER_TIME, ERR_QUERY_OVER_TIME },
                { DbQueryStatus::OVER_LIMIT, ERR_QUERY_OVER_LIMIT },
                { DbQueryStatus::TOO_FREQENTLY, ERR_QUERY_TOO_FREQUENTLY },
            };
            queryRetCode = statusToCode[status];
        });
    return queryRetCode;
}

int32_t SysEventServiceOhos::Query(int64_t beginTime, int64_t endTime, int32_t maxEvents,
    const SysEventQueryRuleGroupOhos& rules, const QuerySysEventCallbackPtrOhos& callback)
{
    if (!HasAccessPermission()) {
        HiLog::Error(LABEL, "access permission check failed.");
        callback->OnComplete(ERR_NO_PERMISSION, 0);
        return ERR_NO_PERMISSION;
    }
    auto checkRet = CheckEventQueryingValidity(rules);
    if (checkRet != IPC_CALL_SUCCEED) {
        callback->OnComplete(checkRet, 0);
        return checkRet;
    }
    QueryArgs queryArgs;
    QueryConds extConds;
    ParseQueryArgs(rules, queryArgs, extConds);
    if (queryArgs.empty()) {
        HiLog::Warn(LABEL, "no valid query rule matched, exit event querying.");
        callback->OnComplete(ERR_DOMIAN_INVALID, 0);
        return ERR_DOMIAN_INVALID;
    }
    auto realBeginTime = beginTime < 0 ? 0 : beginTime;
    auto realEndTime = endTime < 0 ? std::numeric_limits<int64_t>::max() : endTime;
    QueryTimeRange queryTimeRange = std::make_pair(realBeginTime, realEndTime);
    auto remainCnt = maxEvents < 0 ? std::numeric_limits<int32_t>::max() : maxEvents;
    auto totalEventCnt = 0;
    auto queryTypeIter = queryArgs.cbegin();
    bool isFirstPartialQuery = true;
    while (remainCnt > 0) {
        ResultSet ret;
        auto queryLimit = remainCnt < MAX_QUERY_EVENTS ? remainCnt : MAX_QUERY_EVENTS;
        uint32_t queryRetCode = QuerySysEventMiddle(BuildSysEventQuery(queryTypeIter, extConds, queryTimeRange),
            queryLimit, isFirstPartialQuery, ret);
        if (queryRetCode != IPC_CALL_SUCCEED) {
            callback->OnComplete(queryRetCode, totalEventCnt);
            return queryRetCode;
        }
        auto dropCnt = 0;
        auto queryRetCnt = TransSysEvent(ret, callback, queryTimeRange, dropCnt);
        queryTimeRange.first++;
        totalEventCnt += queryRetCnt;
        remainCnt -= queryRetCnt;
        if ((queryRetCnt + dropCnt) < queryLimit || queryTimeRange.first >= queryTimeRange.second) {
            if ((++queryTypeIter) != queryArgs.cend()) {
                queryTimeRange.first = realBeginTime;
                queryTimeRange.second = realEndTime;
            } else {
                break;
            }
        }
        isFirstPartialQuery = false;
    }
    callback->OnComplete(0, totalEventCnt);
    return IPC_CALL_SUCCEED;
}

bool SysEventServiceOhos::HasAccessPermission() const
{
    using namespace Security::AccessToken;
    const int callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid == HID_SHELL || callingUid == HID_ROOT) {
        return true;
    }
    uint32_t tokenId = IPCSkeleton::GetFirstTokenID();
    if (tokenId == 0) {
        tokenId = IPCSkeleton::GetCallingTokenID();
    }
    if (AccessTokenKit::VerifyAccessToken(tokenId, READ_DFX_SYSEVENT_PERMISSION) == RET_SUCCESS) {
        return true;
    }
    HiLog::Error(LABEL, "hiview service permission denial callingUid=%{public}d", callingUid);
    return false;
}

int32_t SysEventServiceOhos::SetDebugMode(const SysEventCallbackPtrOhos& callback, bool mode)
{
    if (!HasAccessPermission()) {
        HiLog::Error(LABEL, "permission denied");
        return ERR_NO_PERMISSION;
    }

    if (mode == isDebugMode_) {
        HiLog::Error(LABEL, "same config, no need set");
        return ERR_DEBUG_MODE_SET_REPEAT;
    }

    auto event = std::make_shared<Event>("SysEventSource");
    event->messageType_ = Event::ENGINE_SYSEVENT_DEBUG_MODE;
    event->SetValue("DEBUGMODE", mode ? "true" : "false");
    gISysEventNotify_(event);

    HiLog::Debug(LABEL, "set debug mode %{public}s", mode ? "true" : "false");
    debugModeCallback_ = callback;
    isDebugMode_ = mode;
    return IPC_CALL_SUCCEED;
}

int SysEventServiceOhos::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    if (fd < 0) {
        HiLog::Error(LABEL, "invalid fd.");
        return -1;
    }
    dprintf(fd, "%s\n", "Hiview SysEventService");
    return 0;
}

void CallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    SysEventServiceOhos::GetInstance().OnRemoteDied(remote);
}
}  // namespace HiviewDFX
}  // namespace OHOS