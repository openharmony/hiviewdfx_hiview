/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <atomic>
#include <functional>
#include <vector>
#include <unordered_map>

#include "data_publisher.h"
#include "event.h"
#include "event_query_wrapper_builder.h"
#include "iquery_base_callback.h"
#include "iquery_sys_event_callback.h"
#include "isys_event_callback.h"
#include "query_argument.h"
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
using SysEventQueryRuleGroupOhos = std::vector<OHOS::HiviewDFX::SysEventQueryRule>;
using RegisteredListeners = std::map<CallbackObjectOhos, std::pair<int32_t, SysEventRuleGroupOhos>>;

namespace OHOS {
namespace HiviewDFX {
using NotifySysEvent = std::function<void (std::shared_ptr<Event>)>;
using GetTagByDomainNameFunc = std::function<std::string(const std::string&, const std::string&)>;
using GetTypeByDomainNameFunc = std::function<int(const std::string&, const std::string&)>;

class SysEventServiceBase {
};

class CallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    CallbackDeathRecipient() = default;
    virtual ~CallbackDeathRecipient() = default;
    void OnRemoteDied(const wptr<IRemoteObject> &object) override;
};

class SysEventServiceOhos : public SystemAbility,
                            public SysEventServiceStub {
    DECLARE_SYSTEM_ABILITY(SysEventServiceOhos);
public:
    DISALLOW_COPY_AND_MOVE(SysEventServiceOhos);
    SysEventServiceOhos()
        : deathRecipient_(new CallbackDeathRecipient()), isDebugMode_(false), dataPublisher_(new DataPublisher()){};
    virtual ~SysEventServiceOhos() = default;

    static sptr<SysEventServiceOhos> GetInstance();
    static void StartService(SysEventServiceBase* service,
        const OHOS::HiviewDFX::NotifySysEvent notify);
    static SysEventServiceBase* GetSysEventService(
        OHOS::HiviewDFX::SysEventServiceBase* service = nullptr);
    void OnSysEvent(std::shared_ptr<OHOS::HiviewDFX::SysEvent>& sysEvent);
    int32_t AddListener(const SysEventRuleGroupOhos& rules, const SysEventCallbackPtrOhos& callback) override;
    int32_t RemoveListener(const SysEventCallbackPtrOhos& callback) override;
    int32_t Query(const QueryArgument& queryArgument, const SysEventQueryRuleGroupOhos& rules,
        const OHOS::sptr<OHOS::HiviewDFX::IQuerySysEventCallback>& callback) override;
    int32_t SetDebugMode(const SysEventCallbackPtrOhos& callback, bool mode) override;
    void OnRemoteDied(const wptr<IRemoteObject> &remote);
    void BindGetTagFunc(const GetTagByDomainNameFunc& getTagFunc);
    void BindGetTypeFunc(const GetTypeByDomainNameFunc& getTypeFunc);
    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;
    int64_t AddSubscriber(const SysEventQueryRuleGroupOhos &rules) override;
    int32_t RemoveSubscriber() override;
    int64_t Export(const QueryArgument &queryArgument, const SysEventQueryRuleGroupOhos &rules) override;
    void SetWorkLoop(std::shared_ptr<EventLoop> looper);

private:
    bool HasAccessPermission() const;
    bool BuildEventQuery(std::shared_ptr<EventQueryWrapperBuilder> builder, const SysEventQueryRuleGroupOhos& rules);
    std::string GetTagByDomainAndName(const std::string& eventDomain, const std::string& eventName);
    uint32_t GetTypeByDomainAndName(const std::string& eventDomain, const std::string& eventName);
    void MergeEventList(const std::vector<SysEventQueryRule>& rules, std::vector<std::string>& events) const;

private:
    sptr<CallbackDeathRecipient> deathRecipient_;
    std::mutex listenersMutex_;
    RegisteredListeners registeredListeners_;
    bool isDebugMode_;
    SysEventCallbackPtrOhos debugModeCallback_;
    GetTagByDomainNameFunc getTagFunc_;
    GetTypeByDomainNameFunc getTypeFunc_;
    static OHOS::HiviewDFX::NotifySysEvent gISysEventNotify_;
    std::mutex publisherMutex_;
    std::shared_ptr<DataPublisher> dataPublisher_;

private:
    static sptr<SysEventServiceOhos> instance;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_SYS_EVENT_SERVICE_OHOS_H