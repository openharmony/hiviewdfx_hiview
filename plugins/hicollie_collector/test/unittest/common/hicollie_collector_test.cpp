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
#include "hisysevent.h"
#include "plugin.h"
#include "sys_event.h"

using namespace testing::ext;
namespace OHOS {
namespace HiviewDFX {
namespace {
std::shared_ptr<SysEvent> makeEvent(const std::string& name,
                                    const std::string& domain,
                                    const std::string& eventName,
                                    const std::string& packageName,
                                    const std::string& logPath)
{
    auto time = TimeUtil::GetMilliseconds();
    std::string str = "******************\n";
    str += "this is test " + eventName + " at " + std::to_string(time) + "\n";
    str += "******************\n";
    FileUtil::SaveStringToFile(logPath, str);

    auto jsonStr = "{\"domain_\":\"" + domain + "\"}";
    auto sysEvent = std::make_shared<SysEvent>(name, nullptr, jsonStr);
    sysEvent->SetEventValue("name_", eventName);
    sysEvent->SetEventValue("type_", 1);
    sysEvent->SetEventValue("time_", time);
    sysEvent->SetEventValue("pid_", getpid());
    sysEvent->SetEventValue("tid_", gettid());
    sysEvent->SetEventValue("uid_", getuid());
    sysEvent->SetEventValue("tz_", TimeUtil::GetTimeZone());
    sysEvent->SetEventValue("PID", getpid());
    sysEvent->SetEventValue("UID", getuid());
    sysEvent->SetEventValue("MSG", "test " + eventName + " event");
    sysEvent->SetEventValue("PACKAGE_NAME", packageName);
    sysEvent->SetEventValue("PROCESS_NAME", packageName);

    std::string tmpStr = R"~(logPath:)~" + logPath;
    sysEvent->SetEventValue("info_", tmpStr);
    if (sysEvent->ParseJson() < 0) {
        printf("Failed to parse from queryResult file name %s.\n", logPath.c_str());
        return nullptr;
    }
    return sysEvent;
}

bool GetFreezeDectorTestFile(const std::string& eventName,
                             const std::string& packageName,
                             uint64_t time)
{
    int count = 0;
    std::string decLogPath = "";
    while (count < 10) { // 10: 最大等待10s
        sleep(1);
        std::vector<std::string> files;
        FileUtil::GetDirFiles("/data/log/faultlog/faultlogger/", files);
        ++count;
        for (auto& i : files) {
            if (i.find(packageName) == std::string::npos) {
                continue;
            }
            std::string content;
            FileUtil::LoadStringFromFile(i, content);
            if (content.find(std::to_string(time)) == std::string::npos) {
                printf("time is not match.\n");
                FileUtil::RemoveFile(i);
                continue;
            }

            if (content.find(eventName) == std::string::npos) {
                printf("Not %s.\n", eventName.c_str());
                FileUtil::RemoveFile(i);
                continue;
            }

            if (content.find(packageName) == std::string::npos) {
                printf("Not %s.\n", packageName.c_str());
                FileUtil::RemoveFile(i);
                continue;
            }
            decLogPath = i;
            break;
        }

        if (decLogPath != "") {
            break;
        }
    }

    if (decLogPath == "") {
        printf("Not find files.\n");
        return false;
    }

    FileUtil::RemoveFile(decLogPath);
    return true;
}
}

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
        printf("Get HiCollieCollector, failed\n");
        FAIL();
    }

    std::string logPath = "/data/test/test_data/LOG001.log";
    FileUtil::CreateFile(logPath);
    if (!FileUtil::FileExists(logPath)) {
        printf("CreateFile file, failed\n");
        FAIL();
    }

    auto sysEvent = makeEvent("HicollieCollectorTest001", "FRAMEWORK", "SERVICE_TIMEOUT",
                              "HicollieCollectorTest001", logPath);
    if (sysEvent == nullptr) {
        printf("GetFreezeDectorTest001File, failed\n");
        FAIL();
    }
    uint64_t time = sysEvent->GetEventIntValue("time_");
    std::shared_ptr<OHOS::HiviewDFX::Event> event = std::static_pointer_cast<Event>(sysEvent);
    std::shared_ptr<HiCollieCollector> hiCollieCollector = std::static_pointer_cast<HiCollieCollector>(plugin);
    hiCollieCollector->OnUnorderedEvent(*(event.get()));

    sleep(3);
    if (!GetFreezeDectorTestFile("SERVICE_TIMEOUT",
                                 "HicollieCollectorTest001",
                                 time)) {
        printf("GetFreezeDectorTest001File, failed\n");
        FAIL();
    }
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
        printf("Get HiCollieCollector, failed\n");
        FAIL();
    }

    std::string logPath = "/data/test/test_data/LOG002.log";
    FileUtil::CreateFile(logPath);
    if (!FileUtil::FileExists(logPath)) {
        printf("CreateFile file, failed\n");
        FAIL();
    }

    auto sysEvent = makeEvent("HicollieCollectorTest002", "FRAMEWORK", "SERVICE_BLOCK",
                              "HicollieCollectorTest002", logPath);
    if (sysEvent == nullptr) {
        printf("GetFreezeDectorTest001File, failed\n");
        FAIL();
    }
    std::shared_ptr<OHOS::HiviewDFX::Event> event = std::static_pointer_cast<Event>(sysEvent);
    std::shared_ptr<HiCollieCollector> hiCollieCollector = std::static_pointer_cast<HiCollieCollector>(plugin);
    hiCollieCollector->OnUnorderedEvent(*(event.get()));

    sleep(3);
    ASSERT_EQ(plugin->GetPluginInfo(), "HiCollieCollector");
}
}
}
