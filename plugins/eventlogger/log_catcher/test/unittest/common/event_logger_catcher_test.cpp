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
#include "event_logger_catcher_test.h"

#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>

#include <fcntl.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/prctl.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "securec.h"

#include "common_utils.h"
#include "file_util.h"

#include "event_log_task.h"
#include "event_logger.h"
#include "event_log_catcher.h"
#include "shell_catcher.h"
#include "binder_catcher.h"
#include "sys_event.h"

#define private public
#include "peer_binder_catcher.h"
#include "open_stacktrace_catcher.h"
#include "dmesg_catcher.h"
#undef private

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

constexpr mode_t DEFAULT_MODE = 0644;
void EventloggerCatcherTest::SetUp()
{
    /**
     * @tc.setup: create an event loop and multiple event handlers
     */
    printf("SetUp.\n");
    printf("path_ is %s\n", path_.c_str());

    sleep(1);
    isSelinuxEnabled_ = false;
    char buffer[BUF_SIZE_64] = {'\0'};
    FILE* fp = popen("getenforce", "r");
    if (fp != nullptr) {
        fgets(buffer, sizeof(buffer), fp);
        std::string str = buffer;
        printf("buffer is %s\n", str.c_str());
        if (str.find("Enforcing") != str.npos) {
            printf("Enforcing %s\n", str.c_str());
            isSelinuxEnabled_ = true;
        } else {
            printf("This isn't Enforcing %s\n", str.c_str());
        }
        pclose(fp);
    } else {
        printf("fp == nullptr\n");
    }
    system("setenforce 0");

    constexpr mode_t defaultLogDirMode = 0770;
    if (!FileUtil::FileExists(path_)) {
        FileUtil::ForceCreateDirectory(path_);
        FileUtil::ChangeModeDirectory(path_, defaultLogDirMode);
    }
}

void EventloggerCatcherTest::TearDown()
{
    /**
     * @tc.teardown: destroy the event loop we have created
     */
    if (isSelinuxEnabled_) {
        system("setenforce 1");
        isSelinuxEnabled_ = false;
    }

    printf("TearDown.\n");
}

int EventloggerCatcherTest::GetFdSize(int32_t fd)
{
    struct stat fileStat;
    if (fstat(fd, &fileStat) == -1) {
        return 0;
    }
    return fileStat.st_size;
}

std::string EventloggerCatcherTest::GetFormatTime(unsigned long timestamp)
{
    struct tm tm;
    time_t ts;
    /* 20: the length of 'YYYYmmddHHMMSS' */
    int strLen = 20;
    ts = timestamp;
    localtime_r(&ts, &tm);
    char buf[strLen];

    (void)memset_s(buf, strLen, 0, strLen);
    strftime(buf, strLen - 1, "%Y%m%d%H%M%S", &tm);
    return std::string(buf, strlen(buf));
}

int EventloggerCatcherTest::JudgmentsFileSize(int minQuantity, const std::string sender)
{
    constexpr mode_t defaultLogFileMode = 0644;
    std::string logPath = path_ + "/" + logFile_;
    auto fd2 = open(logPath.c_str(), O_RDONLY, defaultLogFileMode);
    if (fd2 < 0) {
        printf("second, Fail to create %s. fd2 == %d\n", logFile_.c_str(), fd2);
        return RETURN_OPEN_FAIL2;
    }
    auto size = GetFdSize(fd2);
    printf("%s, file size %d\n", sender.c_str(), size);
    if (size < minQuantity) {
        printf("error %s, less than size %d\n", sender.c_str(), minQuantity);
        close(fd2);
        return RETURN_LESS_THAN_SIZE;
    }
    return fd2;
}

