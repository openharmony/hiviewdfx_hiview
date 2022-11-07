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

void SysEventServiceOhos::ParseQueryArgs(const SysEventQueryRuleGroupOhos& rules, QueryArgs& queryArgs)
{
    if (rules.empty()) {
        for (auto eventType : EVENT_TYPES) {
            queryArgs.insert(std::make_pair(eventType, DomainsWithNames()));
        }
        return;
    }
    for (auto ruleIter = rules.cbegin(); ruleIter < rules.cend(); ++ruleIter) {
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
            if (ruleIter->eventType != INVALID_EVENT_TYPE) {
                BuildQueryArgs(queryArgs, ruleIter->domain, eventName, ruleIter->eventType);
                continue;
            }
            for (auto type : EVENT_TYPES) {
                BuildQueryArgs(queryArgs, ruleIter->domain, eventName, type);
            }
        }
    }
}

bool SysEventServiceOhos::HasDomainNameConditon(EventStore::Cond& domainNameConds,
    const DomainsWithNames::value_type& domainNames) const
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
        [&nameConds, &isNameCondsEmpty] (const std::string& name) {
            if (name.empty()) {
                return;
            }
            nameConds.Or("name_", Op::EQ, name);
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

uint32_t SysEventServiceOhos::QuerySysEventMiddle(QueryArgs::const_iterator queryArgIter,
    const QueryTimeRange& timeRange, int32_t maxEvents, bool isFirstPartialQuery, ResultSet& result)
{
    auto sysEventQuery = SysEventDao::BuildQuery(static_cast<StoreType>(queryArgIter->first));
    Cond timeCond, domainNameConds;
    timeCond.And(EventCol::TS, Op::GE, timeRange.first).And(EventCol::TS, Op::LT, timeRange.second);
    bool hasDomainNameCond = any_of(queryArgIter->second.cbegin(), queryArgIter->second.cend(),
        [this, &domainNameConds] (const DomainsWithNames::value_type& domainNames) {
            return this->HasDomainNameConditon(domainNameConds, domainNames);
        });
    if (hasDomainNameCond) {
        (*sysEventQuery).Where(timeCond).And(domainNameConds).Order(EventCol::TS, true);
    } else {
        (*sysEventQuery).Where(timeCond).Order(EventCol::TS, true);
    }
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
        return ERR_NO_PERMISSION;
    }
    auto checkRet = CheckEventQueryingValidity(rules);
    if (checkRet != IPC_CALL_SUCCEED) {
        return checkRet;
    }
    QueryArgs queryArgs;
    ParseQueryArgs(rules, queryArgs);
    if (queryArgs.empty()) {
        HiLog::Warn(LABEL, "no valid query rule matched, exit event querying.");
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
        uint32_t queryRetCode = QuerySysEventMiddle(queryTypeIter, queryTimeRange, queryLimit,
            isFirstPartialQuery, ret);
        if (queryRetCode != IPC_CALL_SUCCEED) {
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