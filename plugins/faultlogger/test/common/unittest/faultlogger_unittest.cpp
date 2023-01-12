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
#include <string>
#include <vector>

#include <fcntl.h>
#include <gtest/gtest.h>
#include "sys_event.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "event.h"
#include "faultlog_util.h"
#include "faultlog_database.h"
#define private public
#include "faultlogger.h"
#undef private
#include "faultlog_info_ohos.h"
#include "faultlogger_adapter.h"
#include "faultlogger_service_ohos.h"
#include "file_util.h"
#include "hiview_global.h"
#include "hiview_platform.h"
#include "log_analyzer.h"
#include "sys_event.h"
#include "sys_event_dao.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace HiviewDFX {
class FaultloggerUnittest : public testing::Test {
public:
    void SetUp()
    {
        sleep(1);
    };
    void TearDown(){};

    std::shared_ptr<Faultlogger> CreateFaultloggerInstance() const
    {
        static std::unique_ptr<HiviewPlatform> platform = std::make_unique<HiviewPlatform>();
        auto plugin = std::make_shared<Faultlogger>();
        plugin->SetName("Faultlogger");
        plugin->SetHandle(nullptr);
        plugin->SetHiviewContext(platform.get());
        plugin->OnLoad();
        return plugin;
    }
};

static void InitHiviewContext()
{
    OHOS::HiviewDFX::HiviewPlatform &platform = HiviewPlatform::GetInstance();
    bool result = platform.InitEnvironment("/data/test/test_faultlogger_data/hiview_platform_config");
    printf("InitHiviewContext result:%d\n", result);
}

/**
 * @tc.name: dumpFileListTest001
 * @tc.desc: dump with cmds, check the result
 * @tc.type: FUNC
 * @tc.require: SR000F7UQ6 AR000F83AF
 */
HWTEST_F(FaultloggerUnittest, dumpFileListTest001, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. add multiple cmds to faultlogger
     * @tc.expected: check the content size of the dump function
     */
    auto plugin = CreateFaultloggerInstance();
    auto fd = open("/data/test/testFile", O_CREAT | O_WRONLY | O_TRUNC, 770);
    if (fd < 0) {
        printf("Fail to create test result file.\n");
        return;
    }

    std::vector<std::string> cmds;
    plugin->Dump(fd, cmds);
    cmds.push_back("Faultlogger");
    plugin->Dump(fd, cmds);
    cmds.push_back("-l");
    plugin->Dump(fd, cmds);
    cmds.push_back("-f");
    plugin->Dump(fd, cmds);
    cmds.push_back("cppcrash-ModuleName-10-20201209103823");
    plugin->Dump(fd, cmds);
    cmds.push_back("-d");
    plugin->Dump(fd, cmds);
    cmds.push_back("-t");
    plugin->Dump(fd, cmds);
    cmds.push_back("20201209103823");
    plugin->Dump(fd, cmds);
    cmds.push_back("-m");
    plugin->Dump(fd, cmds);
    cmds.push_back("FAULTLOGGER");
    plugin->Dump(fd, cmds);
    close(fd);
    fd = -1;

    std::string result;
    if (FileUtil::LoadStringFromFile("/data/test/testFile", result)) {
        ASSERT_GT(result.length(), 0ul);
    } else {
        FAIL();
    }
}

/**
 * @tc.name: genCppCrashLogTest001
 * @tc.desc: create cpp crash event and send it to faultlogger
 * @tc.type: FUNC
 * @tc.require: SR000F7UQ6 AR000F4380
 */
HWTEST_F(FaultloggerUnittest, genCppCrashLogTest001, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. create a cpp crash event and pass it to faultlogger
     * @tc.expected: the calling is success and the file has been created
     */
    auto plugin = CreateFaultloggerInstance();
    FaultLogInfo info;
    info.time = 1607161163;
    info.id = 0;
    info.pid = 7497;
    info.faultLogType = 2;
    info.module = "com.example.myapplication";
    info.sectionMap["APPVERSION"] = "1.0";
    info.sectionMap["FAULT_MESSAGE"] = "Nullpointer";
    info.sectionMap["TRACEID"] = "0x1646145645646";
    info.sectionMap["KEY_THREAD_INFO"] = "Test Thread Info";
    info.sectionMap["REASON"] = "TestReason";
    info.sectionMap["STACKTRACE"] = "#01 xxxxxx\n#02 xxxxxx\n";
    plugin->AddFaultLog(info);
    std::string timeStr = GetFormatedTime(info.time);
    std::string fileName = "/data/log/faultlog/faultlogger/cppcrash-com.example.myapplication-0-" + timeStr;
    bool exist = FileUtil::FileExists(fileName);
    ASSERT_EQ(exist, true);
    auto size = FileUtil::GetFileSize(fileName);
    ASSERT_GT(size, 0ul);
    auto parsedInfo = plugin->GetFaultLogInfo(fileName);
    ASSERT_EQ(parsedInfo->module, "com.example.myapplication");
}

