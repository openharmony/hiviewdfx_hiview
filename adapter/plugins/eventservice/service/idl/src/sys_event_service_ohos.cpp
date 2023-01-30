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
#include "event_query_wrapper_builder.h"
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
constexpr pid_t HID_ROOT = 0;
constexpr pid_t HID_SHELL = 2000;
constexpr pid_t HID_OHOS = 1000;
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
sptr<OHOS::HiviewDFX::SysEventServiceOhos> SysEventServiceOhos::instance(new SysEventServiceOhos);

sptr<OHOS::HiviewDFX::SysEventServiceOhos> SysEventServiceOhos::GetInstance()
{
    return instance;
}

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
    if (instance == nullptr) {
        HiLog::Error(LABEL, "SysEventServiceOhos service is null.");
        return;
    }
    int ret = samgr->AddSystemAbility(DFX_SYS_EVENT_SERVICE_ABILITY_ID, instance);
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

void SysEventServiceOhos::UpdateEventSeq(int64_t seq)
{
    curSeq.store(seq, std::memory_order_release);
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

bool SysEventServiceOhos::BuildEventQuery(std::shared_ptr<EventQueryWrapperBuilder> builder,
    const SysEventQueryRuleGroupOhos& rules)
{
    if (builder == nullptr) {
        return false;
    }
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (rules.empty() && (callingUid == HID_SHELL || callingUid == HID_ROOT ||
        callingUid == HID_OHOS)) {
        for (auto eventType : EVENT_TYPES) {
            builder->Append(eventType);
        }
        return true;
    }
    return !any_of(rules.cbegin(), rules.cend(), [this, callingUid, &builder] (auto& rule) {
        if (rule.domain.empty() && callingUid != HID_SHELL && callingUid != HID_ROOT &&
            callingUid != HID_OHOS) {
            return true;
        }
        return any_of(rule.eventList.cbegin(), rule.eventList.cend(),
            [this, callingUid, &builder, &rule] (auto& eventName) {
                if (eventName.empty() && callingUid != HID_SHELL && callingUid != HID_ROOT &&
                    callingUid != HID_OHOS) {
                    return true;
                }
                auto eventType = this->GetTypeByDomainAndName(rule.domain, eventName);
                if ((!rule.domain.empty() && !eventName.empty() && eventType == INVALID_EVENT_TYPE) ||
                    (eventType != INVALID_EVENT_TYPE && rule.eventType != INVALID_EVENT_TYPE &&
                    eventType != rule.eventType)) {
                    HiLog::Warn(LABEL, "domain=%{public}s, event name=%{public}s: "
                        "event type configured is %{public}u, "
                        "no match with query event type which is %{public}u.",
                        rule.domain.c_str(), eventName.c_str(), eventType, rule.eventType);
                    return false;
                }
                if (eventType != INVALID_EVENT_TYPE && rule.eventType == INVALID_EVENT_TYPE) {
                    builder->Append(rule.domain, eventName, eventType, rule.condition);
                    return false;
                }
                if (rule.eventType != INVALID_EVENT_TYPE) {
                    builder->Append(rule.domain, eventName, rule.eventType, rule.condition);
                    return false;
                }
                for (auto type : EVENT_TYPES) {
                    builder->Append(rule.domain, eventName, type, rule.condition);
                }
                return false;
            });
    });
}

int32_t SysEventServiceOhos::Query(const QueryArgument& queryArgument, const SysEventQueryRuleGroupOhos& rules,
    const OHOS::sptr<OHOS::HiviewDFX::IQuerySysEventCallback>& callback)
{
    if (callback == nullptr) {
        return ERR_LISTENER_NOT_EXIST;
    }
    if (!HasAccessPermission()) {
        callback->OnComplete(ERR_NO_PERMISSION, 0, curSeq.load(std::memory_order_acquire));
        return ERR_NO_PERMISSION;
    }
    auto checkRet = CheckEventQueryingValidity(rules);
    if (checkRet != IPC_CALL_SUCCEED) {
        callback->OnComplete(checkRet, 0, curSeq.load(std::memory_order_acquire));
        return checkRet;
    }
    auto queryWrapperBuilder = std::make_shared<EventQueryWrapperBuilder>(queryArgument);
    auto buildRet = BuildEventQuery(queryWrapperBuilder, rules);
    if (!buildRet || queryWrapperBuilder == nullptr || !queryWrapperBuilder->IsValid()) {
        HiLog::Warn(LABEL, "invalid query rule, exit sys event querying.");
        callback->OnComplete(ERR_QUERY_RULE_INVALID, 0, curSeq.load(std::memory_order_acquire));
        return ERR_QUERY_RULE_INVALID;
    }
    if (queryArgument.maxEvents == 0) {
        HiLog::Warn(LABEL, "query count is 0, query complete directly.");
        callback->OnComplete(IPC_CALL_SUCCEED, 0, curSeq.load(std::memory_order_acquire));
        return IPC_CALL_SUCCEED;
    }
    auto queryWrapper = queryWrapperBuilder->Build();
    if (queryWrapper == nullptr) {
        HiLog::Warn(LABEL, "query wrapper build failed.");
        callback->OnComplete(ERR_QUERY_RULE_INVALID, 0, curSeq.load(std::memory_order_acquire));
        return ERR_QUERY_RULE_INVALID;
    }
    queryWrapper->SetMaxSequence(curSeq.load(std::memory_order_acquire));
    auto queryRetCode = IPC_CALL_SUCCEED;
    queryWrapper->Query(callback, queryRetCode);
    return queryRetCode;
}

bool SysEventServiceOhos::HasAccessPermission() const
{
    using namespace Security::AccessToken;
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid == HID_SHELL || callingUid == HID_ROOT || callingUid == HID_OHOS) {
        return true;
    }
    auto tokenId = IPCSkeleton::GetFirstTokenID();
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
    auto service = SysEventServiceOhos::GetInstance();
    if (service == nullptr) {
        HiLog::Error(LABEL, "SysEventServiceOhos service is null.");
        return;
    }
    service->OnRemoteDied(remote);
}
}  // namespace HiviewDFX
}  // namespace OHOS