int EventloggerCatcherTest::StartCreate(const std::string sender, const std::string name, const std::string action,
    int pid, const std::string packageName, int interval, int minQuantity)
{
    auto eventlogger = std::make_unique<EventLogger>();
    std::string jsonStr = R"~({"domain_":"demo","name_":")~" + name + R"~(","pid_":)~" +
        std::to_string(pid) + R"~(,"tid_":6527,"PACKAGE_NAME":")~" + packageName + R"~("})~";
    auto event = std::make_shared<SysEvent>("sender", nullptr, jsonStr);
    std::time_t timeTmp = 0;
    time(&timeTmp);
    event->happenTime_ = timeTmp;
    event->SetValue("eventLog_action", action);
    event->SetValue("eventLog_interval", interval);

    printf("pid is %d, pid_ %d; packageName is %s, PACKAGE_NAME %s\n",
        pid, event->GetPid(), packageName.c_str(), event->GetEventValue("PACKAGE_NAME").c_str());

    std::string idStr = event->eventName_;
    logFile_ = idStr + "-" + GetFormatTime(event->happenTime_) + ".log";

    constexpr mode_t defaultLogFileMode = 0644;
    std::string logPath = path_ + "/" + logFile_;
    printf("logPath is %s\n", logPath.c_str());
    auto fd = open(logPath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, defaultLogFileMode);
    if (fd < 0) {
        printf("Fail to create %s. fd == %d\n", logFile_.c_str(), fd);
        return RETURN_OPEN_FAIL;
    }

    auto logTask = std::make_unique<EventLogTask>(fd, event);
    logTask->AddLog(event->GetValue("eventLog_action"));
    auto ret = logTask->StartCompose();
    if (ret != EventLogTask::TASK_SUCCESS) {
        printf("capture fail %d", ret);
        close(fd);
        return RETURN_TASK_NO_SUCCESS;
    }
    close(fd);

    printf("logTask LogSize is %ld\n", logTask->GetLogSize());
    if (logTask->GetLogSize() < minQuantity) {
        printf("error LogSize less than %d\n", minQuantity);
        return RETURN_TASK_LESS_THAN_SIZE;
    }
    return 0;
}

/**
 * @tc.name: EventloggerCatcherTest001
 * @tc.desc: parse a correct config file and check result
 * @tc.type: FUNC
 * @tc.require: AR000FT62O
 */
HWTEST_F(EventloggerCatcherTest, EventloggerCatcherTest001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create event handler and events
     */
    constexpr int minQuantity = 8000;
    int pid = CommonUtils::GetPidByName("foundation");
    auto ret = StartCreate("EventloggerCatcherTest001", "TEST01_SYS_STACKTRACE",
        "s", pid, "foundation", 0, minQuantity);
    if (ret < 0) {
        printf("EventloggerCatcherTest001 StartCreate is error ret == %d\n", ret);
        FAIL();
    }

    auto fd = JudgmentsFileSize(minQuantity, "EventloggerCatcherTest001");
    if (fd < 0) {
        printf("EventloggerCatcherTest001 JudgmentsFileSize is error ret == %d\n", fd);
        FAIL();
    }

    char readTmp[BUF_SIZE_256];
    ret = -1;
    while (read(fd, readTmp, BUF_SIZE_256)) {
        std::string tmp = readTmp;
        if (tmp.find("system/bin") != tmp.npos) {
            ret = 0;
            break;
        }
    }

    if (ret < 0) {
        printf("not find Key Messages\n");
        close(fd);
        FAIL();
    }
    close(fd);
}

/**
 * @tc.name: EventloggerCatcherTest002
 * @tc.desc: parse a correct config file and check result
 * @tc.type: FUNC
 * @tc.require: AR000FT62O
 */
HWTEST_F(EventloggerCatcherTest, EventloggerCatcherTest002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create event handler and events
     */
    printf("EventloggerCatcherTest002 start\n");

    int pid = -1;
    const int memSize = 1024*3;
    if ((pid = fork()) < 0) {
        printf("Fork error, err:%d", errno);
        FAIL();
    }

    // Creating a process with high usage
    if (pid == 0) {
        prctl(PR_SET_NAME, "EventlogTest02");
        prctl(PR_SET_PDEATHSIG, SIGKILL);
        printf("Fork pid == 0, child process\n");
        int volatile temp[memSize] = {0};
        while (true) {
            int i = 0;
            while (i < memSize) {
                temp[i] = i & 0x234567;
                i++;
            }
        }
    }

    sleep(6);
    constexpr int minQuantity = 500;
    auto ret = StartCreate("EventloggerCatcherTest002", "TEST02_CPU", "cmd:c",
        pid, "EventlogTest02", 0, minQuantity);

    if (ret < 0) {
        printf("EventloggerCatcherTest002 StartCreate is error ret == %d\n", ret);
        FAIL();
    }

    auto fd = JudgmentsFileSize(minQuantity, "EventloggerCatcherTest002");
    if (fd < 0) {
        printf("EventloggerCatcherTest002 JudgmentsFileSize is error ret == %d\n", fd);
        FAIL();
    }

    char readTmp[BUF_SIZE_256];
    ret = -1;
    while (read(fd, readTmp, BUF_SIZE_256)) {
        std::string tmp = readTmp;
        if (tmp.find("[cpuusage]") != tmp.npos) {
            ret = 0;
            break;
        }
    }

    if (ret < 0) {
        printf("not find Key Messages\n");
        close(fd);
        FAIL();
    }
    close(fd);
}