/**
 * @tc.name: genCppCrashtoAnalysisFaultlog
 * @tc.desc: create cpp crash event and check AnalysisFaultlog
 * @tc.type: FUNC
 * @tc.require: SR000F7UQ6 AR000F4380
 */
HWTEST_F(FaultloggerUnittest, genCppCrashtoAnalysisFaultlog001, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. create a cpp crash event and pass it to faultlogger
     * @tc.expected: AnalysisFaultlog return expected result
     */
    FaultLogInfo info;
    info.time = 1607161163;
    info.id = 0;
    info.pid = 7497;
    info.faultLogType = 2;
    info.module = "com.example.testapplication";
    info.reason = "TestReason";
    std::map<std::string, std::string> eventInfos;
    ASSERT_EQ(AnalysisFaultlog(info, eventInfos), false);
    ASSERT_EQ(!eventInfos["fingerPrint"].empty(), true);
}

/**
 * @tc.name: genjserrorLogTest002
 * @tc.desc: create JS ERROR event and send it to faultlogger
 * @tc.type: FUNC
 * @tc.require: SR000F7UQ6 AR000F4380
 */
HWTEST_F(FaultloggerUnittest, genjserrorLogTest002, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. create a jss_error event and pass it to faultlogger
     * @tc.expected: the calling is success and the file has been created
     */
    SysEventCreator sysEventCreator("AAFWK", "JSERROR", SysEventCreator::FAULT);
    sysEventCreator.SetKeyValue("SUMMARY", "Error message:is not callable\nStacktrace:");
    sysEventCreator.SetKeyValue("name_", "JS_ERROR");
    sysEventCreator.SetKeyValue("happenTime_", 1670248360359);
    sysEventCreator.SetKeyValue("REASON", "TypeError");
    sysEventCreator.SetKeyValue("tz_", "+0800");
    sysEventCreator.SetKeyValue("pid_", 2413);
    sysEventCreator.SetKeyValue("tid_", 2413);
    sysEventCreator.SetKeyValue("what_", 3);
    sysEventCreator.SetKeyValue("PACKAGE_NAME", "com.ohos.systemui");
    sysEventCreator.SetKeyValue("VERSION", "1.0.0");
    sysEventCreator.SetKeyValue("TYPE", 3);
    sysEventCreator.SetKeyValue("VERSION", "1.0.0");

    auto sysEvent = std::make_shared<SysEvent>("test", nullptr, sysEventCreator);
    auto testPlugin = CreateFaultloggerInstance();
    std::shared_ptr<Event> event = std::dynamic_pointer_cast<Event>(sysEvent);
    bool result = testPlugin->OnEvent(event);
    ASSERT_EQ(result, true);
}

/**
 * @tc.name: SaveFaultLogInfoTest001
 * @tc.desc: Test calling SaveFaultLogInfo Func
 * @tc.type: FUNC
 */
HWTEST_F(FaultloggerUnittest, SaveFaultLogInfoTest001, testing::ext::TestSize.Level3)
{
    InitHiviewContext();
    FaultLogDatabase *faultLogDb = new FaultLogDatabase();
    FaultLogInfo info;
    info.time = std::time(nullptr); // 3 : index of timestamp
    info.pid = getpid();
    info.id = 0;
    info.faultLogType = 2;
    info.module = "FaultloggerUnittest";
    info.reason = "unittest for SaveFaultLogInfo";
    info.summary = "summary for SaveFaultLogInfo";
    info.sectionMap["APPVERSION"] = "1.0";
    info.sectionMap["FAULT_MESSAGE"] = "abort";
    info.sectionMap["TRACEID"] = "0x1646145645646";
    info.sectionMap["KEY_THREAD_INFO"] = "Test Thread Info";
    info.sectionMap["REASON"] = "TestReason";
    info.sectionMap["STACKTRACE"] = "#01 xxxxxx\n#02 xxxxxx\n";
    faultLogDb->SaveFaultLogInfo(info);

    std::string cmd = "hisysevent -l | grep " + std::to_string(info.time);
    FILE* fp = popen(cmd.c_str(), "r");
    char buffer[1024] = {0};
    if (fp != nullptr) {
        fgets(buffer, sizeof(buffer), fp);
        pclose(fp);
        std::string str(buffer);
        if (str.find(std::to_string(info.time).c_str()) != std::string::npos) {
            printf("sucess!\r\n");
        } else {
            FAIL();
        }
    } else {
        FAIL();
    }
}

