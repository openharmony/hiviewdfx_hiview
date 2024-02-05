/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "event_logger_test.h"

#include "common_utils.h"

#define private public
#include "event_logger.h"
#undef private
#include "event.h"
using namespace testing::ext;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace HiviewDFX {
void EventLoggerTest::SetUp()
{
    printf("SetUp.\n");
}

void EventLoggerTest::TearDown()
{
    printf("TearDown.\n");
}

void EventLoggerTest::SetUpTestCase()
{
}

void EventLoggerTest::TearDownTestCase()
{
}

/**
 * @tc.name: EventLoggerTest_001
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
static HWTEST_F(EventLoggerTest, EventLoggerTest_001, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    auto jsonStr = "{\"domain_\":\"RELIABILITY\"}";
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("EventLoggerTest_001",
        nullptr, jsonStr);
    sysEvent->SetEventValue("PACKAGE_NAME", "");
    sysEvent->SetEventValue("MODULE_NAME", "");
    EXPECT_EQ(eventLogger->IsHandleAppfreeze(sysEvent), true);
    sysEvent->SetEventValue("PACKAGE_NAME", "EventLoggerTest");
    EXPECT_EQ(eventLogger->IsHandleAppfreeze(sysEvent), true);
    sysEvent->SetEventValue("PID", 0);
    sysEvent->SetEventValue("eventLog_action", "");
    std::shared_ptr<OHOS::HiviewDFX::Event> event = std::static_pointer_cast<Event>(sysEvent);
    EXPECT_EQ(eventLogger->OnEvent(event), true);
}

/**
 * @tc.name: EventLoggerTest_002
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
static HWTEST_F(EventLoggerTest, EventLoggerTest_002, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    auto jsonStr = "{\"domain_\":\"RELIABILITY\"}";
    std::string testName = "EventLoggerTest_002";
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>(testName,
        nullptr, jsonStr);
    sysEvent->SetEventValue("EVENTNAME", testName);
    sysEvent->SetEventValue("MODULE_NAME", testName);
    sysEvent->SetEventValue("PACKAGE_NAME", testName);
    sysEvent->SetEventValue("PROCESS_NAME", testName);
    sysEvent->SetEventValue("eventLog_action", "pb:1");
    sysEvent->SetEventValue("eventLog_interval", 1);
    sysEvent->SetEventValue("STACK", "TEST\\nTEST\\nTEST");
    sysEvent->SetEventValue("MSG", "TEST\\nTEST\\nTEST");
    EXPECT_EQ(eventLogger->WriteCommonHead(1, sysEvent), true);
    sysEvent->eventName_ = "UI_BLOCK_6S";
    sysEvent->SetEventValue("BINDER_INFO", "async\\nEventLoggerTest");
    EXPECT_EQ(eventLogger->WriteFreezeJsonInfo(1, 1, sysEvent), true);
    sysEvent->SetEventValue("BINDER_INFO", "context");
    EXPECT_EQ(eventLogger->WriteFreezeJsonInfo(1, 1, sysEvent), true);
    std::string binderInfo = "1:1\\n1:1\\n" + std::to_string(getpid()) +
        ":1\\n1:1\\n1:1\\n1:1\\n1:1";
    sysEvent->SetEventValue("BINDER_INFO", binderInfo);
    EXPECT_EQ(eventLogger->WriteFreezeJsonInfo(1, 1, sysEvent), true);
}

/**
 * @tc.name: EventLoggerTest_003
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
static HWTEST_F(EventLoggerTest, EventLoggerTest_003, TestSize.Level3)
{
    auto jsonStr = "{\"domain_\":\"RELIABILITY\"}";
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("EventLoggerTest_003",
        nullptr, jsonStr);
    sysEvent->SetEventValue("eventLog_interval", 1);
    sysEvent->SetEventValue("PID", getpid());
    sysEvent->SetEventValue("NAME", "EventLoggerTest_003");
    auto eventLogger = std::make_shared<EventLogger>();
    eventLogger->eventTagTime_["NAME"] = 100;
    eventLogger->eventTagTime_["EventLoggerTest_003"] = 100;
    bool ret = eventLogger->JudgmentRateLimiting(sysEvent);
    EXPECT_EQ(ret, true);
    ret = eventLogger->UpdateDB(sysEvent, "nolog");
    EXPECT_EQ(ret, true);
    eventLogger->OnLoad();
    eventLogger->OnUnload();
}

/**
 * @tc.name: EventLoggerTest_004
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
static HWTEST_F(EventLoggerTest, EventLoggerTest_004, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    eventLogger->cmdlineContent_ = "reboot_reason = EventLoggerTest "
        "normal_reset_type = EventLoggerTest\\n";
    eventLogger->rebootReasons_.push_back("EventLoggerTest");
    std::string ret = eventLogger->GetRebootReason();
    EXPECT_EQ(ret, "LONG_PRESS");
    eventLogger->ProcessRebootEvent();
    eventLogger->cmdlineContent_ = "reboot_reason";
    ret = eventLogger->GetRebootReason();
    EXPECT_EQ(ret, "");
    eventLogger->ProcessRebootEvent();
    EXPECT_EQ(eventLogger->GetListenerName(), "EventLogger");
}

/**
 * @tc.name: EventLoggerTest_005
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
static HWTEST_F(EventLoggerTest, EventLoggerTest_005, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    eventLogger->GetCmdlineContent();
    eventLogger->GetRebootReasonConfig();
    auto event = std::make_shared<Event>("sender", "event");
    event->messageType_ = Event::MessageType::PLUGIN_MAINTENANCE;
    bool ret = eventLogger->CanProcessRebootEvent(*(event.get()));
    EXPECT_EQ(ret, true);
}
} // namespace HiviewDFX
} // namespace OHOS
