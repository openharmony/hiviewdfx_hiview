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

#include "sys_event_service_ohos_test.h"

#include <cstdlib>
#include <semaphore.h>
#include <string>
#include <vector>

#include "ash_mem_utils.h"
#include "event.h"
#include "event_query_wrapper_builder.h"
#include "file_util.h"
#include "hiview_global.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iquery_sys_event_callback.h"
#include "iservice_registry.h"
#include "isys_event_callback.h"
#include "isys_event_service.h"
#include "query_argument.h"
#include "query_sys_event_callback_proxy.h"
#include "query_sys_event_callback_stub.h"
#include "plugin.h"
#include "ret_code.h"
#include "running_status_log_util.h"
#include "string_ex.h"
#include "sys_event.h"
#include "sys_event_callback_default.h"
#include "sys_event_callback_ohos_test.h"
#include "sys_event_rule.h"
#include "sys_event_service.h"
#include "sys_event_service_adapter.h"
#include "sys_event_service_ohos.h"
#include "sys_event_service_proxy.h"
#include "system_ability.h"
#include "string_ex.h"
#include "string_util.h"
#include "sys_event_callback_proxy.h"
#include "sys_event_callback_stub.h"
#include "sys_event_service_proxy.h"
#include "sys_event_service_stub.h"

using namespace std;

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr char ASH_MEM_NAME[] = "TestSharedMemory";
constexpr int32_t ASH_MEM_SIZE = 1024 * 2; // 2K
constexpr int SYS_EVENT_SERVICE_ID = 1203;
constexpr char TEST_LOG_DIR[] = "/data/log/hiview/sys_event_test";
const std::vector<int> EVENT_TYPES = {1, 2, 3, 4}; // FAULT = 1, STATISTIC = 2 SECURITY = 3, BEHAVIOR = 4

sptr<Ashmem> GetAshmem()
{
    auto ashmem = Ashmem::CreateAshmem(ASH_MEM_NAME, ASH_MEM_SIZE);
    if (ashmem == nullptr) {
        return nullptr;
    }
    if (!ashmem->MapReadAndWriteAshmem()) {
        return ashmem;
    }
    return ashmem;
}

class TestQuerySysEventCallbackStub : public QuerySysEventCallbackStub {
public:
    TestQuerySysEventCallbackStub() {}
    virtual ~TestQuerySysEventCallbackStub() {}

    void OnQuery(const std::vector<std::u16string>& sysEvent, const std::vector<int64_t>& seq) {}
    void OnComplete(int32_t reason, int32_t total, int64_t seq) {}

public:
    enum Code {
        DEFAULT = -1,
        ON_QUERY = 0,
        ON_COMPLETE,
    };
};

class TestSysEventCallbackStub : public SysEventCallbackStub {
public:
    TestSysEventCallbackStub() {}
    virtual ~TestSysEventCallbackStub() {}

    void Handle(const std::u16string& domain, const std::u16string& eventName, uint32_t eventType,
        const std::u16string& eventDetail) {}

public:
    enum Code {
        DEFAULT = -1,
        HANDLE = 0,
    };
};

class TestSysEventServiceStub : public SysEventServiceStub {
public:
    TestSysEventServiceStub() {}
    virtual ~TestSysEventServiceStub() {}

    int32_t AddListener(const std::vector<SysEventRule>& rules, const sptr<ISysEventCallback>& callback)
    {
        return 0;
    }

    int32_t RemoveListener(const sptr<ISysEventCallback>& callback)
    {
        return 0;
    }

    int32_t Query(const QueryArgument& queryArgument, const std::vector<SysEventQueryRule>& rules,
        const sptr<IQuerySysEventCallback>& callback)
    {
        return 0;
    }

    int32_t SetDebugMode(const sptr<ISysEventCallback>& callback, bool mode)
    {
        return 0;
    }

public:
    enum Code {
        DEFAULT = -1,
        ADD_SYS_EVENT_LISTENER = 0,
        REMOVE_SYS_EVENT_LISTENER,
        QUERY_SYS_EVENT,
        SET_DEBUG_MODE
    };
};

class HiviewTestContext : public HiviewContext {
public:
    std::string GetHiViewDirectory(DirectoryType type __UNUSED)
    {
        return TEST_LOG_DIR;
    }
};
}

void SysEventServiceOhosTest::SetUpTestCase() {}

void SysEventServiceOhosTest::TearDownTestCase() {}

void SysEventServiceOhosTest::SetUp() {}

