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

#include <fcntl.h>
#include "common_utils.h"
#include "hisysevent.h"
#include "hiview_platform.h"

#define private public
#include "event_logger.h"
#undef private
#include "event.h"
#include "hiview_platform.h"
#include "sysevent_source.h"
#ifdef WINDOW_MANAGER_ENABLE
#include "focus_change_info.h"
#include "event_focus_listener.h"
#endif
#include "time_util.h"
#include "eventlogger_util_test.h"

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
 * @tc.name: EventLoggerTest_OnEvent_001
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_OnEvent_001, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    std::shared_ptr<Event> event = nullptr;
    EXPECT_FALSE(eventLogger->OnEvent(event));
}

/**
 * @tc.name: EventLoggerTest_OnEvent_002
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_OnEvent_002, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    auto jsonStr = "{\"domain_\":\"RELIABILITY\"}";
    std::shared_ptr<SysEvent> sysEvent1 = std::make_shared<SysEvent>("GESTURE_NAVIGATION_BACK",
        nullptr, jsonStr);
    sysEvent1->eventName_ = "GESTURE_NAVIGATION_BACK";
    sysEvent1->SetEventValue("PID", getpid());
    std::shared_ptr<OHOS::HiviewDFX::Event> event1 = std::static_pointer_cast<Event>(sysEvent1);
    EXPECT_EQ(eventLogger->OnEvent(event1), true);
#ifdef WINDOW_MANAGER_ENABLE
    sptr<Rosen::FocusChangeInfo> focusChangeInfo;
    sptr<EventFocusListener> eventFocusListener_ = EventFocusListener::GetInstance();
    eventFocusListener_->OnFocused(focusChangeInfo);
    eventFocusListener_->OnUnfocused(focusChangeInfo);
    EventFocusListener::registerState_ = EventFocusListener::REGISTERED;
    EXPECT_EQ(eventLogger->OnEvent(event1), true);
#endif
}

/**
 * @tc.name: EventLoggerTest_OnEvent_003
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_OnEvent_003, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    auto jsonStr = "{\"domain_\":\"RELIABILITY\"}";
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("EventLoggerTest_001",
        nullptr, jsonStr);
    EXPECT_EQ(eventLogger->IsHandleAppfreeze(sysEvent), true);
    sysEvent->SetEventValue("PACKAGE_NAME", "EventLoggerTest");
    EXPECT_EQ(eventLogger->IsHandleAppfreeze(sysEvent), true);
    sysEvent->SetEventValue("PID", 0);
    sysEvent->SetEventValue("eventLog_action", "");
    std::shared_ptr<OHOS::HiviewDFX::Event> event = std::static_pointer_cast<Event>(sysEvent);
    EXPECT_EQ(eventLogger->OnEvent(event), true);
    sysEvent->eventName_ = "THREAD_BLOCK_6S";
    event = std::static_pointer_cast<Event>(sysEvent);
    EXPECT_EQ(eventLogger->OnEvent(event), true);
}

/**
 * @tc.name: EventLoggerTest_IsInterestedPipelineEvent_001
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_IsInterestedPipelineEvent_001, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    EXPECT_FALSE(eventLogger->IsInterestedPipelineEvent(nullptr));
    auto jsonStr = "{\"domain_\":\"RELIABILITY\"}";
    std::string testName = "EventLoggerTest_002";
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>(testName,
        nullptr, jsonStr);
    sysEvent->eventId_ = 1000001;
    EXPECT_FALSE(eventLogger->IsInterestedPipelineEvent(sysEvent));
    sysEvent->eventId_ = 1;
    sysEvent->eventName_ = "UninterestedEvent";
    EXPECT_FALSE(eventLogger->IsInterestedPipelineEvent(sysEvent));
    sysEvent->eventName_ = "InterestedEvent";
    eventLogger->eventLoggerConfig_[sysEvent->eventName_] =
        EventLoggerConfig::EventLoggerConfigData();
    EXPECT_TRUE(eventLogger->IsInterestedPipelineEvent(sysEvent));
}

/**
 * @tc.name: EventLoggerTest_CheckProcessRepeatFreeze_001
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_CheckProcessRepeatFreeze_001, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    long pid = getprocpid();
    eventLogger->lastPid_ = pid;
    bool ret = eventLogger->CheckProcessRepeatFreeze("THREAD_BLOCK_6S", pid);
    EXPECT_TRUE(ret);
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
    sysEvent->SetValue("eventLog_action", "pb:1\npb:2");
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
    sysEvent->SetEventValue("PID", 10001); // test value
    eventLogger->StartFfrtDump(sysEvent);
    result = eventLogger->GetFile(sysEvent, logFile, false);
    EXPECT_TRUE(result > 0);
    EXPECT_TRUE(logFile.size() > 0);
    result = eventLogger->GetFile(sysEvent, logFile, true);
    printf("GetFile result=%d\n", result);
    EXPECT_TRUE(logFile.size() > 0);
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
    eventLogger->OnLoad();

    auto jsonStr = "{\"domain_\":\"FORM_MANAGER\"}";
    long pid = getpid();
#ifdef WINDOW_MANAGER_ENABLE
    EventFocusListener::RegisterFocusListener();
    EventFocusListener::registerState_ = EventFocusListener::REGISTERED;
#endif
    uint64_t curentTime = TimeUtil::GetMilliseconds();
    for (int i = 0; i < 5 ; i++) {
        std::shared_ptr<SysEvent> sysEvent1 = std::make_shared<SysEvent>("GESTURE_NAVIGATION_BACK",
            nullptr, jsonStr);
        sysEvent1->SetEventValue("PID", pid);
        sysEvent1->happenTime_ = curentTime;
        std::shared_ptr<OHOS::HiviewDFX::Event> event1 = std::static_pointer_cast<Event>(sysEvent1);
        EXPECT_EQ(eventLogger->OnEvent(event1), true);
        usleep(200 * 1000);
        curentTime += 200;
    }

    std::shared_ptr<SysEvent> sysEvent2 = std::make_shared<SysEvent>("FREQUENT_CLICK_WARNING",
        nullptr, jsonStr);
    sysEvent2->SetEventValue("PID", pid);
    sysEvent2->happenTime_ = TimeUtil::GetMilliseconds();
    std::shared_ptr<OHOS::HiviewDFX::Event> event2 = std::static_pointer_cast<Event>(sysEvent2);
    EXPECT_EQ(eventLogger->OnEvent(event2), true);

    sysEvent2->eventName_ = "FORM_BLOCK_CALLSTACK";
    sysEvent2->domain_ = "FORM_MANAGER";
    eventLogger->WriteCallStack(sysEvent2, 0);
}

/**
 * @tc.name: EventLoggerTest_009
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_009, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    eventLogger->OnLoad();

    auto jsonStr = "{\"domain_\":\"FORM_MANAGER\"}";
    long pid = getpid();
#ifdef WINDOW_MANAGER_ENABLE
    uint64_t curentTime = TimeUtil::GetMilliseconds();
    while (eventLogger->backTimes_.size() < 4) {
        eventLogger->backTimes_.push_back(curentTime);
        curentTime += 100;
    }

    std::shared_ptr<SysEvent> sysEvent2 = std::make_shared<SysEvent>("GESTURE_NAVIGATION_BACK",
        nullptr, jsonStr);
    sysEvent2->SetEventValue("PID", pid);
    sysEvent2->happenTime_ = TimeUtil::GetMilliseconds();
    EventFocusListener::lastChangedTime_ = 0;
    eventLogger->ReportUserPanicWarning(sysEvent2, pid);
#endif

    std::shared_ptr<SysEvent> sysEvent3 = std::make_shared<SysEvent>("FREQUENT_CLICK_WARNING",
        nullptr, jsonStr);
    sysEvent3->SetEventValue("PID", pid);
    sysEvent3->happenTime_ = 4000; // test value
#ifdef WINDOW_MANAGER_ENABLE
    eventLogger->ReportUserPanicWarning(sysEvent3, pid);
    sysEvent3->happenTime_ = 2500; // test value
    eventLogger->ReportUserPanicWarning(sysEvent3, pid);
#endif
    EXPECT_TRUE(true);
}

/**
 * @tc.name: EventLoggerTest_010
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_010, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    auto jsonStr = "{\"domain_\":\"FORM_MANAGER\"}";
    long pid = getpid();
    std::string testName = "FREQUENT_CLICK_WARNING";
    std::shared_ptr<SysEvent> event = std::make_shared<SysEvent>(testName,
        nullptr, jsonStr);
    event->eventName_ = testName;
    event->SetEventValue("PID", pid);
#ifdef WINDOW_MANAGER_ENABLE
    EventFocusListener::lastChangedTime_ = 900; // test value
    event->happenTime_ = 1000; // test value
    eventLogger->ReportUserPanicWarning(event, pid);
    EXPECT_TRUE(eventLogger->backTimes_.empty());
    event->happenTime_ = 4000; // test value
    event->SetEventValue("PROCESS_NAME", "EventLoggerTest_010");
    eventLogger->ReportUserPanicWarning(event, pid);
    EXPECT_TRUE(eventLogger->backTimes_.empty());
#endif
}

/**
 * @tc.name: EventLoggerTest_011
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_011, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    auto jsonStr = "{\"domain_\":\"FORM_MANAGER\"}";
    long pid = getpid();
    std::string testName = "EventLoggerTest_011";
    std::shared_ptr<SysEvent> event = std::make_shared<SysEvent>(testName,
        nullptr, jsonStr);
    event->eventName_ = testName;
    event->SetEventValue("PID", pid);
#ifdef WINDOW_MANAGER_ENABLE
    EXPECT_TRUE(eventLogger->backTimes_.empty());
    EventFocusListener::lastChangedTime_ = 0; // test value
    event->happenTime_ = 3000; // test value
    eventLogger->ReportUserPanicWarning(event, pid);
    EXPECT_EQ(eventLogger->backTimes_.size(), 1);
    while (eventLogger->backTimes_.size() <= 5) {
        int count = 1000; // test value
        eventLogger->backTimes_.push_back(count++);
    }
    EXPECT_TRUE(eventLogger->backTimes_.size() > 5);
    eventLogger->ReportUserPanicWarning(event, pid);
    EXPECT_TRUE(eventLogger->backTimes_.empty());
#endif
}

/**
 * @tc.name: EventLoggerTest_012
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_012, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    auto jsonStr = "{\"domain_\":\"FORM_MANAGER\"}";
    long pid = getpid();
    std::string testName = "EventLoggerTest_012";
    std::shared_ptr<SysEvent> event = std::make_shared<SysEvent>(testName,
        nullptr, jsonStr);
    event->eventName_ = testName;
    event->SetEventValue("PID", pid);
#ifdef WINDOW_MANAGER_ENABLE
    EXPECT_TRUE(eventLogger->backTimes_.empty());
    EventFocusListener::lastChangedTime_ = 0; // test value
    event->happenTime_ = 5000; // test value
    while (eventLogger->backTimes_.size() < 5) {
        int count = 1000; // test value
        eventLogger->backTimes_.push_back(count++);
    }
    EXPECT_TRUE(eventLogger->backTimes_.size() > 0);
    eventLogger->ReportUserPanicWarning(event, pid);
    EXPECT_EQ(eventLogger->backTimes_.size(), 4);
#endif
}

/**
 * @tc.name: EventLoggerTest_013
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_013, TestSize.Level3)
{
    InitSeLinuxEnabled();
    auto eventLogger = std::make_shared<EventLogger>();
    auto jsonStr = "{\"domain_\":\"FORM_MANAGER\"}";
    long pid = getpid();
    std::string testName = "EventLoggerTest_013";
    std::shared_ptr<SysEvent> event = std::make_shared<SysEvent>(testName,
        nullptr, jsonStr);
    eventLogger->WriteCallStack(event, 0);
    event->SetEventValue("PID", pid);
    event->SetEventValue("EVENT_KEY_FORM_BLOCK_CALLSTACK", testName);
    event->SetEventValue("EVENT_KEY_FORM_BLOCK_APPNAME", testName);
    event->eventName_ = "FORM_BLOCK_CALLSTACK";
    event->domain_ = "FORM_MANAGER";
    eventLogger->WriteCallStack(event, 0);
    std::string stackPath = "";
    auto ret = eventLogger->GetAppFreezeFile(stackPath);
    EXPECT_TRUE(ret.empty());
    stackPath = "/data/test/catcherFile";
    auto fd = open(stackPath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create catcherFile. errno: %d\n", errno);
        FAIL();
    }
    eventLogger->GetAppFreezeFile(stackPath);
    close(fd);
    CancelSeLinuxEnabled();
}

/**
 * @tc.name: EventLoggerTest_014
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_014, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    eventLogger->OnLoad();
#ifdef WINDOW_MANAGER_ENABLE
    EventFocusListener::RegisterFocusListener();
    EventFocusListener::registerState_ = EventFocusListener::REGISTERED;
    eventLogger->OnUnload();
    EXPECT_EQ(EventFocusListener::registerState_, EventFocusListener::UNREGISTERED);
#endif
}

/**
 * @tc.name: EventLoggerTest_015
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_015, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    std::string stack = "";
    bool result = eventLogger->IsKernelStack(stack);
    EXPECT_TRUE(!result);
    stack = "Stack backtrace";
    result = eventLogger->IsKernelStack(stack);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: EventLoggerTest_016
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_016, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    std::string stack = "TEST\\nTEST\\nTEST";
    std::string kernelStack = "";
    std::string contentStack = "Test";
    auto jsonStr = "{\"domain_\":\"RELIABILITY\"}";
    std::string testName = "EventLoggerTest_016";
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>(testName,
        nullptr, jsonStr);
    sysEvent->SetEventValue("PROCESS_NAME", testName);
    sysEvent->SetEventValue("APP_RUNNING_UNIQUE_ID", "Test");
    sysEvent->SetEventValue("STACK", stack);
    sysEvent->SetEventValue("MSG", stack);
    sysEvent->eventName_ = "UI_BLOCK_6S";
    sysEvent->SetEventValue("BINDER_INFO", "async\\nEventLoggerTest");
    eventLogger->GetAppFreezeStack(1, sysEvent, stack, "msg", kernelStack);
    EXPECT_TRUE(kernelStack.empty());
    eventLogger->GetNoJsonStack(stack, contentStack, kernelStack, false);
    EXPECT_TRUE(kernelStack.empty());
    stack = "Test:Stack backtrace";
    sysEvent->SetEventValue("STACK", stack);
    eventLogger->GetAppFreezeStack(1, sysEvent, stack, "msg", kernelStack);
    EXPECT_TRUE(!kernelStack.empty());
    eventLogger->GetNoJsonStack(stack, contentStack, kernelStack, false);
    EXPECT_TRUE(!kernelStack.empty());
}

/**
 * @tc.name: EventLoggerTest_017
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_017, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    std::string stack = "";
    auto jsonStr = "{\"domain_\":\"RELIABILITY\"}";
    std::string testName = "EventLoggerTest_017";
    std::shared_ptr<SysEvent> event = std::make_shared<SysEvent>(testName,
        nullptr, jsonStr);
    event->eventName_ = testName;
    int testValue = 1; // test value
    event->SetEventValue("PID", testValue);
    event->happenTime_ = TimeUtil::GetMilliseconds();
    std::string kernelStack = "";
    eventLogger->WriteKernelStackToFile(event, testValue, kernelStack);
    kernelStack = "Test";
    EXPECT_TRUE(!kernelStack.empty());
    eventLogger->WriteKernelStackToFile(event, testValue, kernelStack);
}

/**
 * @tc.name: EventLoggerTest_018
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_018, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    std::string binderInfo = "";
    std::string binderPeerStack = "";
    eventLogger->ParsePeerStack(binderInfo, binderPeerStack);
    EXPECT_TRUE(binderPeerStack.empty());
    binderInfo = "Test";
    eventLogger->ParsePeerStack(binderInfo, binderPeerStack);
    EXPECT_TRUE(binderPeerStack.empty());
    binderInfo = "PeerBinder catcher stacktrace for pid : 111\n Stack "
        "backtrace: Test\n PeerBinder catcher stacktrace for pid : 112\n Test";
    eventLogger->ParsePeerStack(binderInfo, binderPeerStack);
    EXPECT_TRUE(!binderPeerStack.empty());
}

/**
 * @tc.name: EventLoggerTest_019
 * @tc.desc: add testcase coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventLoggerTest, EventLoggerTest_019, TestSize.Level3)
{
    auto eventLogger = std::make_shared<EventLogger>();
    EXPECT_TRUE(!eventLogger->StabilityGetTempFreqInfo().empty());
}
} // namespace HiviewDFX
} // namespace OHOS
