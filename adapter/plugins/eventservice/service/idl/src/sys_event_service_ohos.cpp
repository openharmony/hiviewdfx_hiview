/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "logger.h"
#include "system_ability_definition.h"

using namespace std;
using namespace OHOS::HiviewDFX::EventStore;
using MatchRule = OHOS::HiviewDFX::SysEventServiceOhos::RuleType;

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-SysEventService");

constexpr int MAX_TRANS_BUF = 1024 * 768;  // Maximum transmission 768 at one time
constexpr int MAX_QUERY_EVENTS = 1000; // The maximum number of queries is 1000 at one time
constexpr unsigned int MAX_EVENT_NAME_LENGTH = 32;
constexpr unsigned int MAX_DOMAIN_LENGTH = 16;
constexpr int HID_ROOT = 0;
constexpr int HID_SHELL = 2000;

OHOS::HiviewDFX::NotifySysEvent SysEventServiceOhos::gISysEventNotify;
void SysEventServiceOhos::StartService(OHOS::HiviewDFX::SysEventService *service,
    const OHOS::HiviewDFX::NotifySysEvent notify)
{
    gISysEventNotify = notify;
    GetSysEventService(service);
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        HIVIEW_LOGE("failed to find SystemAbilityManager.");
        return;
    }
    bool rel = samgr->AddSystemAbility(DFX_SYS_EVENT_SERVICE_ABILITY_ID, &(SysEventServiceOhos::GetInstance()));
    if (!rel) {
        HIVIEW_LOGE("failed to add sys event service ability.");
    }
}

static bool IsMatchRule(int type, const string& rule, const string& match)
{
    if (match.empty()) {
        return false;
    }
    switch (type) {
        case MatchRule::FULL_WORD_TYPE:
            return rule.empty() || match.compare(rule) == 0;
        case MatchRule::PREFIX_WORD_TYPE:
            return rule.empty() || match.find(rule) == 0;
        case MatchRule::REGEX_WORD_TYPE: {
                smatch result;
                const regex pattern(rule);
                return rule.empty() || regex_search(match, result, pattern);
            }
        default:
            HIVIEW_LOGE("invalid rule type %{public}d.", type);
            return false;
    }
}

static bool MatchRules(const SysEventRuleGroupOhos& rules, const string& domain, const string& eventName)
{
    for (auto& rule : rules) {
        if (IsMatchRule(rule.ruleType, rule.domain, domain) && IsMatchRule(rule.ruleType, rule.eventName, eventName)) {
            HIVIEW_LOGD("rule type is %{public}d, domain is %{public}s, eventName is %{public}s for matched.",
                rule.ruleType, rule.domain.empty() ? "empty" : rule.domain.c_str(),
                rule.eventName.empty() ? "empty" : rule.eventName.c_str());
            return true;
        }
    }
    return false;
}

static u16string ConvertToString16(const string& source)
{
    wstring_convert<codecvt_utf8_utf16<char16_t>, char16_t> convertor;
    u16string result = convertor.from_bytes(source);
    return result;
}

void SysEventServiceOhos::OnSysEvent(std::shared_ptr<OHOS::HiviewDFX::SysEvent>& event)
{
    lock_guard<mutex> lock(mutex_);
    for (auto listener = registeredListeners.begin(); listener != registeredListeners.end(); ++listener) {
        SysEventCallbackPtrOhos callback = iface_cast<ISysEventCallback>(listener->first);
        if (callback == nullptr) {
            HIVIEW_LOGE("interface is null, no need to match rules.");
            continue;
        }
        bool isMatched = MatchRules(listener->second.second, event->domain_, event->eventName_);
        HIVIEW_LOGD("pid %{public}d rules match %{public}s.", listener->second.first, isMatched ? "success" : "fail");
        if (isMatched) {
            int eventType = event->what_;
            callback->Handle(ConvertToString16(event->domain_), ConvertToString16(event->eventName_),
                eventType, ConvertToString16(event->jsonExtraInfo_));
        }
    }
}

void SysEventServiceOhos::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        HIVIEW_LOGE("remote is null");
        return;
    }
    auto remoteObject = remote.promote();
    if (remoteObject == nullptr) {
        HIVIEW_LOGD("object in remote is null.");
        return;
    }
    lock_guard<mutex> lock(mutex_);
    CallbackObjectOhos callbackObject = debugModeCallback->AsObject();
    if (callbackObject == remoteObject && isDebugMode) {
        HIVIEW_LOGD("quit debugmode.");
        auto event = std::make_shared<Event>("SysEventSource");
        event->messageType_ = Event::ENGINE_SYSEVENT_DEBUG_MODE;
        event->SetValue("DEBUGMODE", "false");
        gISysEventNotify(event);
        isDebugMode = false;
    }
    auto listener = registeredListeners.find(remoteObject);
    if (listener != registeredListeners.end()) {
        listener->first->RemoveDeathRecipient(deathRecipient_);
        HIVIEW_LOGD("pid %{public}d has died and remove listener.", listener->second.first);
        registeredListeners.erase(listener);
    }
}

