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
#include "hisysevent.h"
#include "hiview_platform.h"

#define private public
#include "event_logger.h"
#include "event_focus_listener.h"
#undef private
#include "event.h"
#include "hiview_platform.h"
#include "sysevent_source.h"
#include "focus_change_info.h"
#include "time_util.h"
using namespace testing::ext;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace HiviewDFX {
SysEventSource source;
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
    HiviewPlatform platform;
    source.SetHiviewContext(&platform);
    source.OnLoad();
}

void EventLoggerTest::TearDownTestCase()
{
    source.OnUnload();
}

/**
 * @tc.name: EventLoggerTest_001
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_001, TestSize.Level3)
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
    sptr<Rosen::FocusChangeInfo> focusChangeInfo;
    sptr<EventFocusListener> eventFocusListener_ = EventFocusListener::GetInstance();
    eventFocusListener_->OnFocused(focusChangeInfo);
    eventFocusListener_->OnUnfocused(focusChangeInfo);
    std::shared_ptr<SysEvent> sysEvent1 = std::make_shared<SysEvent>("GESTURE_NAVIGATION_BACK",
        nullptr, jsonStr);
    sysEvent1->eventName_ = "GESTURE_NAVIGATION_BACK";
    sysEvent->SetEventValue("PID", getpid());
    std::shared_ptr<OHOS::HiviewDFX::Event> event1 = std::static_pointer_cast<Event>(sysEvent);
    eventLogger->isRegisterFocusListener = true;
    EXPECT_EQ(eventLogger->OnEvent(event1), true);
}

/**
 * @tc.name: EventLoggerTest_002
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_002, TestSize.Level3)
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
    eventLogger->IsInterestedPipelineEvent(sysEvent);
}

/**
 * @tc.name: EventLoggerTest_003
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_003, TestSize.Level3)
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
    sysEvent->eventName_ = "GET_DISPLAY_SNAPSHOT";
    sysEvent->happenTime_ = TimeUtil::GetMilliseconds();
    sysEvent->SetEventValue("UID", getuid());
    sysEvent->SetEventValue("eventLog_action", "pb:1");
    std::shared_ptr<EventLoop> loop = std::make_shared<EventLoop>("eventLoop");
    loop->StartLoop();
    eventLogger->BindWorkLoop(loop);
    eventLogger->threadLoop_ = loop;
    eventLogger->StartLogCollect(sysEvent);
    ret = eventLogger->UpdateDB(sysEvent, "nolog");
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: EventLoggerTest_004
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_004, TestSize.Level3)
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
HWTEST_F(EventLoggerTest, EventLoggerTest_005, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    eventLogger->GetCmdlineContent();
    eventLogger->GetRebootReasonConfig();
    auto event = std::make_shared<Event>("sender", "event");
    event->messageType_ = Event::MessageType::PLUGIN_MAINTENANCE;
    bool ret = eventLogger->CanProcessRebootEvent(*(event.get()));
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: EventLoggerTest_006
 * @tc.desc: Loging aging test
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_006, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    eventLogger->OnLoad();
    HiSysEventWrite(HiSysEvent::Domain::AAFWK, "THREAD_BLOCK_3S", HiSysEvent::EventType::FAULT,
        "MODULE", "foundation", "MSG", "test remove");
    sleep(3);
    HiSysEventWrite(HiSysEvent::Domain::AAFWK, "THREAD_BLOCK_6S", HiSysEvent::EventType::FAULT,
        "MODULE", "foundation", "MSG", "test remove");
    sleep(3);
    HiSysEventWrite(HiSysEvent::Domain::AAFWK, "LIFECYCLE_HALF_TIMEOUT", HiSysEvent::EventType::FAULT,
        "MODULE", "foundation", "MSG", "test remove");
    std::vector<LogFile> logFileList = eventLogger->logStore_->GetLogFiles();
    auto beforeSize = static_cast<long>(logFileList.size());
    printf("Before-- logFileList num: %ld\n", beforeSize);
    auto iter = logFileList.begin();
    while (iter != logFileList.end()) {
        auto beforeIter = iter;
        iter++;
        EXPECT_TRUE(beforeIter < iter);
    }
    auto folderSize = FileUtil::GetFolderSize(EventLogger::LOGGER_EVENT_LOG_PATH);
    uint32_t maxSize = 10240; // test value
    eventLogger->logStore_->SetMaxSize(maxSize);
    eventLogger->logStore_->ClearOldestFilesIfNeeded();
    auto size = FileUtil::GetFolderSize(EventLogger::LOGGER_EVENT_LOG_PATH);
    auto listSize = static_cast<long>(eventLogger->logStore_->GetLogFiles().size());
    printf("After-- logFileList num: %ld\n", listSize);
    if (listSize == beforeSize) {
        EXPECT_TRUE(size == folderSize);
    } else {
        EXPECT_TRUE(size < folderSize);
    }
    eventLogger->OnUnload();
}

/**
 * @tc.name: EventLoggerTest_007
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_007, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    auto jsonStr = "{\"domain_\":\"RELIABILITY\"}";
    std::string testName = "GET_DISPLAY_SNAPSHOT";
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>(testName,
        nullptr, jsonStr);
    sysEvent->SetEventValue("PID", getpid());
    sysEvent->eventName_ = "GET_DISPLAY_SNAPSHOT";
    sysEvent->happenTime_ = TimeUtil::GetMilliseconds();
    sysEvent->eventId_ = 1;
    std::string logFile = "";
    eventLogger->StartFfrtDump(sysEvent);
    int result = eventLogger->GetFile(sysEvent, logFile, true);
    printf("GetFile result=%d\n", result);
    EXPECT_TRUE(logFile.size() > 0);
    result = eventLogger->GetFile(sysEvent, logFile, false);
    EXPECT_TRUE(result > 0);
    EXPECT_TRUE(logFile.size() > 0);
    sysEvent->eventName_ = "TEST";
    eventLogger->StartFfrtDump(sysEvent);
    result = eventLogger->GetFile(sysEvent, logFile, false);
    EXPECT_TRUE(result > 0);
    EXPECT_TRUE(logFile.size() > 0);
    result = eventLogger->GetFile(sysEvent, logFile, true);
    printf("GetFile result=%d\n", result);
    EXPECT_TRUE(logFile.size() > 0);
    int count = 2;
    eventLogger->ReadShellToFile(0, "GET_DISPLAY_SNAPSHOT", "testCmd", count);
    EXPECT_TRUE(count < 2);
    eventLogger->ReadShellToFile(-1, "GET_DISPLAY_SNAPSHOT", "testCmd", count);
    printf("ReadShellToFile count=%d\n", count);
    sysEvent->SetEventValue("FREEZE_MEMORY", "test\\ntest");
    eventLogger->CollectMemInfo(0, sysEvent);
}

/**
 * @tc.name: EventLoggerTest_008
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_008, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    auto jsonStr = "{\"domain_\":\"FORM_MANAGER\"}";
    long pid = getpid();
    eventLogger->RegisterFocusListener();
    eventLogger->isRegisterFocusListener = true;
    std::string testName = "FREQUENT_CLICK_WARNING";
    std::shared_ptr<SysEvent> sysEvent1 = std::make_shared<SysEvent>(testName,
        nullptr, jsonStr);
    sysEvent1->SetEventValue("PID", pid);
    eventLogger->ReportUserPanicWarning(sysEvent1, pid);
    std::shared_ptr<SysEvent> sysEvent2 = std::make_shared<SysEvent>(testName,
        nullptr, jsonStr);
    sysEvent2->SetEventValue("PID", pid);
    sysEvent2->happenTime_ = TimeUtil::GetMilliseconds();
    eventLogger->eventFocusListener_->lastChangedTime_ = TimeUtil::GetMilliseconds();
    eventLogger->ReportUserPanicWarning(sysEvent2, pid);
    sysEvent2->happenTime_ += 3001; // test value
    eventLogger->ReportUserPanicWarning(sysEvent2, pid);
    testName = "TEST";
    std::shared_ptr<SysEvent> sysEvent3 = std::make_shared<SysEvent>(testName,
        nullptr, jsonStr);
    sysEvent3->SetEventValue("PID", pid);
    while (eventLogger->backTimes_.size() < 5) {
        eventLogger->backTimes_.push_back(sysEvent3->happenTime_ + 1);
    }
    eventLogger->ReportUserPanicWarning(sysEvent3, pid);
    sysEvent3->eventName_ = "FORM_BLOCK_CALLSTACK";
    sysEvent3->domain_ = "FORM_MANAGER";
    eventLogger->WriteCallStack(sysEvent3, 0);
}
} // namespace HiviewDFX
} // namespace OHOS
