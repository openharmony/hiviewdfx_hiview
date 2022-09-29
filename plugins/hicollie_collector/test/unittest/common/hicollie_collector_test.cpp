/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "hicollie_collector_test.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <unistd.h>

#include "event.h"
#include "file_util.h"
#include "time_util.h"

#include "hiview_platform.h"
#include "sys_event.h"

using namespace testing::ext;
namespace OHOS {
namespace HiviewDFX {
void HicollieCollectorTest::SetUp()
{
    /**
     * @tc.setup: create work directories
     */
    printf("SetUp.\n");
}
void HicollieCollectorTest::SetUpTestCase()
{
    /**
     * @tc.setup: all first
     */
    printf("SetUpTestCase.\n");

    HiviewPlatform &platform = HiviewPlatform::GetInstance();
    if (!platform.InitEnvironment("/data/test/test_data/hiview_platform_config")) {
        printf("Fail to init environment.\n");
    }
}

void HicollieCollectorTest::TearDownTestCase()
{
    /**
     * @tc.setup: all end
     */
    printf("TearDownTestCase.\n");
}

void HicollieCollectorTest::TearDown()
{
    /**
     * @tc.teardown: destroy the event loop we have created
     */
    printf("TearDown.\n");
}

bool HicollieCollectorTest::SendServiceWaring(uint64_t time, std::shared_ptr<Plugin> plugin)
{
    auto jsonStr = "{\"domain_\":\"FRAMEWORK\"}";
    auto sysEvent = std::make_shared<SysEvent>("HicollieCollectorTest001", nullptr, jsonStr);
    sysEvent->SetEventValue("name_", "SERVICE_WARNING");
    sysEvent->SetEventValue("type_", 1);
    sysEvent->SetEventValue("time_", TimeUtil::GetMilliseconds());
    sysEvent->SetEventValue("pid_", getpid());
    sysEvent->SetEventValue("tid_", gettid());
    sysEvent->SetEventValue("uid_", getuid());
    sysEvent->SetEventValue("tz_", TimeUtil::GetTimeZone());
    sysEvent->SetEventValue("PID", getpid());
    sysEvent->SetEventValue("TGID", getgid());
    sysEvent->SetEventValue("UID", getuid());
    sysEvent->SetEventValue("MSG", "this is test\n SERVICE_WARNING event time is " + std::to_string(time));
    sysEvent->SetEventValue("MODULE_NAME", "HicollieCollectorTest001");
    sysEvent->SetEventValue("PROCESS_NAME", "HicollieCollectorTest001");

    sysEvent->SetEventValue("info_", "");
    if (sysEvent->ParseJson() < 0) {
        printf("Failed to parse from queryResult.\n");
        return false;
    }

    auto context = plugin->GetHiviewContext();
    if (context != nullptr) {
        auto seq = context->GetPipelineSequenceByName("SysEventPipeline");
        sysEvent->SetPipelineInfo("SysEventPipeline", seq);
        sysEvent->OnContinue();
    }
    return true;
}

bool HicollieCollectorTest::SendServiceBlock(uint64_t time, std::shared_ptr<Plugin> plugin)
{
    auto jsonStr = "{\"domain_\":\"FRAMEWORK\"}";
    auto sysEvent = std::make_shared<SysEvent>("HicollieCollectorTest001", nullptr, jsonStr);
    sysEvent->SetEventValue("name_", "SERVICE_BLOCK");
    sysEvent->SetEventValue("type_", 1);
    sysEvent->SetEventValue("time_", TimeUtil::GetMilliseconds());
    sysEvent->SetEventValue("pid_", getpid());
    sysEvent->SetEventValue("tid_", gettid());
    sysEvent->SetEventValue("uid_", getuid());
    sysEvent->SetEventValue("tz_", TimeUtil::GetTimeZone());
    sysEvent->SetEventValue("PID", getpid());
    sysEvent->SetEventValue("TGID", getgid());
    sysEvent->SetEventValue("UID", getuid());
    sysEvent->SetEventValue("MSG", "this is test\n SERVICE_BLOCK event time is " + std::to_string(time));
    sysEvent->SetEventValue("MODULE_NAME", "HicollieCollectorTest001");
    sysEvent->SetEventValue("PROCESS_NAME", "HicollieCollectorTest001");

    sysEvent->SetEventValue("info_", "");
    if (sysEvent->ParseJson() < 0) {
        printf("Failed to parse from queryResult\n");
        return false;
    }

    auto context = plugin->GetHiviewContext();
    if (context != nullptr) {
        auto seq = context->GetPipelineSequenceByName("SysEventPipeline");
        sysEvent->SetPipelineInfo("SysEventPipeline", seq);
        sysEvent->OnContinue();
    }
    return true;
}

bool HicollieCollectorTest::GetHicollieCollectorTest001File(uint64_t time1, uint64_t time2)
{
    int count = 0;
    std::string decLogPath = "";
    while (count < 10) { // 10: 最大等待10s
        sleep(1);
        std::vector<std::string> files;
        FileUtil::GetDirFiles("/data/log/faultlog/faultlogger/", files);
        ++count;
        for (auto& i : files) {
            if (i.find("HicollieCollectorTest001") == std::string::npos) {
                continue;
            }
            std::string content;
            FileUtil::LoadStringFromFile(i, content);
            if (content.find(std::to_string(time1)) == std::string::npos
                || content.find(std::to_string(time2)) == std::string::npos) {
                printf("time is not match. %llu\n", time1);
                FileUtil::RemoveFile(i);
                continue;
            }

            if (content.find("SERVICE_WARNING") == std::string::npos
                || content.find("SERVICE_BLOCK") == std::string::npos) {
                printf("Not SERVICE_WARNING || SERVICE_BLOCK.\n");
                FileUtil::RemoveFile(i);
                continue;
            }

            if (content.find("HicollieCollectorTest001") == std::string::npos) {
                printf("Not HicollieCollectorTest001.\n");
                FileUtil::RemoveFile(i);
                continue;
            }
            decLogPath = i;
            break;
        }

        if (decLogPath != "") {
            printf("decLogPath is %s.\n", decLogPath.c_str());
            break;
        }
    }

    printf("decLogPath is %s.\n", decLogPath.c_str());
    if (decLogPath == "") {
        printf("Not find files.\n");
        return false;
    }

    FileUtil::RemoveFile(decLogPath);
    return true;
}

/**
 * @tc.name: HicollieCollectorTest001
 * @tc.desc: HicollieCollector send SERVICE_BLOCK
 * @tc.type: FUNC
 * @tc.require: AR000H3T5D
 */
HWTEST_F(HicollieCollectorTest, HicollieCollectorTest001, TestSize.Level3)
{
    HiviewPlatform &platform = HiviewPlatform::GetInstance();
    std::shared_ptr<Plugin> plugin = platform.GetPluginByName("HiCollieCollector");
    if (plugin == nullptr) {
        printf("Get HiCollieCollector, failed");
        FAIL();
    }

    auto time1 = TimeUtil::GetMilliseconds();
    SendServiceWaring(time1, plugin);

    sleep(3);

    auto time2 = TimeUtil::GetMilliseconds();
    SendServiceBlock(time2, plugin);

    if (!GetHicollieCollectorTest001File(time1, time2)) {
        printf("GetFreezeDectorTest001File, failed\n");
        FAIL();
    }
}

bool HicollieCollectorTest::GetHicollieCollectorTest002File(uint64_t time)
{
    int count = 0;
    std::string decLogPath = "";
    while (count < 10) { // 10: 最大等待10s
        sleep(1);
        std::vector<std::string> files;
        FileUtil::GetDirFiles("/data/log/faultlog/faultlogger/", files);
        ++count;
        for (auto& i : files) {
            if (i.find("HicollieCollectorTest002") == std::string::npos) {
                continue;
            }
            std::string content;
            FileUtil::LoadStringFromFile(i, content);
            if (content.find(std::to_string(time)) == std::string::npos) {
                printf("time is not match.\n");
                FileUtil::RemoveFile(i);
                continue;
            }

            if (content.find("SERVICE_TIMEOUT") == std::string::npos) {
                printf("Not SERVICE_TIMEOUT.\n");
                FileUtil::RemoveFile(i);
                continue;
            }

            if (content.find("HicollieCollectorTest002") == std::string::npos) {
                printf("Not HicollieCollectorTest002.\n");
                FileUtil::RemoveFile(i);
                continue;
            }
            decLogPath = i;
            break;
        }

        if (decLogPath != "") {
            printf("decLogPath is %s.\n", decLogPath.c_str());
            break;
        }
    }

    printf("decLogPath is %s.\n", decLogPath.c_str());
    if (decLogPath == "") {
        printf("Not find files.\n");
        return false;
    }

    FileUtil::RemoveFile(decLogPath);
    return true;
}

/**
 * @tc.name: HicollieCollectorTest002
 * @tc.desc: HicollieCollector send SERVICE_TIMEOUT
 * @tc.type: FUNC
 * @tc.require: AR000H3T5D
 */
HWTEST_F(HicollieCollectorTest, HicollieCollectorTest002, TestSize.Level3)
{
    HiviewPlatform &platform = HiviewPlatform::GetInstance();
    std::shared_ptr<Plugin> plugin = platform.GetPluginByName("HiCollieCollector");
    if (plugin == nullptr) {
        printf("Get HiCollieCollector, failed");
        FAIL();
    }

    auto time = TimeUtil::GetMilliseconds();
    auto jsonStr = "{\"domain_\":\"FRAMEWORK\"}";
    auto sysEvent = std::make_shared<SysEvent>("HicollieCollectorTest002", nullptr, jsonStr);
    sysEvent->SetEventValue("name_", "SERVICE_TIMEOUT");
    sysEvent->SetEventValue("type_", 1);
    sysEvent->SetEventValue("time_", TimeUtil::GetMilliseconds());
    sysEvent->SetEventValue("pid_", getpid());
    sysEvent->SetEventValue("tid_", gettid());
    sysEvent->SetEventValue("uid_", getuid());
    sysEvent->SetEventValue("tz_", TimeUtil::GetTimeZone());
    sysEvent->SetEventValue("PID", getpid());
    sysEvent->SetEventValue("TGID", getgid());
    sysEvent->SetEventValue("UID", getuid());
    sysEvent->SetEventValue("MSG", "this is test\n SERVICE_TIMEOUT event time is " + std::to_string(time));
    sysEvent->SetEventValue("MODULE_NAME", "HicollieCollectorTest002");
    sysEvent->SetEventValue("PROCESS_NAME", "HicollieCollectorTest002");

    sysEvent->SetEventValue("info_", "");
    if (sysEvent->ParseJson() < 0) {
        printf("Failed to parse from queryResult.\n");
        FAIL();
    }

    auto context = plugin->GetHiviewContext();
    if (context != nullptr) {
        auto seq = context->GetPipelineSequenceByName("SysEventPipeline");
        sysEvent->SetPipelineInfo("SysEventPipeline", seq);
        sysEvent->OnContinue();
    }

    if (!GetHicollieCollectorTest002File(time)) {
        printf("GetHicollieCollectorTest002File, failed\n");
        FAIL();
    }
}
}
}