void SysEventServiceOhosTest::TearDown()
{
    (void)FileUtil::ForceRemoveDirectory(TEST_LOG_DIR);
}

static SysEventRule GetTestRule(int type, const string &domain, const string &eventName)
{
    SysEventRule rule;
    rule.ruleType = type;
    rule.domain = domain;
    rule.eventName = eventName;
    return rule;
}

static vector<SysEventRule> GetTestRules(int type, const string &domain, const string &eventName)
{
    vector<SysEventRule> rules;
    rules.push_back(GetTestRule(type, domain, eventName));
    return rules;
}

/**
 * @tc.name: CommonTest001
 * @tc.desc: Check service is null condition.
 * @tc.type: FUNC
 * @tc.require: SR000GGSVB
 */
HWTEST_F(SysEventServiceOhosTest, CommonTest001, testing::ext::TestSize.Level3)
{
    sptr<ISysEventCallback> callbackDefault = new SysEventCallbackDefault();
    vector<SysEventRule> rules = GetTestRules(1, "", "");
    auto service = SysEventServiceOhos::GetInstance();
    if (service == nullptr) {
        return;
    }
    auto ret = service->AddListener(rules, callbackDefault);
    printf("add listener result is %d.\n", ret);
    ASSERT_TRUE(ret != 0);
    ret = service->RemoveListener(callbackDefault);
    printf("remove listener result is %d.\n", ret);
    ASSERT_TRUE(ret != 0);
}

/**
 * @tc.name: AddListenerTest001
 * @tc.desc: Check AddListener Function.
 * @tc.type: FUNC
 * @tc.require: SR000GGS49
 */
HWTEST_F(SysEventServiceOhosTest, AddListenerTest001, testing::ext::TestSize.Level3)
{
    sptr<ISysEventCallback> callbackDefault = new SysEventCallbackDefault();
    sptr<ISysEventCallback> callbackTest = new SysEventCallbackOhosTest();
    vector<SysEventRule> rules = GetTestRules(1, "", "");
    SysEventService sysEventService;
    SysEventServiceOhos::GetSysEventService(&sysEventService);
    auto service = SysEventServiceOhos::GetInstance();
    if (service == nullptr) {
        return;
    }
    auto ret = service->AddListener(rules, nullptr);
    ASSERT_TRUE(ret != 0);
    ret = service->AddListener(rules, callbackDefault);
    ASSERT_TRUE(ret != 0);
    ret = service->AddListener(rules, callbackTest);
    ASSERT_TRUE(ret != 0);
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        printf("SystemAbilityManager is nullptr.\n");
        ASSERT_TRUE(false);
    } else {
        sptr<IRemoteObject> stub = sam->CheckSystemAbility(SYS_EVENT_SERVICE_ID);
        if (stub != nullptr) {
            printf("check sys event service success.\n");
            auto proxy = new SysEventServiceProxy(stub);
            auto ret = proxy->AddListener(rules, callbackTest);
            printf("add listener result is %d.\n", ret);
            ASSERT_TRUE(ret == IPC_CALL_SUCCEED);
            if (ret == 0) {
                sleep(1);
                proxy->AddListener(rules, callbackTest);
            } else {
                printf("add listener fail.\n");
                ASSERT_TRUE(false);
            }
        } else {
            printf("check sys event service failed.\n");
            ASSERT_TRUE(false);
        }
    }
}

/**
 * @tc.name: RemoveListenerTest001
 * @tc.desc: Check RemoveListener Function.
 * @tc.type: FUNC
 * @tc.require: SR000GGS49
 */
HWTEST_F(SysEventServiceOhosTest, RemoveListenerTest001, testing::ext::TestSize.Level3)
{
    auto service = SysEventServiceOhos::GetInstance();
    if (service == nullptr) {
        return;
    }
    service->RemoveListener(nullptr);
    sptr<ISysEventCallback> callbackTest = new SysEventCallbackOhosTest();
    vector<SysEventRule> rules = GetTestRules(1, "", "");
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        printf("SystemAbilityManager is nullptr.\n");
        ASSERT_TRUE(false);
    } else {
        sptr<IRemoteObject> stub = sam->CheckSystemAbility(SYS_EVENT_SERVICE_ID);
        if (stub != nullptr) {
            printf("check sys event service success.\n");
            auto proxy = new SysEventServiceProxy(stub);
            auto ret = proxy->AddListener(rules, callbackTest);
            if (ret == IPC_CALL_SUCCEED) {
                sleep(1);
                ret = proxy->RemoveListener(callbackTest);
                printf("remove listener result is %d.\n", ret);
            } else {
                printf("add listener fail.\n");
                ASSERT_TRUE(false);
            }
        } else {
            printf("check sys event service failed.\n");
            ASSERT_TRUE(false);
        }
    }
}

