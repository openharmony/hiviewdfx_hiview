 /*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "event_logger_catcher_test.h"

#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>

#include <fcntl.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <string>

#include "securec.h"
#include "common_utils.h"
#include "file_util.h"
#define private public
#ifdef DMESG_CATCHER_ENABLE
#include "dmesg_catcher.h"
#include "time_util.h"
#endif // DMESG_CATCHER_ENABLE
#include "event_log_task.h"
#ifdef STACKTRACE_CATCHER_ENABLE
#include "open_stacktrace_catcher.h"
#endif // STACKTRACE_CATCHER_ENABLE
#include "shell_catcher.h"
#ifdef BINDER_CATCHER_ENABLE
#include "peer_binder_catcher.h"
#include "parameter_ex.h"
#endif // BINDER_CATCHER_ENABLE
#ifdef USAGE_CATCHER_ENABLE
#include "cpu_core_info_catcher.h"
#include "memory_catcher.h"
#endif // USAGE_CATCHER_ENABLE
#include "summary_log_info_catcher.h"
#undef private
#ifdef BINDER_CATCHER_ENABLE
#include "binder_catcher.h"
#endif // BINDER_CATCHER_ENABLE
#ifdef OTHER_CATCHER_ENABLE
#include "ffrt_catcher.h"
#endif // OTHER_CATCHER_ENABLE
#include "event_logger.h"
#include "event_log_catcher.h"
#include "sys_event.h"
#include "hisysevent.h"
#include "eventlogger_util_test.h"
#include "log_catcher_utils.h"
#include "thermal_info_catcher.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace HiviewDFX {
void EventloggerCatcherTest::SetUp()
{
    /**
     * @tc.setup: create an event loop and multiple event handlers
     */
    printf("SetUp.\n");
    InitSeLinuxEnabled();
}

void EventloggerCatcherTest::TearDown()
{
    /**
     * @tc.teardown: destroy the event loop we have created
     */
    CancelSeLinuxEnabled();
    printf("TearDown.\n");
}