/**
 * @tc.name: EventloggerCatcherTest003
 * @tc.desc: parse a correct config file and check result
 * @tc.type: FUNC
 * @tc.require: AR000FT62O
 */
HWTEST_F(EventloggerCatcherTest, EventloggerCatcherTest003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create event handler and events
     */

    int pid = -1;
    const int memSize = 1024*3;
    if ((pid = fork()) < 0) {
        printf("Fork error, err:%d", errno);
        FAIL();
    }

    // Creating a process with high memory
    if (pid == 0) {
        prctl(PR_SET_NAME, "EventlogTest03");
        prctl(PR_SET_PDEATHSIG, SIGKILL);
        printf("Fork pid == 0, child process\n");
        int volatile temp[memSize] = {0};
        while (true) {
            int i = 0;
            while (i < memSize) {
                temp[i] = i & 0x234567;
                i++;
            }
        }
    }

    sleep(6);
    constexpr int minQuantity = 500;
    auto ret = StartCreate("EventloggerCatcherTest003", "TEST03_MEM", "cmd:m",
        pid, "EventlogTest03", 0, minQuantity);
    if (ret < 0) {
        printf("EventloggerCatcherTest003 is error ret == %d\n", ret);
        FAIL();
    }

    auto fd = JudgmentsFileSize(minQuantity, "EventloggerCatcherTest003");
    if (fd < 0) {
        printf("EventloggerCatcherTest003 JudgmentsFileSize is error ret == %d\n", fd);
        FAIL();
    }

    char readTmp[BUF_SIZE_256];
    ret = -1;
    while (read(fd, readTmp, BUF_SIZE_256)) {
        std::string tmp = readTmp;
        if (tmp.find("[memory]") != tmp.npos) {
            ret = 0;
            break;
        }
    }

    if (ret < 0) {
        printf("not find Key Messages\n");
        close(fd);
        FAIL();
    }
    close(fd);
}

/**
 * @tc.name: EventloggerCatcherTest004
 * @tc.desc: test binder_catcher
 * @tc.type: FUNC
 * @tc.require: AR000I3GC3
 */
HWTEST_F(EventloggerCatcherTest, EventloggerCatcherTest004, TestSize.Level3)
{
    printf("BinderCatcher Start\n");
    auto fd = open("/data/test/binderCatcher", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create binderCatcher. errno: %d\n", errno);
        FAIL();
    }
    auto binderCatcher = std::make_shared<BinderCatcher>();
    binderCatcher->Initialize("", 0, 0);
    EXPECT_TRUE(binderCatcher->Catch(fd) > 0);
    close(fd);
    printf("BinderCatcher End\n");
}

/**
 * @tc.name: EventloggerCatcherTest005
 * @tc.desc: test EventLogCatcher
 * @tc.type: FUNC
 * @tc.require: AR000I3GC3
 */
HWTEST_F(EventloggerCatcherTest, EventloggerCatcherTest005, TestSize.Level3)
{
    printf("EventLogCatcher Start\n");
    auto eventLogCatcher = std::make_shared<EventLogCatcher>();
    EXPECT_TRUE(eventLogCatcher->GetLogSize() == -1);
    eventLogCatcher->SetLogSize(1);
    EXPECT_TRUE(eventLogCatcher->GetLogSize() == 1);
    EXPECT_TRUE(eventLogCatcher->AppendFile(-1, "") == 0);
    auto fd = open("/data/test/eventLogCatcher", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create eventLogCatcher. errno: %d\n", errno);
        FAIL();
    }
    int res = eventLogCatcher->Catch(fd);
    EXPECT_TRUE(res == 0);
    EXPECT_TRUE(eventLogCatcher->AppendFile(fd, "") == 0);
    EXPECT_TRUE(eventLogCatcher->AppendFile(fd, "/data/test/eventLogCatcher") == 0);
    close(fd);
    printf("EventLogCatcher End\n");
}