/**
 * @tc.name: GetFaultInfoListTest001
 * @tc.desc: Test calling GetFaultInfoList Func
 * @tc.type: FUNC
 */
HWTEST_F(FaultloggerUnittest, GetFaultInfoListTest001, testing::ext::TestSize.Level3)
{
    InitHiviewContext();

    std::string jsonStr = R"~({"domain_":"RELIABILITY","name_":"CPP_CRASH","type_":1,"time_":1501973701070,"tz_":
    "+0800","pid_":1854,"tid_":1854,"uid_":0,"FAULT_TYPE":"2","PID":1854,"UID":0,"MODULE":"FaultloggerUnittest",
    "REASON":"unittest for SaveFaultLogInfo","SUMMARY":"summary for SaveFaultLogInfo","LOG_PATH":"","VERSION":"",
    "HAPPEN_TIME":"1501973701","PNAME":"/","FIRST_FRAME":"/","SECOND_FRAME":"/","LAST_FRAME":"/","FINGERPRINT":
    "04c0d6f03c73da531f00eb112479a8a2f19f59fafba6a474dcbe455a13288f4d","level_":"CRITICAL","tag_":"STABILITY","id_":
    "17165544771317691984","info_":"","seq_":447})~";
    auto sysEvent = std::make_shared<SysEvent>("SysEventSource", nullptr, jsonStr);
    ASSERT_TRUE(sysEvent->ParseJson() == 0);
    EventStore::SysEventDao::Insert(sysEvent);
    FaultLogDatabase *faultLogDb = new FaultLogDatabase();
    std::list<FaultLogInfo> infoList = faultLogDb->GetFaultInfoList("FaultloggerUnittest", 0, 2, 10);
    ASSERT_GT(infoList.size(), 0);
}

/**
 * @tc.name: FaultLogManager::CreateTempFaultLogFile
 * @tc.desc: Test calling CreateTempFaultLogFile Func
 * @tc.type: FUNC
 */
HWTEST_F(FaultloggerUnittest, FaultlogManager001, testing::ext::TestSize.Level3)
{
    std::unique_ptr<FaultLogManager> faultLogManager = std::make_unique<FaultLogManager>(nullptr);
    faultLogManager->Init();
    int fd = faultLogManager->CreateTempFaultLogFile(1607161345, 0, 2, "FaultloggerUnittest");
    ASSERT_GT(fd, 0);
}

/**
 * @tc.name: FaultLogManager::SaveFaultInfoToRawDb
 * @tc.desc: Test calling SaveFaultInfoToRawDb Func
 * @tc.type: FUNC
 */
HWTEST_F(FaultloggerUnittest, FaultLogManagerTest001, testing::ext::TestSize.Level3)
{
    InitHiviewContext();

    FaultLogInfo info;
    info.time = std::time(nullptr); // 3 : index of timestamp
    info.pid = getpid();
    info.id = 0;
    info.faultLogType = 2;
    info.module = "FaultloggerUnittest1111";
    info.reason = "unittest for SaveFaultLogInfo";
    info.summary = "summary for SaveFaultLogInfo";
    info.sectionMap["APPVERSION"] = "1.0";
    info.sectionMap["FAULT_MESSAGE"] = "abort";
    info.sectionMap["TRACEID"] = "0x1646145645646";
    info.sectionMap["KEY_THREAD_INFO"] = "Test Thread Info";
    info.sectionMap["REASON"] = "TestReason";
    info.sectionMap["STACKTRACE"] = "#01 xxxxxx\n#02 xxxxxx\n";
    std::unique_ptr<FaultLogManager> faultLogManager = std::make_unique<FaultLogManager>(nullptr);
    faultLogManager->Init();
    faultLogManager->SaveFaultInfoToRawDb(info);

    std::string cmd = "hisysevent -l | grep " + std::to_string(info.time);
    FILE* fp = popen(cmd.c_str(), "r");
    char buffer[1024] = {0};
    if (fp != nullptr) {
        fgets(buffer, sizeof(buffer), fp);
        pclose(fp);
        std::string str(buffer);
        if (str.find(std::to_string(info.time).c_str()) != std::string::npos) {
            printf("sucess!\r\n");
        } else {
            FAIL();
        }
    } else {
        FAIL();
    }
}

/**
 * @tc.name: FaultLogManager::SaveFaultLogToFile
 * @tc.desc: Test calling SaveFaultLogToFile Func
 * @tc.type: FUNC
 */