/**
 * @tc.name: EventLogCatcher
 * @tc.desc: test EventLogCatcher
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, EventLogCatcherTest_001, TestSize.Level3)
{
    auto eventLogCatcher = std::make_shared<EventLogCatcher>();
    EXPECT_TRUE(eventLogCatcher->GetLogSize() == -1);
    eventLogCatcher->SetLogSize(1);
    EXPECT_TRUE(eventLogCatcher->GetLogSize() == 1);
    auto fd = open("/data/test/catcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create catcherFile. errno: %d\n", errno);
        FAIL();
    }
    int res = eventLogCatcher->Catch(fd, 1);
    EXPECT_TRUE(res == 0);
    close(fd);
}

/**
 * @tc.name: EventlogTask
 * @tc.desc: test EventLogTask
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, EventlogTask_001, TestSize.Level0)
{
    auto fd = open("/data/test/testFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create testFile. errno: %d\n", errno);
        FAIL();
    }
    SysEventCreator sysEventCreator("HIVIEWDFX", "EventlogTask", SysEventCreator::FAULT);
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("EventlogTask", nullptr, sysEventCreator);
    std::unique_ptr<EventLogTask> logTask = std::make_unique<EventLogTask>(fd, 1, sysEvent);
    logTask->AddStopReason(fd, nullptr, "Test");
    auto eventLogCatcher = std::make_shared<EventLogCatcher>();
    logTask->AddStopReason(fd, eventLogCatcher, "Test");
    logTask->AddSeparator(fd, eventLogCatcher);
    bool ret = logTask->ShouldStopLogTask(fd, 1, -1, eventLogCatcher);
    EXPECT_EQ(ret, false);
    ret = logTask->ShouldStopLogTask(fd, 1, 20000, eventLogCatcher);
    EXPECT_EQ(ret, false);
    logTask->status_ = EventLogTask::Status::TASK_TIMEOUT;
    ret = logTask->ShouldStopLogTask(fd, 0, 1, eventLogCatcher);
    EXPECT_EQ(ret, true);
    close(fd);
}

/**
 * @tc.name: EventlogTask
 * @tc.desc: test EventlogTask
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, EventlogTask_002, TestSize.Level3)
{
    auto fd = open("/data/test/testFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create testFile. errno: %d\n", errno);
        FAIL();
    }
    SysEventCreator sysEventCreator("HIVIEWDFX", "EventlogTask", SysEventCreator::FAULT);
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("EventlogTask", nullptr, sysEventCreator);
    std::unique_ptr<EventLogTask> logTask = std::make_unique<EventLogTask>(fd, 1, sysEvent);
    EXPECT_EQ(logTask->GetTaskStatus(), EventLogTask::Status::TASK_RUNNABLE);
    auto ret = logTask->StartCompose();
    EXPECT_EQ(ret, 2);
    EXPECT_EQ(logTask->GetTaskStatus(), EventLogTask::Status::TASK_RUNNING);
    ret = logTask->StartCompose();
    EXPECT_EQ(ret, 1);
    EXPECT_EQ(logTask->GetLogSize(), 0);
    close(fd);
}

/**
 * @tc.name: EventlogTask
 * @tc.desc: test EventlogTask
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, EventlogTask_003, TestSize.Level3)
{
    auto fd = open("/data/test/testFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create testFile. errno: %d\n", errno);
        FAIL();
    }
    SysEventCreator sysEventCreator("HIVIEWDFX", "EventlogTask", SysEventCreator::FAULT);
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("EventlogTask", nullptr, sysEventCreator);
    std::unique_ptr<EventLogTask> logTask = std::make_unique<EventLogTask>(fd, 1, sysEvent);
    logTask->AddLog("cmd:scbCS");
#ifdef STACKTRACE_CATCHER_ENABLE
    logTask->AppStackCapture();
    logTask->SystemStackCapture();
    logTask->RemoteStackCapture();
    logTask->GetGPUProcessStack();
#endif // STACKTRACE_CATCHER_ENABLE

#ifdef BINDER_CATCHER_ENABLE
    logTask->BinderLogCapture();
    EXPECT_EQ(logTask->PeerBinderCapture("Test"), false);
    EXPECT_EQ(logTask->PeerBinderCapture("pb"), false);
    EXPECT_EQ(logTask->PeerBinderCapture("pb:1:a"), true);
#endif // BINDER_CATCHER_ENABLE

#ifdef HILOG_CATCHER_ENABLE
    logTask->HilogCapture();
    logTask->LightHilogCapture();
    logTask->InputHilogCapture();
#endif // HILOG_CATCHER_ENABLE

#ifdef SCB_CATCHER_ENABLE
    logTask->SCBSessionCapture();
    logTask->SCBViewParamCapture();
    logTask->SCBWMSCapture();
    logTask->SCBWMSEVTCapture();
#endif // SCB_CATCHER_ENABLE

#ifdef USAGE_CATCHER_ENABLE
    logTask->DumpAppMapCapture();
    logTask->WMSUsageCapture();
    logTask->AMSUsageCapture();
    logTask->PMSUsageCapture();
    logTask->DPMSUsageCapture();
    logTask->RSUsageCapture();
    logTask->MemoryUsageCapture();
    logTask->CpuUsageCapture();
    logTask->CpuCoreInfoCapture();
#endif // USAGE_CATCHER_ENABLE

#ifdef DMESG_CATCHER_ENABLE
    logTask->DmesgCapture(0, 0);
    logTask->DmesgCapture(0, 1);
    logTask->DmesgCapture(1, 1);
    logTask->DmesgCapture(0, 2);
    logTask->DmesgCapture(1, 2);
#endif // DMESG_CATCHER_ENABLE

#ifdef OTHER_CATCHER_ENABLE
    logTask->Screenshot();
    logTask->FfrtCapture();
    logTask->DMSUsageCapture();
    logTask->MMIUsageCapture();
    logTask->EECStateCapture();
    logTask->GECStateCapture();
    logTask->UIStateCapture();
#endif // OTHER_CATCHER_ENABLE

#ifdef HITRACE_CATCHER_ENABLE
    logTask->HitraceCapture(false);
    sysEvent->eventName_ = "THREAD_BLOCK_6S";
    sysEvent->SetValue("PROCESS_NAME", "EventloggerCatcherTest");
    logTask->HitraceCapture(true);
#endif // HITRACE_CATCHER_ENABLE
    logTask->GetThermalInfoCapture();
    logTask->AddLog("Test");
    logTask->AddLog("cmd:w");
    logTask->status_ = EventLogTask::Status::TASK_RUNNING;
    auto ret = logTask->StartCompose();
    printf("task size: %d\n", static_cast<int>(logTask->tasks_.size()));

    close(fd);
}

#ifdef STACKTRACE_CATCHER_ENABLE
/**
 * @tc.name: EventlogTask
 * @tc.desc: test EventlogTask
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, EventlogTask_004, TestSize.Level3)
{
    auto fd = open("/data/test/testFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create testFile. errno: %d\n", errno);
        FAIL();
    }
    SysEventCreator sysEventCreator("HIVIEWDFX", "EventlogTask", SysEventCreator::FAULT);
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("EventlogTask", nullptr, sysEventCreator);
    std::unique_ptr<EventLogTask> logTask = std::make_unique<EventLogTask>(fd, 1, sysEvent);
    logTask->GetStackByProcessName();
    sysEvent->SetEventValue("PROCESS_NAME", "EventloggerCatcherTest");
    logTask->GetStackByProcessName();
    EXPECT_TRUE(logTask != nullptr);
}
#endif // STACKTRACE_CATCHER_ENABLE

/**
 * @tc.name: EventlogTask
 * @tc.desc: test EventlogTask
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, EventlogTask_005, TestSize.Level3)
{
    auto fd = open("/data/test/vreFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create vreFile. errno: %d\n", errno);
        FAIL();
    }
    SysEventCreator sysEventCreator("HIVIEWDFX", "EventlogTask", SysEventCreator::FAULT);
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("EventlogTask", nullptr, sysEventCreator);
    sysEvent->SetEventValue("PID", getpid());
    sysEvent->SetEventValue("APPNODEID", 2025);
    sysEvent->SetEventValue("APPNODENAME", "test appNodeName");
    sysEvent->SetEventValue("LEASHWINDOWID", 319);
    sysEvent->SetEventValue("LEASHWINDOWNAME", "test leashWindowName");
    sysEvent->SetEventValue("EXT_INFO", "test ext_info");

    sysEvent->domain_ = "AAFWK";
    sysEvent->eventName_ = "APP_INPUT_BLOCK";
    std::unique_ptr<EventLogTask> logTask = std::make_unique<EventLogTask>(fd, 1, sysEvent);
    logTask->SaveRsVulKanError();
    sysEvent->domain_ = "GRAPHIC";
    sysEvent->eventName_ = "RS_VULKAN_ERROR";
    logTask->SaveRsVulKanError();
    close(fd);

    std::string line;
    std::ifstream ifs("/data/test/vreFile", std::ios::in);
    if (ifs.is_open()) {
        while (std::getline(ifs, line)) {
            if (line.find("APPNODEID") != std::string::npos) {
                printf("%s", line.c_str());
                EXPECT_EQ(line, "APPNODEID=2025");
            }
            if (line.find("EXT_INFO") != std::string::npos) {
                printf("%s", line.c_str());
                EXPECT_EQ(line, "EXT_INFO=test ext_info");
            }
        }
    }
    EXPECT_EQ(sysEvent->GetEventValue("PROCESS_NAME"), "EventloggerCatcherTest");
}

#if defined(KERNELSTACK_CATCHER_ENABLE) && defined(DMESG_CATCHER_ENABLE)
/**
 * @tc.name: EventlogTask
 * @tc.desc: test EventlogTask
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, EventlogTask_006, TestSize.Level3)
{
    auto fd = open("/data/test/testFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create testFile. errno: %d\n", errno);
        FAIL();
    }
    SysEventCreator sysEventCreator("HIVIEWDFX", "EventlogTask", SysEventCreator::FAULT);
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("EventlogTask", nullptr, sysEventCreator);
    std::unique_ptr<EventLogTask> logTask = std::make_unique<EventLogTask>(fd, 1, sysEvent);
    logTask->SysrqCapture(true);
    sysEvent->SetEventValue("SPECIFICSTACK_NAME", "foundation");
    logTask->SysrqCapture(true);
    EXPECT_TRUE(logTask != nullptr);
}
#endif

#ifdef BINDER_CATCHER_ENABLE
/**
 * @tc.name: BinderCatcherTest_001
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, BinderCatcherTest_001, TestSize.Level1)
{
    auto binderCatcher = std::make_shared<BinderCatcher>();
    bool ret = binderCatcher->Initialize("test", 1, 2);
    EXPECT_EQ(ret, true);
    auto fd = open("/data/test/catcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create catcherFile. errno: %d\n", errno);
        FAIL();
    }
    int res = binderCatcher->Catch(fd, 1);
    EXPECT_TRUE(res > 0);
    close(fd);
}
#endif // BINDER_CATCHER_ENABLE

#ifdef USAGE_CATCHER_ENABLE
/**
 * @tc.name: MemoryCatcherTest_001
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, MemoryCatcherTest_001, TestSize.Level0)
{
    auto memoryCatcher = std::make_shared<MemoryCatcher>();
    bool ret = memoryCatcher->Initialize("test", 1, 2);
    EXPECT_EQ(ret, true);
    auto fd = open("/data/test/catcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create catcherFile. errno: %d\n", errno);
        FAIL();
    }
    int res = memoryCatcher->Catch(fd, 1);
    EXPECT_TRUE(res > 0);
    res = memoryCatcher->Catch(0, 1);
    EXPECT_EQ(res, 0);
    printf("memoryCatcher result: %d\n", res);
    close(fd);
}

/**
 * @tc.name: MemoryCatcherTest_002
 * @tc.desc: EventloggerCatcherTest
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, MemoryCatcherTest_002, TestSize.Level3)
{
    auto memoryCatcher = std::make_shared<MemoryCatcher>();
    EXPECT_EQ(memoryCatcher->GetStringFromFile("/data/log/test"), "");
}

/**
 * @tc.name: MemoryCatcherTest_003
 * @tc.desc: EventloggerCatcherTest
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, MemoryCatcherTest_003, TestSize.Level3)
{
    auto memoryCatcher = std::make_shared<MemoryCatcher>();
    int ret = memoryCatcher->GetNumFromString("abc");
    EXPECT_EQ(ret, 0);
    ret = memoryCatcher->GetNumFromString("100");
    EXPECT_EQ(ret, 100);
}

/**
 * @tc.name: MemoryCatcherTest_004
 * @tc.desc: EventloggerCatcherTest
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, MemoryCatcherTest_004, TestSize.Level3)
{
    auto memoryCatcher = std::make_shared<MemoryCatcher>();
    std::string data;
    memoryCatcher->CheckString(0, "abc: 100", data, "abc", "/data/log/test");
    EXPECT_TRUE(data.empty());
}

/**
 * @tc.name: MemoryCatcherTest_004
 * @tc.desc: EventloggerCatcherTest
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, MemoryCatcherTest_005, TestSize.Level3)
{
    auto memoryCatcher = std::make_shared<MemoryCatcher>();
    std::string memInfo = memoryCatcher->CollectFreezeSysMemory();
    EXPECT_TRUE(!memInfo.empty());
}

/**
 * @tc.name: MemoryCatcherTest_006
 * @tc.desc: EventloggerCatcherTest
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, MemoryCatcherTest_006, TestSize.Level3)
{
    auto fd = open("/data/test/testFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create testFile. errno: %d\n", errno);
        FAIL();
    }
    SysEventCreator sysEventCreator("HIVIEWDFX", "EventlogTask", SysEventCreator::FAULT);
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("EventlogTask", nullptr, sysEventCreator);
    sysEvent->SetEventValue("FREEZE_MEMORY", "test\\ntest");
    std::unique_ptr<EventLogTask> logTask = std::make_unique<EventLogTask>(fd, 1, sysEvent);
    logTask->MemoryUsageCapture();
    EXPECT_TRUE(logTask != nullptr);
    close(fd);
}

/**
 * @tc.name: MemoryCatcherTest_007
 * @tc.desc: EventloggerCatcherTest
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, MemoryCatcherTest_007, TestSize.Level3)
{
    auto fd = open("/data/test/testFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create testFile. errno: %d\n", errno);
        FAIL();
    }

    SysEventCreator sysEventCreator("HIVIEWDFX", "EventlogTask", SysEventCreator::FAULT);
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("EventlogTask", nullptr, sysEventCreator);
    sysEvent->SetEventValue("FREEZE_MEMORY", "AshmemUsed  3200000 kB\\n"
        "DMAHEAP  3200000 kB");
    std::unique_ptr<EventLogTask> logTask = std::make_unique<EventLogTask>(fd, 1, sysEvent);
    logTask->MemoryUsageCapture();
    EXPECT_TRUE(logTask != nullptr);
    close(fd);
}

/**
 * @tc.name: CpuCoreInfoCatcherTest_001
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, CpuCoreInfoCatcherTest_001, TestSize.Level1)
{
    auto cpuCoreInfoCatcher = std::make_shared<CpuCoreInfoCatcher>();
    bool ret = cpuCoreInfoCatcher->Initialize("test", 1, 2);
    EXPECT_EQ(ret, true);
    auto fd = open("/data/test/catcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create catcherFile. errno: %d\n", errno);
        FAIL();
    }
    auto eventLogCatcher = std::make_shared<EventLogCatcher>();
    int originSize = eventLogCatcher->GetFdSize(fd);
    printf("Get testFile originSize: %d\n", originSize);

    int res = cpuCoreInfoCatcher->Catch(fd, 1);
    EXPECT_TRUE(res > 0);
    int currentSize = eventLogCatcher->GetFdSize(fd) - originSize;
    printf("Get testFile size: %d\n", currentSize);
    close(fd);
}
#endif // USAGE_CATCHER_ENABLE

#ifdef OTHER_CATCHER_ENABLE
/**
 * @tc.name: FfrtCatcherTest_001
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, FfrtCatcherTest_001, TestSize.Level0)
{
    auto fd = open("/data/test/FfrtCatcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create FfrtCatcherFile. errno: %d\n", errno);
        FAIL();
    }

    auto ffrtCatcher = std::make_shared<FfrtCatcher>();
    int pid = CommonUtils::GetPidByName("foundation");
    if (pid > 0) {
        bool res = ffrtCatcher->Initialize("", pid, 0);
        EXPECT_TRUE(res);

        int jsonFd = 1;
        EXPECT_TRUE(ffrtCatcher->Catch(fd, jsonFd) > 0);
    }
    EXPECT_TRUE(true);
    close(fd);
}

/**
 * @tc.name: FfrtCatcherTest_002
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, FfrtCatcherTest_002, TestSize.Level1)
{
    auto ffrtCatcher = std::make_shared<FfrtCatcher>();
    bool ret = ffrtCatcher->Initialize("test", 1, 2);
    EXPECT_EQ(ret, true);
    auto fd = open("/data/test/catcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create catcherFile. errno: %d\n", errno);
        FAIL();
    }
    int res = ffrtCatcher->Catch(fd, 1);
    EXPECT_TRUE(res > 0);
    res = ffrtCatcher->Catch(0, 1);
    EXPECT_EQ(res, 0);
    printf("ffrtCatcher result: %d\n", res);
    close(fd);
}
#endif // OTHER_CATCHER_ENABLE

#ifdef DMESG_CATCHER_ENABLE
/**
 * @tc.name: DmesgCatcherTest_001
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, DmesgCatcherTest_001, TestSize.Level0)
{
    auto dmesgCatcher = std::make_shared<DmesgCatcher>();
    auto jsonStr = "{\"domain_\":\"KERNEL_VENDOR\"}";
    std::shared_ptr<SysEvent> event = std::make_shared<SysEvent>("DmesgCatcherTest_001",
        nullptr, jsonStr);
    event->eventId_ = 0;
    event->domain_ = "KERNEL_VENDOR";
    event->eventName_ = "HUNGTASK";
    event->SetEventValue("PID", 0);
    EXPECT_TRUE(dmesgCatcher->Init(event));
}

/**
 * @tc.name: DmesgCatcherTest_002
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, DmesgCatcherTest_002, TestSize.Level1)
{
    auto dmesgCatcher = std::make_shared<DmesgCatcher>();
    auto fd = open("/data/test/dmesgCatcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create dmesgCatcherFile. errno: %d\n", errno);
        FAIL();
    }

    dmesgCatcher->Initialize("", 0, 0);
    int jsonFd = 1;
    EXPECT_TRUE(dmesgCatcher->Catch(fd, jsonFd) > 0);

    dmesgCatcher->Initialize("", 0, 1);
    EXPECT_TRUE(dmesgCatcher->Catch(fd, jsonFd) > 0);

    dmesgCatcher->Initialize("", 1, 1);
    printf("dmesgCatcher result: %d\n", dmesgCatcher->Catch(fd, jsonFd));

    dmesgCatcher->Initialize("", 0, 2);
    printf("dmesgCatcher result: %d\n", dmesgCatcher->Catch(fd, jsonFd));

    dmesgCatcher->Initialize("", 1, 2);
    printf("dmesgCatcher result: %d\n", dmesgCatcher->Catch(fd, jsonFd));

    close(fd);
}

/**
 * @tc.name: DmesgCatcherTest_003
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, DmesgCatcherTest_003, TestSize.Level1)
{
    auto jsonStr = "{\"domain_\":\"KERNEL_VENDOR\"}";
    std::shared_ptr<SysEvent> event = std::make_shared<SysEvent>("DmesgCatcherTest_003",
        nullptr, jsonStr);
    event->SetEventValue("SYSRQ_TIME", "20250124");
    auto dmesgCatcher = std::make_shared<DmesgCatcher>();
    dmesgCatcher->Init(event);

    bool ret = dmesgCatcher->DumpDmesgLog(-1);
    EXPECT_EQ(ret, false);    
    ret = dmesgCatcher->WriteSysrqTrigger();
    EXPECT_EQ(ret, true);
    
    auto fd = open("/data/test/dmesgCatcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create dmesgCatcherFile. errno: %d\n", errno);
        FAIL();
    }
    dmesgCatcher->Initialize("", true, 1);
    ret = dmesgCatcher->DumpDmesgLog(fd);
    close(fd);
    EXPECT_EQ(ret, true);

    auto fd1 = open("/data/test/dmesgCatcherFile2", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd1 < 0) {
        printf("Fail to create dmesgCatcherFile2. errno: %d\n", errno);
        FAIL();
    }
    dmesgCatcher->Initialize("", true, 2);
    ret = dmesgCatcher->DumpDmesgLog(fd1);
    close(fd1);
    EXPECT_EQ(ret, true);
}

#ifdef KERNELSTACK_CATCHER_ENABLE
/**
 * @tc.name: DmesgCatcherTest_004
 * @tc.desc: add test
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, DmesgCatcherTest_004, TestSize.Level1)
{
    int pid = getpid();
    auto dmesgCatcher = std::make_shared<DmesgCatcher>();
    int ret = dmesgCatcher->DumpKernelStacktrace(-1, pid);
    EXPECT_EQ(ret, -1);
    auto fd = open("/data/test/logCatcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create logCatcherFile. errno: %d\n", errno);
        FAIL();
    }
    ret = dmesgCatcher->DumpKernelStacktrace(fd, pid);
    EXPECT_EQ(ret, 0);
    close(fd);
}

/**
 * @tc.name: DmesgCatcherTest_005
 * @tc.desc: add test
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, DmesgCatcherTest_005, TestSize.Level1)
{
    std::vector<pid_t> tids;
    auto dmesgCatcher = std::make_shared<DmesgCatcher>();
    dmesgCatcher->GetTidsByPid(-1, tids);
    EXPECT_TRUE(tids.size() == 0);
    dmesgCatcher->GetTidsByPid(getpid(), tids);
    EXPECT_TRUE(tids.size() > 0);
}
#endif // KERNELSTACK_CATCHER_ENABLE

#endif // DMESG_CATCHER_ENABLE

#ifdef STACKTRACE_CATCHER_ENABLE
/**
 * @tc.name: OpenStacktraceCatcherTest_001
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, OpenStacktraceCatcherTest_001, TestSize.Level0)
{
    auto fd = open("/data/test/catcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create catcherFile. errno: %d\n", errno);
        FAIL();
    }

    auto openStackCatcher = std::make_shared<OpenStacktraceCatcher>();
    ASSERT_EQ(openStackCatcher->Initialize("", 0, 0), false);

    int jsonFd = 1;
    bool ret = openStackCatcher->Catch(fd, jsonFd);
    EXPECT_TRUE(ret == 0);

    EXPECT_EQ(openStackCatcher->Initialize("test", 0, 0), false);
    ret = openStackCatcher->Catch(fd, jsonFd);
    EXPECT_TRUE(ret == 0);

    EXPECT_EQ(openStackCatcher->Initialize("", 1, 0), true);
    EXPECT_EQ(openStackCatcher->Initialize("test", 1, 0), true);
    ret = openStackCatcher->Catch(fd, jsonFd);
    EXPECT_TRUE(ret > 0);
    close(fd);
}

/**
 * @tc.name: OpenStacktraceCatcherTest_002
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, OpenStacktraceCatcherTest_002, TestSize.Level1)
{
    auto fd = open("/data/test/catcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create catcherFile. errno: %d\n", errno);
        FAIL();
    }
    
    auto openStackCatcher = std::make_shared<OpenStacktraceCatcher>();
    bool ret = openStackCatcher->Catch(fd, 1);
    EXPECT_TRUE(ret == 0);
    EXPECT_EQ(openStackCatcher->ForkAndDumpStackTrace(fd), 0);
    close(fd);
}
#endif // STACKTRACE_CATCHER_ENABLE

#ifdef BINDER_CATCHER_ENABLE
/**
 * @tc.name: PeerBinderCatcherTest_001
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, PeerBinderCatcherTest_001, TestSize.Level0)
{
    auto fd = open("/data/test/catcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create catcherFile. errno: %d\n", errno);
        FAIL();
    }
    auto peerBinderCatcher = std::make_shared<PeerBinderCatcher>();
    peerBinderCatcher->Initialize("PeerBinderCatcherTest", 0, 0);
    int jsonFd = 1;
    int res = peerBinderCatcher->Catch(fd, jsonFd);
    EXPECT_TRUE(res < 0);

    peerBinderCatcher->Initialize("a", 0, 0);
    peerBinderCatcher->Initialize("a", 1, 0);
    res = peerBinderCatcher->Catch(fd, jsonFd);
    EXPECT_TRUE(res < 0);

    peerBinderCatcher->Initialize("a", 1, 1);
    res = peerBinderCatcher->Catch(fd, jsonFd);
    if (Parameter::IsOversea()) {
        EXPECT_TRUE(res == 0);
    } else {
        EXPECT_TRUE(res > 0);
    }

    int pid = CommonUtils::GetPidByName("foundation");
#ifdef HAS_HIPERF
    std::set<int> pids;
    pids.insert(pid);
    peerBinderCatcher->DoExecHiperf("peerBinderCatcher", pids, pid, "r");
#endif
    peerBinderCatcher->Initialize("", 0, pid);
    peerBinderCatcher->Initialize("foundation", 0, pid);
    peerBinderCatcher->Initialize("foundation", 1, pid);
    peerBinderCatcher->CatcherFfrtStack(fd, pid);
    peerBinderCatcher->CatcherStacktrace(fd, pid);
    close(fd);
}

/**
 * @tc.name: PeerBinderCatcherTest_002
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, PeerBinderCatcherTest_002, TestSize.Level1)
{
    auto fd = open("/data/test/catcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create catcherFile. errno: %d\n", errno);
        FAIL();
    }
    auto peerBinderCatcher = std::make_shared<PeerBinderCatcher>();
    peerBinderCatcher->Initialize("a", 1, 0);
    auto jsonStr = "{\"domain_\":\"KERNEL_VENDOR\"}";
    std::shared_ptr<SysEvent> event = std::make_shared<SysEvent>("PeerBinderCatcherTest_002",
        nullptr, jsonStr);
    event->eventId_ = 0;
    event->domain_ = "KERNEL_VENDOR";
    event->eventName_ = "HUNGTASK";
    event->SetEventValue("PID", 0);
    std::string filePath = "/data/test/catcherFile";
    std::set<int> catchedPids;
    catchedPids.insert(0);
    catchedPids.insert(1);
    int pid = CommonUtils::GetPidByName("foundation");
    catchedPids.insert(pid);
    peerBinderCatcher->Init(event, filePath, catchedPids);
    peerBinderCatcher->Initialize("foundation", 1, pid);
    int res = peerBinderCatcher->Catch(fd, 1);
    if (Parameter::IsOversea()) {
        EXPECT_EQ(res, 0);
    } else {
        EXPECT_GT(res, 0);
    }
    close(fd);
}

/**
 * @tc.name: PeerBinderCatcherTest_003
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, PeerBinderCatcherTest_003, TestSize.Level1)
{
    std::map<int, std::list<OHOS::HiviewDFX::PeerBinderCatcher::BinderInfo>> manager;
    auto peerBinderCatcher = std::make_shared<PeerBinderCatcher>();
    std::set<int> pids;
    int eventPid = 1;
    int eventTid = 3;
    PeerBinderCatcher::ParseBinderParam params = {eventPid, eventTid};

    PeerBinderCatcher::BinderInfo info = {2, 0, 4, 0, 1};
    manager[info.clientPid].push_back(info);
    peerBinderCatcher->ParseBinderCallChain(manager, pids, eventPid, params, true);
    EXPECT_TRUE(pids.empty());

    PeerBinderCatcher::BinderInfo info1 = {1, 3, 5, 7, 3};
    manager[info1.clientPid].push_back(info1);
    PeerBinderCatcher::BinderInfo info2 = {5, 7, 9, 13, 3};
    manager[info2.clientPid].push_back(info2);
    PeerBinderCatcher::BinderInfo info3 = {5, 6, 10, 120, 3};
    manager[info3.clientPid].push_back(info3);
    PeerBinderCatcher::BinderInfo info4 = {9, 10, 11, 120, 3};
    manager[info4.clientPid].push_back(info4);
    PeerBinderCatcher::BinderInfo info5 = {9, 13, 19, 12666, 3};
    manager[info5.clientPid].push_back(info5);
    peerBinderCatcher->ParseBinderCallChain(manager, pids, eventPid, params, true);
    EXPECT_EQ(pids.size(), 5);
    EXPECT_EQ(peerBinderCatcher->terminalBinder_.pid, info5.serverPid);
    EXPECT_EQ(peerBinderCatcher->terminalBinder_.tid, info5.serverTid);
#ifdef HAS_HIPERF
    pids.insert(3);
    pids.insert(4);
    int processId = getpid();
    std::string perfCmd = "r";
    peerBinderCatcher->DoExecHiperf("peerBinderCatcher", pids, processId, perfCmd);
    peerBinderCatcher->DumpHiperf(pids, processId, perfCmd);
#endif
}

/**
 * @tc.name: PeerBinderCatcherTest_004
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, PeerBinderCatcherTest_004, TestSize.Level1)
{
    auto fd = open("/data/test/catcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create catcherFile. errno: %d\n", errno);
        FAIL();
    }
    auto peerBinderCatcher = std::make_shared<PeerBinderCatcher>();
    std::set<int> asyncPids;
    std::set<int> pids = peerBinderCatcher->GetBinderPeerPids(fd, 1, asyncPids);
    EXPECT_TRUE(pids.empty());
    close(fd);
}

/**
 * @tc.name: PeerBinderCatcherTest_005
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, PeerBinderCatcherTest_005, TestSize.Level1)
{
    auto peerBinderCatcher = std::make_shared<PeerBinderCatcher>();
    std::list<PeerBinderCatcher::OutputBinderInfo> infoList;
    peerBinderCatcher->AddBinderJsonInfo(infoList, -1);
    PeerBinderCatcher::OutputBinderInfo info = {
        .info = "Test",
        .pid = 0
    };
    infoList.push_back(info);
    peerBinderCatcher->AddBinderJsonInfo(infoList, 1);
    PeerBinderCatcher::OutputBinderInfo info1 = {
        .info = "Test",
        .pid = getpid()
    };
    infoList.push_back(info1);
    peerBinderCatcher->AddBinderJsonInfo(infoList, 1);
    std::string str = "/proc/" + std::to_string(getpid()) + "/cmdline";
    printf("%s\n", str.c_str());
    EXPECT_TRUE(!str.empty());
}

/**
 * @tc.name: PeerBinderCatcherTest_006
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, PeerBinderCatcherTest_006, TestSize.Level1)
{
    auto fd = open("/data/test/peerFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create peerFile. errno: %d\n", errno);
        FAIL();
    }
    std::ofstream testFile;
    std::string path = "/data/test/peerFile";
    testFile.open(path);
    testFile << "0:pid\tcontext:binder\t0:request\t3:started\t"
        "16:max\t4:ready\t521092:free_space\n";
    testFile.close();

    auto peerBinderCatcher = std::make_shared<PeerBinderCatcher>();
    std::ifstream fin;
    fin.open(path.c_str());
    if (!fin.is_open()) {
        printf("open binder file failed, %s\n.", path.c_str());
        FAIL();
    }
    auto fd1 = open("/data/test/peerTestFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd1 < 0) {
        printf("Fail to create peerTestFile. errno: %d\n", errno);
        FAIL();
    }
    std::set<int> asyncPids;
    peerBinderCatcher->BinderInfoParser(fin, fd1, 1, asyncPids);
    std::set<int> pids = peerBinderCatcher->GetBinderPeerPids(fd, 1, asyncPids);
    EXPECT_TRUE(pids.empty());
    pids = peerBinderCatcher->GetBinderPeerPids(-1, 1, asyncPids);
    EXPECT_TRUE(pids.empty());
    fin.close();
    close(fd);
    close(fd1);
}

/**
 * @tc.name: PeerBinderCatcherTest_007
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, PeerBinderCatcherTest_007, TestSize.Level1)
{
    auto peerBinderCatcher = std::make_shared<PeerBinderCatcher>();
    bool ret = peerBinderCatcher->IsAncoProc(getpid());
    EXPECT_TRUE(!ret);
}

/**
 * @tc.name: PeerBinderCatcherTest_008
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, PeerBinderCatcherTest_008, TestSize.Level1)
{
    auto peerBinderCatcher = std::make_shared<PeerBinderCatcher>();
    std::string str = "123";
    uint16_t index = 1;
    std::string ret = peerBinderCatcher->StrSplit(str, index);
    EXPECT_EQ(ret, "");
    str = "123:456";
    ret = peerBinderCatcher->StrSplit(str, index);
    EXPECT_EQ(ret, "456");

    bool isBinderMatchup = false;
    std::string line = "    async_space";

    auto fd = open("/data/test/peerBinderTestFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create peerBinderTestFile. errno: %d\n", errno);
        FAIL();
    }
    peerBinderCatcher->SaveBinderLineToFd(fd, line, isBinderMatchup);
    EXPECT_FALSE(isBinderMatchup);

    line = "    free_async_space";
    peerBinderCatcher->SaveBinderLineToFd(fd, line, isBinderMatchup);
    EXPECT_TRUE(isBinderMatchup);

    peerBinderCatcher->SaveBinderLineToFd(fd, line, isBinderMatchup);
    EXPECT_TRUE(isBinderMatchup);

    fsync(fd);
    close(fd);

    std::ifstream testFile("/data/test/peerBinderTestFile");
    if (testFile.is_open()) {
        std::string line;
        while (std::getline(testFile, line)) {
            printf("%s\n", line.c_str());
            EXPECT_TRUE(line.find("async_space") != std::string::npos);
        }
        testFile.close();
    }
}
#endif // BINDER_CATCHER_ENABLE

/**
 * @tc.name: ShellCatcherTest
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, ShellCatcherTest_001, TestSize.Level0)
{
    auto fd = open("/data/test/shellCatcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create shellCatcherFile. errno: %d\n", errno);
        FAIL();
    }

    auto shellCatcher = std::make_shared<ShellCatcher>();
    int pid = CommonUtils::GetPidByName("foundation");

#ifdef USAGE_CATCHER_ENABLE
    bool res = shellCatcher->Initialize("", ShellCatcher::CATCHER_WMS, pid);
    EXPECT_TRUE(res);
#endif // USAGE_CATCHER_ENABLE

    int jsonFd = 1;
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) < 0);

    std::string cmd = "ShellCatcherTest_001";
#ifdef USAGE_CATCHER_ENABLE
    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_WMS, pid);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) > 0);

    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_AMS, pid);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) > 0);

    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_CPU, pid);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) > 0);

    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_PMS, pid);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) > 0);
    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_DPMS, pid);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) > 0);

    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_RS, pid);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) > 0);
#endif // USAGE_CATCHER_ENABLE

#ifdef HILOG_CATCHER_ENABLE
    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_HILOG, 0);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) > 0);

    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_LIGHT_HILOG, 0);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) > 0);
#endif // HILOG_CATCHER_ENABLE

    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_SNAPSHOT, 0);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) > 0);
    close(fd);
}

/**
 * @tc.name: ShellCatcherTest
 * @tc.desc: GET_DISPLAY_SNAPSHOT test
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, ShellCatcherTest_002, TestSize.Level1)
{
    auto fd = open("/data/test/shellCatcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create shellCatcherFile. errno: %d\n", errno);
        FAIL();
    }
    auto shellCatcher = std::make_shared<ShellCatcher>();
    int jsonFd = 1;
    std::string cmd = "ShellCatcherTest_002";
#ifdef HILOG_CATCHER_ENABLE
    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_INPUT_HILOG, 0);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) >= 0);
    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_INPUT_EVENT_HILOG, 0);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) >= 0);
#endif // HILOG_CATCHER_ENABLE

#ifdef OTHER_CATCHER_ENABLE
    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_EEC, 0);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) >= 0);
    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_GEC, 0);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) >= 0);
    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_UI, 0);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) >= 0);
    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_SNAPSHOT, 0);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) > 0);
    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_MMI, 0);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) >= 0);
    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_DMS, 0);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) >= 0);
#endif // OTHER_CATCHER_ENABLE
    auto jsonStr = "{\"domain_\":\"KERNEL_VENDOR\"}";
    std::shared_ptr<SysEvent> event = std::make_shared<SysEvent>("ShellCatcherTest", nullptr, jsonStr);
    event->SetValue("FOCUS_WINDOW", 4); // 4 test value
    shellCatcher->SetEvent(event);
#ifdef SCB_CATCHER_ENABLE
    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_SCBWMSEVT, 0);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) >= 0);
    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_SCBVIEWPARAM, 0);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) >= 0);
    shellCatcher->Initialize(cmd, ShellCatcher::CATCHER_SCBWMS, 0);
    EXPECT_TRUE(shellCatcher->Catch(fd, jsonFd) >= 0);
#endif // SCB_CATCHER_ENABLE
    close(fd);
}

/**
 * @tc.name: ShellCatcherTest
 * @tc.desc: add test
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, ShellCatcherTest_003, TestSize.Level1)
{
    auto shellCatcher = std::make_shared<ShellCatcher>();
    shellCatcher->SetFocusWindowId("ShellCatcherTest_003");
    EXPECT_TRUE(!shellCatcher->focusWindowId_.empty());
}

/**
 * @tc.name: LogCatcherUtilsTest_001
 * @tc.desc: add test
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, LogCatcherUtilsTest_001, TestSize.Level0)
{
    auto fd = open("/data/test/dumpstacktrace_file", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create dumpstacktrace_file. errno: %d\n", errno);
        FAIL();
    }
    int pid = getpid();
    std::string threadStack;
    int ret = LogCatcherUtils::DumpStacktrace(-1, pid, threadStack);
    EXPECT_EQ(ret, -1);
    std::thread thread1([pid]{
        auto fd1 = open("/data/test/dumpstacktrace_file1", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
        if (fd1 < 0) {
            printf("Fail to create dumpstacktrace_file1. errno: %d\n", errno);
            FAIL();
        }
        std::string threadStack1;
        LogCatcherUtils::DumpStacktrace(fd1, pid, threadStack1);
        close(fd1);
    });
    if (thread1.joinable()) {
        thread1.detach();
    }
    ret = LogCatcherUtils::DumpStacktrace(fd, pid, threadStack);
    close(fd);
    EXPECT_TRUE(threadStack.empty());
    EXPECT_EQ(ret, 0);
    ret = LogCatcherUtils::WriteKernelStackToFd(200, "Test 01\n", getprocpid());
    EXPECT_EQ(ret, 0);
    ret = LogCatcherUtils::WriteKernelStackToFd(200, "Test 02\n", getprocpid());
    EXPECT_EQ(ret, 0);
    ret = LogCatcherUtils::WriteKernelStackToFd(2, "Test", -1);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: LogCatcherUtilsTest_002
 * @tc.desc: add test
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, LogCatcherUtilsTest_002, TestSize.Level1)
{
    std::string processStack = "LogCatcherUtilsTest_002";
    std::string stack = "";
    LogCatcherUtils::GetThreadStack(processStack, stack, 0);
    EXPECT_TRUE(stack.empty());
    int tid = gettid();
    processStack = "Tid:1234, Name: TestThread\n#00 pc 0017888c /system/lib/libark_jsruntime.so\n"
        "#01 pc 00025779 /system/lib/platformsdk/libipc_core.z.so";
    stack.clear();
    LogCatcherUtils::GetThreadStack(processStack, stack, tid);
    EXPECT_TRUE(stack.empty());

    constexpr int testTid = 1234;
    stack.clear();
    processStack = "Tid:1234, Name: TestThread\n"
        "#00 pc 0017888c /system/lib/libark_jsruntime.so\n"
        "#01 pc 00025779 /system/lib/platformsdk/libipc_core.z.so";
    LogCatcherUtils::GetThreadStack(processStack, stack, testTid);
    EXPECT_FALSE(stack.empty());

    stack.clear();
    processStack = "Tid:1234, Name: TestThread\n"
        "ThreadInfo:state=SLEEP, utime=1, stime=2, cutime=0, schedstat={254042, 0, 3}\n"
        "#00 pc 0017888c /system/lib/libark_jsruntime.so\n"
        "#01 pc 00025779 /system/lib/platformsdk/libipc_core.z.so";
    LogCatcherUtils::GetThreadStack(processStack, stack, testTid);
    const string result = "#00 pc 0017888c /system/lib/libark_jsruntime.so\n"
        "#01 pc 00025779 /system/lib/platformsdk/libipc_core.z.so\n";
    EXPECT_EQ(stack, result);

    stack.clear();
    processStack = "Tid:1234, Name: TestThread\n"
        "ThreadInfo:state=SLEEP, utime=1, stime=2, cutime=0, schedstat={254042, 0, 3}\n"
        "#00 pc 0017888c /system/lib/libark_jsruntime.so\n"
        "#01 pc 00025779 /system/lib/platformsdk/libipc_core.z.so\n"
        "Tid:1235, Name: TestThread\n"
        "ThreadInfo:state=SLEEP, utime=1, stime=2, cutime=0, schedstat={254042, 0, 3}\n"
        "#00 pc 0028888c /system/lib/libark_jsruntime.so\n"
        "#01 pc 00036881 /system/lib/platformsdk/libipc_core.z.so";
    LogCatcherUtils::GetThreadStack(processStack, stack, testTid);
    EXPECT_EQ(stack, result);
}

/**
 * @tc.name: LogCatcherUtilsTest_003
 * @tc.desc: add test
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, LogCatcherUtilsTest_003, TestSize.Level1)
{
    auto fd = open("/data/test/logCatcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create logCatcherFile. errno: %d\n", errno);
        FAIL();
    }
    int ret = LogCatcherUtils::DumpStackFfrt(fd, "");
    EXPECT_EQ(ret, 0);
    close(fd);
}

/**
 * @tc.name: LogCatcherUtilsTest_004
 * @tc.desc: add test
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, LogCatcherUtilsTest_004, TestSize.Level1)
{
    auto fd = open("/data/test/logCatcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create logCatcherFile. errno: %d\n", errno);
        FAIL();
    }
    std::string serviceName = "ApplicationManagerService";
    std::string cmd = "Test";
    int count = 0;
    LogCatcherUtils::ReadShellToFile(fd, serviceName, cmd, count);
    EXPECT_EQ(count, 0);
    close(fd);
}

#ifdef HITRACE_CATCHER_ENABLE
/**
 * @tc.name: LogCatcherUtilsTest_005
 * @tc.desc: add test
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, LogCatcherUtilsTest_005, TestSize.Level1)
{
    std::map<std::string, std::string> valuePairs;
    LogCatcherUtils::HandleTelemetryMsg(valuePairs);
    std::pair<std::string, std::string> telemetryInfo = LogCatcherUtils::GetTelemetryInfo();

    EXPECT_EQ(telemetryInfo.first, "");
    EXPECT_EQ(telemetryInfo.second, "");

    valuePairs["telemetryId"] = "testId2025";
    LogCatcherUtils::HandleTelemetryMsg(valuePairs);
    telemetryInfo = LogCatcherUtils::GetTelemetryInfo();
    EXPECT_EQ(telemetryInfo.first, "");
    EXPECT_EQ(telemetryInfo.second, "");

    valuePairs["fault"] = "1";
    LogCatcherUtils::HandleTelemetryMsg(valuePairs);
    telemetryInfo = LogCatcherUtils::GetTelemetryInfo();
    EXPECT_EQ(telemetryInfo.first, "");
    EXPECT_EQ(telemetryInfo.second, "");

    valuePairs["fault"] = "32";
    valuePairs["telemetryStatus"] = "on";
    valuePairs["traceAppFilter"] = "testPackageName2025";
    LogCatcherUtils::HandleTelemetryMsg(valuePairs);
    telemetryInfo = LogCatcherUtils::GetTelemetryInfo();
    EXPECT_EQ(telemetryInfo.first, "testId2025");
    EXPECT_EQ(telemetryInfo.second, "testPackageName2025");

    LogCatcherUtils::FreezeFilterTraceOn("testPackageName2025");
    uint64_t faultTime = TimeUtil::GetMilliseconds() / 1000;
    auto dumpResult = LogCatcherUtils::FreezeDumpTrace(faultTime, true, "testPackageName2025");
    EXPECT_TRUE(dumpResult.first.empty());
    EXPECT_TRUE(dumpResult.second.empty());

    valuePairs["telemetryStatus"] = "off";;
    LogCatcherUtils::HandleTelemetryMsg(valuePairs);
    telemetryInfo = LogCatcherUtils::GetTelemetryInfo();
    EXPECT_EQ(telemetryInfo.first, "");
    EXPECT_EQ(telemetryInfo.second, "");
}
#endif

/**
 * @tc.name: ThermalInfoCatcherTest_001
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, ThermalInfoCatcherTest_001, TestSize.Level1)
{
    auto fd = open("/data/test/catcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create catcherFile. errno: %d\n", errno);
        FAIL();
    }
    
    auto thermalInfoCatcher = std::make_shared<ThermalInfoCatcher>();
    bool ret = thermalInfoCatcher->Catch(fd, 1);
    EXPECT_TRUE(ret > 0);
    close(fd);
}

/**
 * @tc.name: SummaryLogInfoCatcherTest_001
 * @tc.desc: add testcase code coverage
 * @tc.type: FUNC
 */