/**
 * @tc.name: EventloggerCatcherTest006
 * @tc.desc: test open_stacktrace_catcher
 * @tc.type: FUNC
 * @tc.require: AR000I3GC3
 */
HWTEST_F(EventloggerCatcherTest, EventloggerCatcherTest006, TestSize.Level3)
{
    printf("OpenStacktraceCatcher Start\n");
    auto fd = open("/data/test/openStackCatcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create openStackCatcherFile. errno: %d\n", errno);
        FAIL();
    }

    auto openStackCatcher = std::make_shared<OpenStacktraceCatcher>();
    EXPECT_EQ(openStackCatcher->Initialize("", -1, 0), false);
    bool ret = openStackCatcher->Catch(fd);
    EXPECT_TRUE(ret == 0);

    EXPECT_EQ(openStackCatcher->Initialize("foundation", -1, 0), false);
    EXPECT_EQ(openStackCatcher->Initialize("", 1, 0), true);
    EXPECT_EQ(openStackCatcher->Initialize("foundation", 1, 0), true);
    ret = openStackCatcher->Catch(fd);
    EXPECT_TRUE(ret > 0);
    EXPECT_EQ(openStackCatcher->ForkAndDumpStackTrace(fd), 0);
    int pid = CommonUtils::GetPidByName("foundation");
    openStackCatcher->WaitChildPid(pid);
    close(fd);
    printf("EventLogCatcher End\n");
}

/**
 * @tc.name: EventloggerCatcherTest007
 * @tc.desc: test shell_catcher
 * @tc.type: FUNC
 * @tc.require: AR000I3GC3
 */
HWTEST_F(EventloggerCatcherTest, EventloggerCatcherTest007, TestSize.Level3)
{
    printf("ShellCatcher Start\n");
    auto fd = open("/data/test/shellCatcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create shellCatcherFile. errno: %d\n", errno);
        FAIL();
    }

    auto shellCatcher = std::make_shared<ShellCatcher>();
    int pid_ = CommonUtils::GetPidByName("foundation");

    bool res = shellCatcher->Initialize("", ShellCatcher::CATCHER_WMS, pid_);
    EXPECT_TRUE(res);

    shellCatcher->Initialize("hidumper -s WindowManagerService -a -a", ShellCatcher::CATCHER_WMS, pid_);
    EXPECT_TRUE(shellCatcher->Catch(fd) > 0);

    shellCatcher->Initialize("hidumper -s AbilityManagerService -a -a", ShellCatcher::CATCHER_AMS, pid_);
    EXPECT_TRUE(shellCatcher->Catch(fd) > 0);

    shellCatcher->Initialize("hidumper --cpuusage", ShellCatcher::CATCHER_CPU, pid_);
    EXPECT_TRUE(shellCatcher->Catch(fd) > 0);

    shellCatcher->Initialize("hidumper --mem", ShellCatcher::CATCHER_MEM, pid_);
    EXPECT_TRUE(shellCatcher->Catch(fd) > 0);

    shellCatcher->Initialize("hidumper -s PowerManagerService -a -s", ShellCatcher::CATCHER_PMS, pid_);
    EXPECT_TRUE(shellCatcher->Catch(fd) > 0);

    shellCatcher->Initialize("hidumper -s DisplayPowerManagerService", ShellCatcher::CATCHER_DPMS, pid_);
    EXPECT_TRUE(shellCatcher->Catch(fd) > 0);

    shellCatcher->Initialize("hilog -x", ShellCatcher::CATCHER_HILOG, 0);
    EXPECT_TRUE(shellCatcher->Catch(fd) > 0);

    close(fd);
    printf("ShellCatcher End\n");
}

/**
 * @tc.name: EventloggerCatcherTest008
 * @tc.desc: test PeerBinderCatcher
 * @tc.type: FUNC
 * @tc.require: AR000I3GC3
 */