OHOS::HiviewDFX::SysEventService* SysEventServiceOhos::GetSysEventService(
    OHOS::HiviewDFX::SysEventService* service)
{
    static OHOS::HiviewDFX::SysEventService* ref = nullptr;
    if (service != nullptr) {
        ref = service;
    }
    return ref;
}

int SysEventServiceOhos::AddListener(const std::vector<SysEventRule>& rules, const sptr<ISysEventCallback>& callback)
{
    if (!HasAccessPermission()) {
        HIVIEW_LOGE("HasAccessPermission check failed");
        return -5; // -5 means permisson denied
    }
    auto service = GetSysEventService();
    if (service == nullptr) {
        HIVIEW_LOGE("subscribe fail, sys event service is null.");
        return -1; // -1 means service null
    }
    if (callback == nullptr) {
        HIVIEW_LOGE("subscribe fail, callback is null.");
        return -2; // -2 means callback null
    }
    CallbackObjectOhos callbackObject = callback->AsObject();
    if (callbackObject == nullptr) {
        HIVIEW_LOGE("subscribe fail, object in callback is null.");
        return -3; // -3 means object in callback null
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t pid = IPCSkeleton::GetCallingPid();
    lock_guard<mutex> lock(mutex_);
    pair<int32_t, SysEventRuleGroupOhos> rulesPair(pid, rules);
    if (registeredListeners.find(callbackObject) != registeredListeners.end()) {
        registeredListeners[callbackObject] = rulesPair;
        HIVIEW_LOGD("uid %{public}d pid %{public}d listener has been added and update rules.", uid, pid);
        return 0; // 0 means add duplicate listener
    }
    if (!callbackObject->AddDeathRecipient(deathRecipient_)) {
        HIVIEW_LOGE("subscribe fail, can not add death recipient.");
        return -4; // -4 means object in callback can not add death recipient
    }
    registeredListeners.insert(make_pair(callbackObject, rulesPair));
    HIVIEW_LOGD("uid %{public}d pid %{public}d listener is added successfully, total is %{public}zu.",
        uid, pid, registeredListeners.size());
    return 1; // 1 means add listener success
}

void SysEventServiceOhos::RemoveListener(const SysEventCallbackPtrOhos& callback)
{
    if (!HasAccessPermission()) {
        HIVIEW_LOGE("HasAccessPermission check failed");
        return;
    }
    auto service = GetSysEventService();
    if (service == nullptr) {
        HIVIEW_LOGE("sys event service is null.");
        return;
    }
    if (callback == nullptr) {
        HIVIEW_LOGE("callback is null.");
        return;
    }
    CallbackObjectOhos callbackObject = callback->AsObject();
    if (callbackObject == nullptr) {
        HIVIEW_LOGE("object in callback is null.");
        return;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t pid = IPCSkeleton::GetCallingPid();
    lock_guard<mutex> lock(mutex_);
    if (registeredListeners.empty()) {
        HIVIEW_LOGD("has no any listeners.");
        return;
    }
    auto registeredListener = registeredListeners.find(callbackObject);
    if (registeredListener != registeredListeners.end()) {
        if (!callbackObject->RemoveDeathRecipient(deathRecipient_)) {
            HIVIEW_LOGE("uid %{public}d pid %{public}d listener can not remove death recipient.", uid, pid);
            return;
        }
        registeredListeners.erase(registeredListener);
        HIVIEW_LOGD("uid %{public}d pid %{public}d has found listener and removes it.", uid, pid);
    } else {
        HIVIEW_LOGD("uid %{public}d pid %{public}d has not found listener.", uid, pid);
    }
}

int64_t SysEventServiceOhos::TransSysEvent(EventStore::ResultSet& result,
    const QuerySysEventCallbackPtrOhos& callback,
    int64_t& lastRecordTime, int32_t& drops)
{
    std::vector<u16string> events;
    std::vector<int64_t> seqs;
    EventStore::ResultSet::RecordIter iter;
    int32_t curTotal = 0;
    int32_t totalRecords = 0;
    while (result.HasNext()) {
        iter = result.Next();
        u16string curJson = ConvertToString16(iter->jsonExtraInfo_);
        int32_t jsonSize = (curJson.size() + 1) * sizeof(u16string);
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
        lastRecordTime = static_cast<int64_t>(iter->happenTime_);
    }

    if (events.size()) {
        callback->OnQuery(events, seqs);
    }

    return totalRecords;
}

bool SysEventServiceOhos::IsValidName(const std::string &value, const unsigned int maxSize) const
{
    if (value.empty() || value.size() > maxSize || !isalpha(value[0])) { // must start with alpha
        return false;
    }

    auto it = std::find_if(value.cbegin() + 1, value.cend(), [](const char& ch) { // 1: midle char verify
        return !isdigit(ch) && !isalpha(ch) && ch != '_';
    });

    return it == value.cend();
}

bool SysEventServiceOhos::CheckDomainEvent(const SysEventQueryRuleGroupOhos& rules) const
{
    for (auto iter = rules.cbegin(); iter < rules.cend(); iter++) {
        if (!IsValidName(iter->domain, MAX_DOMAIN_LENGTH)) {
            HIVIEW_LOGD("CheckDomainEvent domain failed %{public}s", iter->domain.c_str());
            return false;
        }

        auto it = std::find_if(iter->eventList.cbegin(), iter->eventList.cend(), [this](const std::string& name) {
            return !IsValidName(name, MAX_EVENT_NAME_LENGTH);
        });
        if (it != iter->eventList.cend()) {
            HIVIEW_LOGD("CheckDomainEvent name failed %{public}s", it->c_str());
            return false;
        }
    }

    return true;
}

void SysEventServiceOhos::QuerySysEventMiddle(int64_t beginTime, int64_t endTime, int32_t maxEvents,
    const SysEventQueryRuleGroupOhos& rules, EventStore::ResultSet& result)
{
    EventStore::SysEventQuery sysEventQuery = EventStore::SysEventDao::BuildQuery();
    EventStore::Cond timCond;
    timCond.And(EventStore::EventCol::TS, EventStore::Op::GE, beginTime);
    timCond.And(EventStore::EventCol::TS, EventStore::Op::LT, endTime);

    EventStore::Cond domainNameConds;
    for_each(rules.cbegin(), rules.cend(), [&sysEventQuery, &domainNameConds](const SysEventQueryRule& rule) {
        EventStore::Cond domainConds("domain_", EventStore::Op::EQ, rule.domain);
        EventStore::Cond nameConds;
        for_each(rule.eventList.cbegin(), rule.eventList.cend(), [&sysEventQuery, &nameConds](const std::string& name) {
            nameConds.Or("name_", EventStore::Op::EQ, name);
        });
        if (rule.eventList.size()) {
            domainConds.And(nameConds);
        }
        domainNameConds.Or(domainConds);
    });

    if (rules.size()) {
        sysEventQuery = sysEventQuery.Where(timCond).And(domainNameConds).Order(EventStore::EventCol::TS, true);
    } else {
        sysEventQuery = sysEventQuery.Where(timCond).Order(EventStore::EventCol::TS, true);
    }
    result = sysEventQuery.Execute(maxEvents);
}

bool SysEventServiceOhos::QuerySysEvent(int64_t beginTime, int64_t endTime, int32_t maxEvents,
    const SysEventQueryRuleGroupOhos& rules, const QuerySysEventCallbackPtrOhos& callback)
{
    if (!HasAccessPermission()) {
        HIVIEW_LOGE("HasAccessPermission check failed");
        return false;
    }
    int64_t lastQueryUpperLimit = endTime;
    if (endTime < 0) {
        lastQueryUpperLimit = std::numeric_limits<int64_t>::max();
    }
    int64_t lastQueryLowerLimit = beginTime;
    int32_t queryTotal = maxEvents;
    if (maxEvents < 0) {
        queryTotal = std::numeric_limits<int32_t>::max();
    }
    if (!CheckDomainEvent(rules)) {
        HIVIEW_LOGE("CheckDomainEvent check failed");
        return false;
    }
    int32_t remainEvents = queryTotal;
    int32_t transRecord = 0;
    int32_t transTotalEvents = 0;

    while (remainEvents > 0) {
        EventStore::ResultSet result;
        int32_t queryCnt = 0;
        int32_t dropCnt = 0;
        if (remainEvents < MAX_QUERY_EVENTS) {
            queryCnt = remainEvents;
        } else {
            queryCnt = MAX_QUERY_EVENTS;
        }

        QuerySysEventMiddle(lastQueryLowerLimit, lastQueryUpperLimit, queryCnt, rules, result);
        transRecord = TransSysEvent(result, callback, lastQueryLowerLimit, dropCnt);

        lastQueryLowerLimit++;
        transTotalEvents += transRecord;
        remainEvents -= queryCnt;
        if ((transRecord + dropCnt) < queryCnt || remainEvents <= 0 || lastQueryLowerLimit >= lastQueryUpperLimit) {
            callback->OnComplete(0, transTotalEvents);
            break;
        }
    }
    return true;
}

bool SysEventServiceOhos::HasAccessPermission() const
{
    const int callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid == HID_SHELL || callingUid == HID_ROOT) {
        return true;
    }
    HIVIEW_LOGE("hiview service permission denial callingUid=%{public}d", callingUid);
    return true;
}

bool SysEventServiceOhos::SetDebugMode(const SysEventCallbackPtrOhos& callback, bool mode)
{
    if (!HasAccessPermission()) {
        HIVIEW_LOGE("permission denied");
        return false;
    }

    if (mode == isDebugMode) {
        HIVIEW_LOGE("same config, no need set");
        return false;
    }

    auto event = std::make_shared<Event>("SysEventSource");
    event->messageType_ = Event::ENGINE_SYSEVENT_DEBUG_MODE;
    event->SetValue("DEBUGMODE", mode ? "true" : "false");
    gISysEventNotify(event);

    HIVIEW_LOGE("set debug mode %{public}s", mode ? "true" : "false");
    debugModeCallback = callback;
    isDebugMode = mode;
    return true;
}

void CallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    SysEventServiceOhos::GetInstance().OnRemoteDied(remote);
}
}  // namespace HiviewDFX
}  // namespace OHOS