HWTEST_F(EventloggerCatcherTest, SummaryLogInfoCatcherCatcherTest_001, TestSize.Level1)
{
    auto fd = open("/data/test/summaryLogInfoFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create summaryLogInfoFile. errno: %d\n", errno);
        FAIL();
    }
    uint64_t faultTime = TimeUtil::GetMilliseconds() / 1000;
    std::string formatTime = TimeUtil::TimestampFormatToDate(faultTime, "%Y/%m/%d-%H:%M:%S");

    SysEventCreator sysEventCreator("HIVIEWDFX", "EventlogTask", SysEventCreator::FAULT);
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("EventlogTask", nullptr, sysEventCreator);
    sysEvent->SetEventValue("MSG", "Fault time:" + formatTime + "\n");
    std::unique_ptr<EventLogTask> logTask = std::make_unique<EventLogTask>(0, 0, sysEvent);
    logTask->SaveSummaryLogInfo();

    auto summaryLogInfoCatcher = std::make_shared<SummaryLogInfoCatcher>();
    summaryLogInfoCatcher->SetFaultTime(static_cast<int64_t>(logTask->GetFaultTime()));
    bool ret = summaryLogInfoCatcher->Catch(fd, 1);
    close(fd);
    EXPECT_TRUE(!ret);
    EXPECT_EQ(logTask->GetFaultTime(), faultTime);

    EXPECT_EQ(summaryLogInfoCatcher->CharArrayStr(nullptr, 8), "");
    char chars[8] = {'s', 'u', 'm', 'm', 'a', 'r', 'y', '\0'};
    EXPECT_EQ(summaryLogInfoCatcher->CharArrayStr(chars, 8), "summary");

    std::shared_ptr<SysEvent> sysEvent1 = std::make_shared<SysEvent>("EventlogTask", nullptr, sysEventCreator);
    sysEvent1->SetEventValue("MSG", "Fault time:xx:");
    sysEvent1->happenTime_ = TimeUtil::GetMilliseconds();
    logTask->faultTime_ = 0;
    logTask = std::make_unique<EventLogTask>(0, 0, sysEvent1);
    EXPECT_EQ(logTask->GetFaultTime(), sysEvent1->happenTime_ / 1000);

    std::shared_ptr<SysEvent> sysEvent2 = std::make_shared<SysEvent>("EventlogTask", nullptr, sysEventCreator);
    sysEvent2->SetEventValue("MSG", "Fault time:2025/07/14-18:28/59\\n");
    sysEvent2->happenTime_ = TimeUtil::GetMilliseconds();
    logTask->faultTime_ = 0;
    logTask = std::make_unique<EventLogTask>(0, 0, sysEvent2);
    EXPECT_EQ(logTask->GetFaultTime(), sysEvent2->happenTime_ / 1000);
}
} // namespace HiviewDFX
} // namespace OHOS