HWTEST_F(EventloggerCatcherTest, EventloggerCatcherTest008, TestSize.Level3)
{
    printf("PeerBinderCatcher Start\n");
    auto fd = open("/data/test/peerBinderCatcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create peerBinderCatcherFile. errno: %d\n", errno);
        FAIL();
    }
    auto peerBinderCatcher = std::make_shared<PeerBinderCatcher>();
    peerBinderCatcher->Initialize("r", -1, 0);
    int res = peerBinderCatcher->Catch(fd);
    EXPECT_TRUE(res == -1);

    peerBinderCatcher->Initialize("a", 1, 0);
    res = peerBinderCatcher->Catch(fd);
    EXPECT_TRUE(res == -1);

    auto jsonStr = "{\"domain_\":\"KERNEL_VENDOR\"}";
    std::shared_ptr<SysEvent> event = std::make_shared<SysEvent>("Eventlogger004", nullptr, jsonStr);
    event->eventId_ = 0;
    event->domain_ = "KERNEL_VENDOR";
    event->eventName_ = "HUNGTASK";
    event->SetEventValue("PID", 0);
    std::string filePath = "/data/test/peerBinderCatcherFile";
    peerBinderCatcher->Init(event, filePath);

    int pid_ = CommonUtils::GetPidByName("foundation");
    peerBinderCatcher->Initialize("", 0, pid_);
    peerBinderCatcher->Initialize("foundation", 0, pid_);
    peerBinderCatcher->Initialize("foundation", 1, pid_);
    peerBinderCatcher->GetBinderPeerPids(fd);
    peerBinderCatcher->CatcherStacktrace(fd, pid_);
#ifdef HAS_HIPERF
    std::set<int> pids;
    pids.insert(pid_);
    peerBinderCatcher->DoExecHiperf("peerBinderCatcher", pids);
#endif
    close(fd);
    printf("PeerBinderCatcher End\n");
}

/**
 * @tc.name: EventloggerCatcherTest009
 * @tc.desc: test DmesgCatcher
 * @tc.type: FUNC
 * @tc.require: AR000I3GC3
 */
HWTEST_F(EventloggerCatcherTest, EventloggerCatcherTest009, TestSize.Level3)
{
    auto dmesgCatcher = std::make_shared<DmesgCatcher>();
    auto jsonStr = "{\"domain_\":\"KERNEL_VENDOR\"}";
    std::shared_ptr<SysEvent> event = std::make_shared<SysEvent>("Eventlogger004", nullptr, jsonStr);
    event->eventId_ = 0;
    event->domain_ = "KERNEL_VENDOR";
    event->eventName_ = "HUNGTASK";
    event->SetEventValue("PID", 0);
    EXPECT_TRUE(dmesgCatcher->Init(event));

    auto fd = open("/data/test/dmesgCatcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create dmesgCatcherFile. errno: %d\n", errno);
        FAIL();
    }
    dmesgCatcher->Initialize("", 0, 0);
    EXPECT_TRUE(dmesgCatcher->Catch(fd) > 0);

    dmesgCatcher->Initialize("", true, 1);
    EXPECT_TRUE(dmesgCatcher->Catch(fd) > 0);

    dmesgCatcher->Initialize("", true, 0);
    EXPECT_TRUE(dmesgCatcher->Catch(fd) > 0);

    dmesgCatcher->Initialize("", false, 0);
    EXPECT_TRUE(dmesgCatcher->Catch(fd) > 0);

    close(fd);
}

/**
 * @tc.name: EventloggerCatcherTest010
 * @tc.desc: test EventlogTask
 * @tc.type: FUNC
 * @tc.require: AR000I3GC3
 */
static HWTEST_F(EventloggerCatcherTest, EventloggerCatcherTest010, TestSize.Level3)
{
    auto fd = open("/data/test/dmesgCatcherFile", O_CREAT | O_WRONLY | O_TRUNC, DEFAULT_MODE);
    if (fd < 0) {
        printf("Fail to create dmesgCatcherFile. errno: %d\n", errno);
        FAIL();
    }
    SysEventCreator sysEventCreator("HIVIEWDFX", "EventlogTask", SysEventCreator::FAULT);
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("EventlogTask", nullptr, sysEventCreator);
    std::unique_ptr<EventLogTask> logTask = std::make_unique<EventLogTask>(fd, sysEvent);
    const std::string cmds[] = {
        "S",
        "b",
        "cmd:w",
        "cmd:a",
        "e",
        "T",
        "cmd:d",
    };
    for (const std::string& cmd : cmds) {
        logTask->AddLog(cmd);
    }
    printf("logTask size %ld\n", logTask->GetLogSize());
    auto ret = logTask->StartCompose();
    if (ret != EventLogTask::TASK_SUCCESS) {
        printf("capture fail %d\n", ret);
    }
    close(fd);
}