HWTEST_F(FaultloggerUnittest, FaultLogManagerTest003, testing::ext::TestSize.Level3)
{
    InitHiviewContext();

    FaultLogInfo info;
    std::unique_ptr<FaultLogManager> faultLogManager = std::make_unique<FaultLogManager>(nullptr);
    faultLogManager->Init();
    for (int i = 1; i < 7; i++) {
        info.time = std::time(nullptr); // 3 : index of timestamp
        info.pid = getpid();
        info.id = 0;
        info.faultLogType = i;
        info.module = "FaultloggerUnittest1111";
        info.reason = "unittest for SaveFaultLogInfo";
        info.summary = "summary for SaveFaultLogInfo";
        info.sectionMap["APPVERSION"] = "1.0";
        info.sectionMap["FAULT_MESSAGE"] = "abort";
        info.sectionMap["TRACEID"] = "0x1646145645646";
        info.sectionMap["KEY_THREAD_INFO"] = "Test Thread Info";
        info.sectionMap["REASON"] = "TestReason";
        info.sectionMap["STACKTRACE"] = "#01 xxxxxx\n#02 xxxxxx\n";

        std::string fileName = faultLogManager->SaveFaultLogToFile(info);
        if (fileName.find("FaultloggerUnittest1111") == std::string::npos) {
            FAIL();
        }
    }
}

/**
 * @tc.name: faultLogManager GetFaultInfoListTest001
 * @tc.desc: Test calling faultLogManager.GetFaultInfoList Func
 * @tc.type: FUNC
 */
HWTEST_F(FaultloggerUnittest, FaultLogManagerTest002, testing::ext::TestSize.Level3)
{
    InitHiviewContext();

    std::string jsonStr = R"~({"domain_":"RELIABILITY","name_":"CPP_CRASH","type_":1,"time_":1501973701070,"tz_":
    "+0800","pid_":1854,"tid_":1854,"uid_":0,"FAULT_TYPE":"2","PID":1854,"UID":0,"MODULE":"FaultloggerUnittest",
    "REASON":"unittest for SaveFaultLogInfo","SUMMARY":"summary for SaveFaultLogInfo","LOG_PATH":"","VERSION":"",
    "HAPPEN_TIME":"1501973701","PNAME":"/","FIRST_FRAME":"/","SECOND_FRAME":"/","LAST_FRAME":"/","FINGERPRINT":
    "04c0d6f03c73da531f00eb112479a8a2f19f59fafba6a474dcbe455a13288f4d","level_":"CRITICAL","tag_":"STABILITY","id_":
    "17165544771317691984","info_":"","seq_":447})~";
    auto sysEvent = std::make_shared<SysEvent>("SysEventSource", nullptr, jsonStr);
    ASSERT_TRUE(sysEvent->ParseJson() == 0);
    EventStore::SysEventDao::Insert(sysEvent);

    std::unique_ptr<FaultLogManager> faultLogManager = std::make_unique<FaultLogManager>(nullptr);
    auto isProcessedFault1 = faultLogManager->IsProcessedFault(1854, 0, 2);
    ASSERT_EQ(isProcessedFault1, false);

    faultLogManager->Init();

    auto list = faultLogManager->GetFaultInfoList("FaultloggerUnittest", 0, 2, 10);
    ASSERT_GT(list.size(), 0);

    auto isProcessedFault2 = faultLogManager->IsProcessedFault(1854, 0, 2);
    ASSERT_EQ(isProcessedFault2, true);

    auto isProcessedFault3 = faultLogManager->IsProcessedFault(1855, 0, 2);
    ASSERT_EQ(isProcessedFault3, false);

    auto isProcessedFault4 = faultLogManager->IsProcessedFault(1855, 5, 2);
    ASSERT_EQ(isProcessedFault4, false);
}

/**
 * @tc.name: FaultLogUtilTest001
 * @tc.desc: check ExtractInfoFromFileName Func
 * @tc.type: FUNC
 */
HWTEST_F(FaultloggerUnittest, FaultLogUtilTest001, testing::ext::TestSize.Level3)
{
    std::string filename = "appfreeze-com.ohos.systemui-10006-20170805172159";
    auto info = ExtractInfoFromFileName(filename);
    ASSERT_EQ(info.pid, 0);
    ASSERT_EQ(info.faultLogType, FaultLogType::APP_FREEZE); // 4 : APP_FREEZE
    ASSERT_EQ(info.module, "com.ohos.systemui");
    ASSERT_EQ(info.id, 10006); // 10006 : test uid
}

