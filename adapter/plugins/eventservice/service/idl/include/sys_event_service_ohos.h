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

#ifndef OHOS_HIVIEWDFX_SYS_EVENT_SERVICE_OHOS_H
#define OHOS_HIVIEWDFX_SYS_EVENT_SERVICE_OHOS_H

#include <functional>
#include <vector>
#include <unordered_map>

#include "event.h"
#include "iquery_sys_event_callback.h"
#include "isys_event_callback.h"
#include "singleton.h"
#include "sys_event_dao.h"
#include "sys_event_query.h"
#include "sys_event_query_rule.h"
#include "sys_event_rule.h"
#include "sys_event_service_stub.h"
#include "system_ability.h"

using CallbackObjectOhos = OHOS::sptr<OHOS::IRemoteObject>;
using SysEventCallbackPtrOhos = OHOS::sptr<OHOS::HiviewDFX::ISysEventCallback>;
using SysEventRuleGroupOhos = std::vector<OHOS::HiviewDFX::SysEventRule>;
using QuerySysEventCallbackPtrOhos = OHOS::sptr<OHOS::HiviewDFX::IQuerySysEventCallback>;
using SysEventQueryRuleGroupOhos = std::vector<OHOS::HiviewDFX::SysEventQueryRule>;
using DomainNameTagMap = std::map<std::pair<std::string, std::string>, std::string>;
using EventNames = std::vector<std::string>;
using DomainsWithNames = std::unordered_map<std::string, EventNames>;
using QueryArgs = std::map<int, DomainsWithNames>;
using RegisteredListeners = std::map<CallbackObjectOhos, std::pair<int32_t, SysEventRuleGroupOhos>>;

namespace OHOS {
namespace HiviewDFX {
using NotifySysEvent = std::function<void (std::shared_ptr<Event>)>;
using GetTagByDomainNameFunc = std::function<std::string(std::string, std::string)>;
using GetTypeByDomainNameFunc = std::function<int(std::string, std::string)>;
using QueryTimeRange = std::pair<int64_t, int64_t>;

class SysEventServiceBase {
};

class CallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    CallbackDeathRecipient() = default;
    virtual ~CallbackDeathRecipient() = default;
    void OnRemoteDied(const wptr<IRemoteObject> &object) override;
};

class SysEventServiceOhos : public SystemAbility,
                            public SysEventServiceStub,
                            public Singleton<SysEventServiceOhos> {
    DECLARE_SYSTEM_ABILITY(SysEventServiceOhos);
public:
    DISALLOW_COPY_AND_MOVE(SysEventServiceOhos);
    SysEventServiceOhos() : deathRecipient_(new CallbackDeathRecipient()), isDebugMode_(false) {};
    virtual ~SysEventServiceOhos() = default;

    static void StartService(SysEventServiceBase* service,
        const OHOS::HiviewDFX::NotifySysEvent notify);
    static SysEventServiceBase* GetSysEventService(
        OHOS::HiviewDFX::SysEventServiceBase* service = nullptr);
    void OnSysEvent(std::shared_ptr<OHOS::HiviewDFX::SysEvent>& sysEvent);
    int32_t AddListener(const SysEventRuleGroupOhos& rules, const SysEventCallbackPtrOhos& callback) override;
    int32_t RemoveListener(const SysEventCallbackPtrOhos& callback) override;
    int32_t QuerySysEvent(int64_t beginTime, int64_t endTime, int32_t maxEvents,
        const SysEventQueryRuleGroupOhos& rules, const QuerySysEventCallbackPtrOhos& callback) override;
    int32_t SetDebugMode(const SysEventCallbackPtrOhos& callback, bool mode) override;
    void OnRemoteDied(const wptr<IRemoteObject> &remote);
    void BindGetTagFunc(const GetTagByDomainNameFunc& getTagFunc);
    void BindGetTypeFunc(const GetTypeByDomainNameFunc& getTypeFunc);

private:
    bool HasAccessPermission() const;
    void ParseQueryArgs(const SysEventQueryRuleGroupOhos& rules, QueryArgs& queryArgs);
    std::string GetTagByDomainAndName(const std::string& eventDomain, const std::string& eventName);
    int GetTypeByDomainAndName(const std::string& eventDomain, const std::string& eventName);
    uint32_t QuerySysEventMiddle(QueryArgs::const_iterator queryArgIter, const QueryTimeRange& timeRange,
        int32_t maxEvents, bool isFirstPartialQuery, OHOS::HiviewDFX::EventStore::ResultSet& result);
    int64_t TransSysEvent(OHOS::HiviewDFX::EventStore::ResultSet& result,
        const QuerySysEventCallbackPtrOhos& callback, QueryTimeRange& timeRange, int32_t& drops);

private:
    std::mutex mutex_;
    sptr<CallbackDeathRecipient> deathRecipient_;
    RegisteredListeners registeredListeners_;
    bool isDebugMode_;
    SysEventCallbackPtrOhos debugModeCallback_;
    DomainNameTagMap tagCache_;
    GetTagByDomainNameFunc getTagFunc_;
    GetTypeByDomainNameFunc getTypeFunc_;
    static OHOS::HiviewDFX::NotifySysEvent gISysEventNotify_;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_SYS_EVENT_SERVICE_OHOS_H