/**
 * @tc.name: OnSysEventTest001
 * @tc.desc: Check OnSysEvent Function.
 * @tc.type: FUNC
 * @tc.require: SR000GGS49
 */
HWTEST_F(SysEventServiceOhosTest, OnSysEventTest001, testing::ext::TestSize.Level3)
{
    sptr<ISysEventCallback> callbackTest = new SysEventCallbackOhosTest();
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        printf("SystemAbilityManager is nullptr.\n");
        ASSERT_TRUE(false);
    } else {
        sptr<IRemoteObject> stub = sam->CheckSystemAbility(SYS_EVENT_SERVICE_ID);
        if (stub != nullptr) {
            printf("check sys event service success.\n");
            auto proxy = new SysEventServiceProxy(stub);
            vector<SysEventRule> rules;
            SysEventRule rule0 = GetTestRule(0, "", "");
            SysEventRule rule1 = GetTestRule(1, "Test", "Test");
            SysEventRule rule2 = GetTestRule(2, "Test", "Test");
            SysEventRule rule3 = GetTestRule(3, "", "[0-9]*");
            rules.push_back(rule0);
            rules.push_back(rule1);
            rules.push_back(rule2);
            rules.push_back(rule3);
            auto ret = proxy->AddListener(rules, callbackTest);
            sleep(5);
            if (ret == IPC_CALL_SUCCEED) {
                sleep(1);
                proxy->RemoveListener(callbackTest);
            } else {
                printf("add listener fail.\n");
                ASSERT_TRUE(false);
            }
        } else {
            printf("check sys event service failed.\n");
            ASSERT_TRUE(false);
        }
    }
}

/**
 * @tc.name: SetDebugModeTest
 * @tc.desc: Check SetDebugMode Function.
 * @tc.type: FUNC
 * @tc.require: SR000GGSVA
 */
HWTEST_F(SysEventServiceOhosTest, SetDebugModeTest, testing::ext::TestSize.Level3)
{
    sptr<ISysEventCallback> callbackTest = new SysEventCallbackOhosTest();
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        printf("SystemAbilityManager is nullptr.\n");
        ASSERT_TRUE(false);
    } else {
        sptr<IRemoteObject> stub = sam->CheckSystemAbility(SYS_EVENT_SERVICE_ID);
        if (stub != nullptr) {
            printf("check sys event service success.\n");
            auto proxy = new SysEventServiceProxy(stub);
            auto result = proxy->SetDebugMode(callbackTest, true);
            printf("SetDebugMode result is %d.\n", result);
            if (result == IPC_CALL_SUCCEED) {
                result = proxy->SetDebugMode(callbackTest, true);
                ASSERT_TRUE(result == ERR_DEBUG_MODE_SET_REPEAT);
                result = proxy->SetDebugMode(callbackTest, false);
                ASSERT_TRUE(result == IPC_CALL_SUCCEED);
            }
            ASSERT_TRUE(true);
        } else {
            printf("check sys event service failed.\n");
            ASSERT_TRUE(false);
        }
    }
}

/**
 * @tc.name: SysEventServiceAdapterTest
 * @tc.desc: test apis of SysEventServiceAdapter
 * @tc.type: FUNC
 * @tc.require: issueI62BDW
 */
HWTEST_F(SysEventServiceOhosTest, SysEventServiceAdapterTest, testing::ext::TestSize.Level3)
{
    OHOS::HiviewDFX::SysEventServiceAdapter::StartService(nullptr, nullptr);
    std::shared_ptr<SysEvent> sysEvent = nullptr;
    OHOS::HiviewDFX::SysEventServiceAdapter::OnSysEvent(sysEvent);
    SysEventCreator sysEventCreator("DEMO", "EVENT_NAME", SysEventCreator::FAULT);
    std::vector<int> values = {1, 2, 3};
    sysEventCreator.SetKeyValue("KEY", values);
    sysEvent = std::make_shared<SysEvent>("test", nullptr, sysEventCreator);
    OHOS::HiviewDFX::SysEventServiceAdapter::OnSysEvent(sysEvent);
    ASSERT_TRUE(true);
    OHOS::HiviewDFX::SysEventServiceAdapter::UpdateEventSeq(0);
    ASSERT_TRUE(true);
    OHOS::HiviewDFX::SysEventServiceAdapter::BindGetTagFunc(nullptr);
    ASSERT_TRUE(true);
    OHOS::HiviewDFX::SysEventServiceAdapter::BindGetTypeFunc(nullptr);
    ASSERT_TRUE(true);
}