/**
 * @tc.name: FaultLogUtilTest002
 * @tc.desc: check ExtractInfoFromTempFile Func
 * @tc.type: FUNC
 */
HWTEST_F(FaultloggerUnittest, FaultLogUtilTest002, testing::ext::TestSize.Level3)
{
    std::string filename = "appfreeze-10006-20170805172159";
    auto info = ExtractInfoFromTempFile(filename);
    ASSERT_EQ(info.faultLogType, FaultLogType::APP_FREEZE); // 4 : APP_FREEZE
    ASSERT_EQ(info.pid, 10006); // 10006 : test uid

    std::string filename3 = "jscrash-10006-20170805172159";
    auto info3 = ExtractInfoFromTempFile(filename3);
    ASSERT_EQ(info3.faultLogType, FaultLogType::JS_CRASH); // 3 : JS_CRASH
    ASSERT_EQ(info3.pid, 10006); // 10006 : test uid

    std::string filename4 = "cppcrash-10006-20170805172159";
    auto info4 = ExtractInfoFromTempFile(filename4);
    ASSERT_EQ(info4.faultLogType, FaultLogType::CPP_CRASH); // 2 : CPP_CRASH
    ASSERT_EQ(info4.pid, 10006); // 10006 : test uid

    std::string filename5 = "all-10006-20170805172159";
    auto info5 = ExtractInfoFromTempFile(filename5);
    ASSERT_EQ(info5.faultLogType, FaultLogType::ALL); // 0 : ALL
    ASSERT_EQ(info5.pid, 10006); // 10006 : test uid

    std::string filename6 = "other-10006-20170805172159";
    auto info6 = ExtractInfoFromTempFile(filename6);
    ASSERT_EQ(info6.faultLogType, -1); // -1 : other
    ASSERT_EQ(info6.pid, 10006); // 10006 : test uid
}

/**
 * @tc.name: FaultloggerAdapter.StartService
 * @tc.desc: Test calling FaultloggerAdapter.StartService Func
 * @tc.type: FUNC
 */
HWTEST_F(FaultloggerUnittest, FaultloggerAdapterTest001, testing::ext::TestSize.Level3)
{
    InitHiviewContext();
    FaultloggerAdapter::StartService(nullptr);
    ASSERT_EQ(FaultloggerServiceOhos::GetOrSetFaultlogger(nullptr), nullptr);

    Faultlogger faultlogger;
    FaultloggerAdapter::StartService(&faultlogger);
    ASSERT_EQ(FaultloggerServiceOhos::GetOrSetFaultlogger(nullptr), &faultlogger);
}

/**
 * @tc.name: FaultloggerServiceOhos.StartService
 * @tc.desc: Test calling FaultloggerServiceOhos.StartService Func
 * @tc.type: FUNC
 */
HWTEST_F(FaultloggerUnittest, FaultloggerServiceOhosTest001, testing::ext::TestSize.Level3)
{
    InitHiviewContext();

    auto service = CreateFaultloggerInstance();
    FaultloggerServiceOhos serviceOhos;
    FaultloggerServiceOhos::StartService(service.get());
    ASSERT_EQ(FaultloggerServiceOhos::GetOrSetFaultlogger(nullptr), service.get());
    FaultLogInfoOhos info;
    info.time = std::time(nullptr); // 3 : index of timestamp
    info.pid = getpid();
    info.uid = 0;
    info.faultLogType = 2;
    info.module = "FaultloggerUnittest333";
    info.reason = "unittest for SaveFaultLogInfo";
    serviceOhos.AddFaultLog(info);
    auto list = serviceOhos.QuerySelfFaultLog(2, 10);
    ASSERT_NE(list, nullptr);
    info.time = std::time(nullptr); // 3 : index of timestamp
    info.pid = getpid();
    info.uid = 10;
    info.faultLogType = 2;
    info.module = "FaultloggerUnittest333";
    info.reason = "unittest for SaveFaultLogInfo";
    serviceOhos.AddFaultLog(info);
    list = serviceOhos.QuerySelfFaultLog(2, 10);
    ASSERT_EQ(list, nullptr);
    info.time = std::time(nullptr); // 3 : index of timestamp
    info.pid = getpid();
    info.uid = 0;
    info.faultLogType = 2;
    info.module = "FaultloggerUnittest333";
    info.reason = "unittest for SaveFaultLogInfo";
    serviceOhos.AddFaultLog(info);
    list = serviceOhos.QuerySelfFaultLog(8, 10);
    ASSERT_EQ(list, nullptr);

    serviceOhos.Destroy();
}
} // namespace HiviewDFX
} // namespace OHOS