/**
 * @tc.name: TestAshMemory
 * @tc.desc: Ashmemory test
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(SysEventServiceOhosTest, TestAshMemory, testing::ext::TestSize.Level1)
{
    MessageParcel msgParcel;
    std::vector<std::u16string> from = {
        Str8ToStr16(std::string("11")),
        Str8ToStr16(std::string("22")),
    };
    auto result = AshMemUtils::WriteBulkData(msgParcel, from);
    ASSERT_TRUE(result != nullptr);
    std::vector<std::u16string> to;
    auto result1 = AshMemUtils::ReadBulkData(msgParcel, to);
    ASSERT_TRUE(result1);
    ASSERT_TRUE(from.size() == to.size());
    ASSERT_TRUE(Str16ToStr8(to[0]) == "11" && Str16ToStr8(to[1]) == "22");
    AshMemUtils::CloseAshmem(nullptr);
    ASSERT_TRUE(true);
    AshMemUtils::CloseAshmem(GetAshmem());
    ASSERT_TRUE(true);
}

/**
 * @tc.name: TestQuerySysEventCallback
 * @tc.desc: QuerySysEventCallbackProxy/Stub test
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(SysEventServiceOhosTest, TestQuerySysEventCallback, testing::ext::TestSize.Level1)
{
    QuerySysEventCallbackStub* querySysEventCallback = new(std::nothrow) TestQuerySysEventCallbackStub();
    MessageParcel data, reply;
    MessageOption option;
    querySysEventCallback->OnRemoteRequest(TestQuerySysEventCallbackStub::Code::DEFAULT, data, reply, option);
    ASSERT_TRUE(true);
    querySysEventCallback->OnRemoteRequest(TestQuerySysEventCallbackStub::Code::ON_QUERY, data, reply, option);
    ASSERT_TRUE(true);
    querySysEventCallback->OnRemoteRequest(TestQuerySysEventCallbackStub::Code::ON_COMPLETE, data, reply, option);
    ASSERT_TRUE(true);
    const sptr<IRemoteObject>& impl(querySysEventCallback);
    QuerySysEventCallbackProxy sysEventCallbackProxy(impl);
    std::vector<std::u16string> sysEvent {};
    std::vector<int64_t> seq {};
    sysEventCallbackProxy.OnQuery(sysEvent, seq);
    ASSERT_TRUE(true);
    sysEvent.emplace_back(Str8ToStr16(std::string("0")));
    seq.emplace_back(1);
    sysEventCallbackProxy.OnQuery(sysEvent, seq);
    ASSERT_TRUE(true);
    sysEventCallbackProxy.OnComplete(0, 0, 0);
    ASSERT_TRUE(true);
}

/**
 * @tc.name: TestSysEventCallback
 * @tc.desc: SysEventCallbackProxy/Stub test
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(SysEventServiceOhosTest, TestSysEventCallback, testing::ext::TestSize.Level1)
{
    SysEventCallbackStub* sysEventCallback = new(std::nothrow) TestSysEventCallbackStub();
    MessageParcel data, reply;
    MessageOption option;
    sysEventCallback->OnRemoteRequest(TestSysEventCallbackStub::Code::DEFAULT, data, reply, option);
    ASSERT_TRUE(true);
    sysEventCallback->OnRemoteRequest(TestSysEventCallbackStub::Code::HANDLE, data, reply, option);
    ASSERT_TRUE(true);
    const sptr<IRemoteObject>& impl(sysEventCallback);
    SysEventCallbackProxy sysEventCallbackProxy(impl);
    sysEventCallbackProxy.Handle(Str8ToStr16(std::string("DOMAIN1")), Str8ToStr16(std::string("EVENT_NAME1")), 0,
        Str8ToStr16(std::string("{}")));
    ASSERT_TRUE(true);
}

/**
 * @tc.name: TestSysEventService
 * @tc.desc: SysEventServiceProxy/Stub test
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(SysEventServiceOhosTest, TestSysEventService, testing::ext::TestSize.Level1)
{
    SysEventServiceStub* sysEventService = new(std::nothrow) TestSysEventServiceStub();
    MessageParcel data, reply;
    MessageOption option;
    sysEventService->OnRemoteRequest(TestSysEventServiceStub::Code::DEFAULT, data, reply, option);
    ASSERT_TRUE(true);
    sysEventService->OnRemoteRequest(TestSysEventServiceStub::Code::ADD_SYS_EVENT_LISTENER, data, reply, option);
    ASSERT_TRUE(true);
    sysEventService->OnRemoteRequest(TestSysEventServiceStub::Code::REMOVE_SYS_EVENT_LISTENER, data, reply,
        option);
    ASSERT_TRUE(true);
    sysEventService->OnRemoteRequest(TestSysEventServiceStub::Code::QUERY_SYS_EVENT, data, reply, option);
    ASSERT_TRUE(true);
    sysEventService->OnRemoteRequest(TestSysEventServiceStub::Code::SET_DEBUG_MODE, data, reply, option);
    ASSERT_TRUE(true);
    const sptr<IRemoteObject>& impl(sysEventService);
    SysEventServiceProxy sysEventServiceProxy(impl);
    OHOS::HiviewDFX::SysEventRule sysEventRule("DOMAIN", "EVENT_NAME", "TAG", OHOS::HiviewDFX::RuleType::WHOLE_WORD);
    std::vector<OHOS::HiviewDFX::SysEventRule> sysRules;
    sysRules.emplace_back(sysEventRule);
    const sptr<SysEventCallbackStub>& listener(new(std::nothrow) TestSysEventCallbackStub);
    auto ret = sysEventServiceProxy.AddListener(sysRules, listener);
    ASSERT_TRUE(ret == IPC_CALL_SUCCEED);
    ret = sysEventServiceProxy.SetDebugMode(listener, true);
    ASSERT_TRUE(ret == IPC_CALL_SUCCEED);
    ret = sysEventServiceProxy.RemoveListener(listener);
    ASSERT_TRUE(ret == IPC_CALL_SUCCEED);
    const sptr<QuerySysEventCallbackStub>& querier(new(std::nothrow) TestQuerySysEventCallbackStub);
    long long defaultTimeStap = -1;
    int queryCount = 10;
    OHOS::HiviewDFX::QueryArgument argument(defaultTimeStap, defaultTimeStap, queryCount);
    std::vector<OHOS::HiviewDFX::SysEventQueryRule> queryRules;
    std::vector<std::string> eventNames { "EVENT_NAME1", "EVENT_NAME2" };
    OHOS::HiviewDFX::SysEventQueryRule queryRule("DOMAIN", eventNames);
    queryRules.emplace_back(queryRule);
    ret = sysEventServiceProxy.Query(argument, queryRules, querier);
    ASSERT_TRUE(ret == IPC_CALL_SUCCEED);
}

/**
 * @tc.name: MarshallingTAndUnmarshallingTest
 * @tc.desc: Unmarshalling test
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(SysEventServiceOhosTest, MarshallingTAndUnmarshallingTest, testing::ext::TestSize.Level1)
{
    long long defaultTimeStap = -1;
    int queryCount = 10;
    OHOS::HiviewDFX::QueryArgument argument(defaultTimeStap, defaultTimeStap, queryCount);
    MessageParcel parcel1;
    auto ret = argument.Marshalling(parcel1);
    ASSERT_TRUE(ret);
    QueryArgument* argsPtr = argument.Unmarshalling(parcel1);
    ASSERT_TRUE(argsPtr != nullptr && argsPtr->maxEvents == 10 && argsPtr->beginTime == -1);
    OHOS::HiviewDFX::SysEventRule rule("DOMAIN1", "EVENT_NAME2", "TAG3", OHOS::HiviewDFX::RuleType::WHOLE_WORD);
    MessageParcel parcel2;
    ret = rule.Marshalling(parcel2);
    ASSERT_TRUE(ret);
    OHOS::HiviewDFX::SysEventRule* rulePtr = rule.Unmarshalling(parcel2);
    ASSERT_TRUE(rulePtr != nullptr && rulePtr->domain == "DOMAIN1" &&
        rulePtr->eventName == "EVENT_NAME2" && rulePtr->tag == "TAG3");

    std::vector<std::string> eventNames { "EVENT_NAME1", "EVENT_NAME2" };
    OHOS::HiviewDFX::SysEventQueryRule eventQueryRule("DOMAIN", eventNames);
    MessageParcel parcel3;
    ret = eventQueryRule.Marshalling(parcel3);
    ASSERT_TRUE(ret);
    OHOS::HiviewDFX::SysEventQueryRule* eventQueryRulePtr = eventQueryRule.Unmarshalling(parcel3);
    ASSERT_TRUE(eventQueryRulePtr != nullptr && eventQueryRulePtr->domain == "DOMAIN" &&
        eventQueryRulePtr->eventList.size() == 2 && eventQueryRulePtr->eventList[0] == "EVENT_NAME1");
}

/**
 * @tc.name: RunningStatusLogUtilTest
 * @tc.desc: Test apis of RunningStatusLogUtil
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(SysEventServiceOhosTest, RunningStatusLogUtilTest, testing::ext::TestSize.Level1)
{
    HiviewTestContext hiviewTestContext;
    HiviewGlobal::CreateInstance(hiviewTestContext);
    std::vector<OHOS::HiviewDFX::SysEventQueryRule> queryRules;
    std::vector<std::string> eventNames { "EVENT_NAME1", "EVENT_NAME2" };
    OHOS::HiviewDFX::SysEventQueryRule queryRule("DOMAIN", eventNames);
    RunningStatusLogUtil::LogTooManyQueryRules(queryRules);
    ASSERT_TRUE(true);
    queryRules.emplace_back(queryRule);
    RunningStatusLogUtil::LogTooManyQueryRules(queryRules);
    ASSERT_TRUE(true);
    vector<SysEventRule> sysEventRules1;
    RunningStatusLogUtil::LogTooManyWatchRules(sysEventRules1);
    ASSERT_TRUE(true);
    vector<SysEventRule> sysEventRules2 = GetTestRules(1, "", "");
    RunningStatusLogUtil::LogTooManyWatchRules(sysEventRules2);
    ASSERT_TRUE(true);
    RunningStatusLogUtil::LogTooManyWatchers(30);
}

/**
 * @tc.name: SysEventServiceOhosIntanceTest
 * @tc.desc: Test apis of SysEventServiceOhos
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(SysEventServiceOhosTest, SysEventServiceOhosIntanceTest, testing::ext::TestSize.Level1)
{
    auto service = SysEventServiceOhos::GetInstance();
    if (service == nullptr) {
        return;
    }
    HiviewTestContext hiviewTestContext;
    HiviewGlobal::CreateInstance(hiviewTestContext);
    const sptr<QuerySysEventCallbackStub>& querier(new(std::nothrow) TestQuerySysEventCallbackStub);
    long long defaultTimeStap = -1;
    OHOS::HiviewDFX::QueryArgument argument1(defaultTimeStap, defaultTimeStap, 10);
    std::vector<OHOS::HiviewDFX::SysEventQueryRule> queryRules;
    std::vector<std::string> eventNames { "" };
    OHOS::HiviewDFX::SysEventQueryRule queryRule("", eventNames);
    queryRules.emplace_back(queryRule);
    auto ret = service->Query(argument1, queryRules, querier);
    ASSERT_TRUE(ret == IPC_CALL_SUCCEED);
    ret = service->Query(argument1, queryRules, querier);
    ASSERT_TRUE(ret == ERR_QUERY_TOO_FREQUENTLY);
    sptr<ISysEventCallback> callbackTest = new SysEventCallbackOhosTest();
    vector<SysEventRule> rules;
    SysEventRule rule = GetTestRule(0, "DOMAIN", "EVENT_NAME");
    rules.push_back(rule);
    ret = service->AddListener(rules, callbackTest);
    ASSERT_TRUE(ret == ERR_ADD_DEATH_RECIPIENT);
    ret = service->RemoveListener(callbackTest);
    ASSERT_TRUE(ret == ERR_LISTENERS_EMPTY);
    std::vector<std::u16string> args;
    auto dumpRet = service->Dump(-1, args);
    ASSERT_TRUE(dumpRet == -1);
    dumpRet = service->Dump(0, args);
    ASSERT_TRUE(dumpRet == 0);
    service->OnRemoteDied(nullptr);
}

/**
 * @tc.name: ConditionParserTest01
 * @tc.desc: Test apis of ConditionParser
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(SysEventServiceOhosTest, ConditionParserTest01, testing::ext::TestSize.Level1)
{
    OHOS::HiviewDFX::ConditionParser parser;
    EventStore::Cond cond;
    std::string condStr = R"~({"version":"V1","condition":{"and":[{"param":"NAME","op":"=",
        "value":"SysEventService"}]}})~";
    auto ret = parser.ParseCondition(condStr, cond);
    ASSERT_TRUE(ret);
    ret = parser.ParseCondition(condStr, cond);
    ASSERT_TRUE(ret);
    std::string condStr1 = R"~({"version":"V1","condition":{"or":[{"param":"NAME","op":"=",
        "value":"SysEventService"},{"param":"NAME","op":"=","value":"SysEventSource"}]}})~";
    ret = parser.ParseCondition(condStr1, cond);
    ASSERT_TRUE(ret);
    ret = parser.ParseCondition(condStr1, cond);
    ASSERT_TRUE(ret);
    std::string condStr2 = R"~({"version":"V1","condition":{"and":[{"param":"NAME","op":"=",
        "value":"SysEventService"},{"param":"uid_","op":"=","value":1201}]}})~";
    ret = parser.ParseCondition(condStr2, cond);
    ASSERT_TRUE(ret);
    ret = parser.ParseCondition(condStr2, cond);
    ASSERT_TRUE(ret);
    std::string condStr3 = R"~({"version":"V1","condition":{"and":[{"param":"type_","op":">","value":0},
        {"param":"uid_","op":"=","value":1201}],"or":[{"param":"NAME","op":"=","value":"SysEventService"},
        {"param":"NAME","op":"=","value":"SysEventSource"}]}})~";
    ret = parser.ParseCondition(condStr3, cond);
    ASSERT_TRUE(ret);
    ret = parser.ParseCondition(condStr3, cond);
    ASSERT_TRUE(ret);
    std::string condStr4 = R"~({"version":"V1","condition":{"and":[{"param1":"type_","op":">","value":0},
        {"param2":"uid_","op":"=","value":1201}],"or":[{"param4":"NAME","op":"=","value":"SysEventService"},
        {"param3":"NAME","op":"=","value":"SysEventSource"}]}})~";
    ret = parser.ParseCondition(condStr4, cond);
    ASSERT_TRUE(!ret);
    ret = parser.ParseCondition(condStr4, cond);
    ASSERT_TRUE(!ret);
    std::string condSt5 = R"~({"version":"V1","condition":{"and":[{"param":"","op":">","value":0},
        {"param":"","op":"=","value":1201}],"or":[{"param":"","op":"=","value":"SysEventService"},
        {"param":"","op":"=","value":"SysEventSource"}]}})~";
    ret = parser.ParseCondition(condSt5, cond);
    ASSERT_TRUE(!ret);
    ret = parser.ParseCondition(condSt5, cond);
    ASSERT_TRUE(!ret);
    std::string condSt6 = R"~({"version":"V1","condition":{"and":[{"param":"type_","op1":">","value":0},
        {"param":"uid_","op2":"=","value":1201}],"or":[{"param":"NAME","op3":"=","value":"SysEventService"},
        {"param":"NAME","op4":"=","value":"SysEventSource"}]}})~";
    ret = parser.ParseCondition(condSt6, cond);
    ASSERT_TRUE(!ret);
    ret = parser.ParseCondition(condSt6, cond);
    ASSERT_TRUE(!ret);
}

/**
 * @tc.name: ConditionParserTest02
 * @tc.desc: Test apis of ConditionParser
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(SysEventServiceOhosTest, ConditionParserTest02, testing::ext::TestSize.Level1)
{
    OHOS::HiviewDFX::ConditionParser parser;
    EventStore::Cond cond;
    std::string condSt7 = R"~({"version":"V1","condition":{"and":[{"param":"type_","op":">","value11":0},
        {"param":"uid_","op":"=","value2":1201}],"or":[{"param":"NAME","op":"=","value3":"SysEventService"},
        {"param":"NAME","op":"=","value3":"SysEventSource"}]}})~";
    auto ret = parser.ParseCondition(condSt7, cond);
    ASSERT_TRUE(!ret);
    ret = parser.ParseCondition(condSt7, cond);
    ASSERT_TRUE(!ret);
    std::string condStr8 = R"~({"version":"V1","condition":{"and":[{"param":"type_","op":">","value":[]},
        {"param":"uid_","op":"=","value":[]}],"or":[{"param":"NAME","op":"=","value":[]},
        {"param":"NAME","op":"=","value":[]}]}})~";
    ret = parser.ParseCondition(condStr8, cond);
    ASSERT_TRUE(!ret);
    ret = parser.ParseCondition(condStr8, cond);
    ASSERT_TRUE(!ret);
    std::string condStr9 = R"~({"version":"V1","condition1":{"and":[{"param":"type_","op":">","value":0},
        {"param":"uid_","op":"=","value":1201}],"or":[{"param":"NAME","op":"=","value":"SysEventService"},
        {"param":"NAME","op":"=","value":"SysEventSource"}]}})~";
    ret = parser.ParseCondition(condStr9, cond);
    ASSERT_TRUE(!ret);
    ret = parser.ParseCondition(condStr9, cond);
    ASSERT_TRUE(!ret);
    std::string condStr10 = R"~({"version":"V1","condition":1})~";
    ret = parser.ParseCondition(condStr10, cond);
    ASSERT_TRUE(!ret);
    ret = parser.ParseCondition(condStr10, cond);
    ASSERT_TRUE(!ret);
    std::string condStr11 = R"~({"version":"V2","condition":{"and":[{"param1":"type_","op":">","value":0},
        {"param2":"uid_","op":"=","value":1201}],"or":[{"param4":"NAME","op":"=","value":"SysEventService"},
        {"param3":"NAME","op":"=","value":"SysEventSource"}]}})~";
    ret = parser.ParseCondition(condStr11, cond);
    ASSERT_TRUE(!ret);
    ret = parser.ParseCondition(condStr11, cond);
    ASSERT_TRUE(!ret);
}

/**
 * @tc.name: QueryWrapperTest01
 * @tc.desc: BUild query wrapper with all event types
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(SysEventServiceOhosTest, QueryWrapperTest01, testing::ext::TestSize.Level1)
{
    HiviewTestContext hiviewTestContext;
    HiviewGlobal::CreateInstance(hiviewTestContext);
    QueryArgument queryArgument1(-1, -1, 10);
    auto queryWrapperBuilder1 = std::make_shared<EventQueryWrapperBuilder>(queryArgument1);
    QueryArgument queryArgument2(-1, -1, 10, 1, 20);
    auto queryWrapperBuilder2 = std::make_shared<EventQueryWrapperBuilder>(queryArgument2);
    for (auto eventType : EVENT_TYPES) {
        queryWrapperBuilder1->Append(eventType);
        queryWrapperBuilder2->Append(eventType);
    }
    auto queryWrapper1 = queryWrapperBuilder1->Build();
    ASSERT_TRUE(queryWrapper1 != nullptr);
    auto queryWrapper2 = queryWrapperBuilder2->Build();
    ASSERT_TRUE(queryWrapper2 != nullptr);

    const sptr<QuerySysEventCallbackStub>& querier(new(std::nothrow) TestQuerySysEventCallbackStub);
    auto queryRetCode = IPC_CALL_SUCCEED;
    sleep(2);
    queryWrapper1->Query(querier, queryRetCode);
    ASSERT_TRUE(queryRetCode == IPC_CALL_SUCCEED);
    sleep(2);
    queryWrapper2->Query(querier, queryRetCode);
    ASSERT_TRUE(queryRetCode == IPC_CALL_SUCCEED);
}

/**
 * @tc.name: QueryWrapperTest02
 * @tc.desc: BUild query wrapper with domain, event name and event type.
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(SysEventServiceOhosTest, QueryWrapperTest02, testing::ext::TestSize.Level1)
{
    HiviewTestContext hiviewTestContext;
    HiviewGlobal::CreateInstance(hiviewTestContext);
    QueryArgument queryArgument1(-1, -1, 10);
    auto queryWrapperBuilder = std::make_shared<EventQueryWrapperBuilder>(queryArgument1);
    queryWrapperBuilder->Append("DOMAIN1", "EVENTNAME1", 1, R"~({"version":"V1","condition":
        {"and":[{"param":"NAME","op":"=","value":"SysEventService"}]}})~");
    queryWrapperBuilder->Append("DOMAIN2", "EVENTNAME2", 3, R"~({"version":"V1","condition":
        {"and":[{"param":"NAME","op":"=","value":"SysEventService"}]}})~");
    auto queryWrapper = queryWrapperBuilder->Build();
    ASSERT_TRUE(queryWrapper != nullptr);
    const sptr<QuerySysEventCallbackStub>& querier(new(std::nothrow) TestQuerySysEventCallbackStub);
    auto queryRetCode = IPC_CALL_SUCCEED;
    sleep(2);
    queryWrapper->Query(querier, queryRetCode);
    ASSERT_TRUE(queryRetCode == IPC_CALL_SUCCEED);
}
} // namespace HiviewDFX
} // namespace OHOS