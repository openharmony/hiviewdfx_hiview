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

#include "event_service_action_test.h"

#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include "event_json_parser.h"
#include "event.h"
#include "hiview_global.h"
#include "hiview_platform.h"
#include "sys_event.h"
#include "sys_event_stat.h"
#include "sys_event_db_mgr.h"
#include "sys_event_dao.h"
#include "sys_event_db_backup.h"

namespace OHOS {
namespace HiviewDFX {
void EventServiceActionTest::SetUpTestCase()
{
    OHOS::HiviewDFX::HiviewPlatform &platform = HiviewPlatform::GetInstance();
    std::string defaultDir = "/data/test/test_data/hiview_platform_config";
    if (!platform.InitEnvironment(defaultDir)) {
        std::cout << "fail to init environment" << std::endl;
    } else {
        std::cout << "init environment successful" << std::endl;
    }
}

void EventServiceActionTest::TearDownTestCase() {}

void EventServiceActionTest::SetUp() {}

void EventServiceActionTest::TearDown() {}

/**
 * @tc.name: EventJsonParserTest001
 * @tc.desc: parse a event and check Json info
 * @tc.type: FUNC
 * @tc.require: SR000GGSVB
 */
HWTEST_F(EventServiceActionTest, EventJsonParserTest001, testing::ext::TestSize.Level3)
{
    printf("start EventServiceActionTest\n");
    constexpr char JSON_STR[] = "{\"domain_\":\"DEMO\",\"name_\":\"EVENT_NAME_A\",\"type_\":4,\
        \"PARAM_A\":\"param a\",\"PARAM_B\":\"param b\"}";
    auto sysEvent = std::make_shared<SysEvent>("SysEventService", nullptr, JSON_STR);
    std::string yamlFile =
        HiviewGlobal::GetInstance()->GetHiViewDirectory(HiviewContext::DirectoryType::CONFIG_DIRECTORY);
    yamlFile = (yamlFile.back() != '/') ? (yamlFile + "/hisysevent.def") : (yamlFile + "hisysevent.def");
    auto sysEventParser = std::make_unique<EventJsonParser>(yamlFile);
    ASSERT_TRUE(sysEventParser->HandleEventJson(sysEvent));
}

int OpenTestFile(const char *filename)
{
    int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd <= 0) {
        printf("[ NET ]:open file error\n");
        return -1;
    }
    return fd;
}

std::string GetFileContent(const std::string& filename)
{
    std::ifstream inputfile(filename);
    std::string contents;
    std::string temp;
    while (inputfile >> temp) {
        contents.append(temp);
    }
    return contents;
}

/**
 * @tc.name: SysEventStatTest002
 * @tc.desc: check event state
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(EventServiceActionTest, SysEventStatTest002, testing::ext::TestSize.Level3)
{
    SysEventStat sysEventStat_;
    sysEventStat_.AccumulateEvent(true);
    sysEventStat_.AccumulateEvent(false);
    sysEventStat_.AccumulateEvent("domain1", "eventName_1");
    sysEventStat_.AccumulateEvent("domain1", "eventName_2");
    sysEventStat_.AccumulateEvent("domain2", "eventName_3");
    sysEventStat_.AccumulateEvent("domain4", "eventName_4", false);
    sysEventStat_.AccumulateEvent("domain4", "eventName_5", false);
    sysEventStat_.AccumulateEvent("domain5", "eventName_6", false);
    int fd1 = OpenTestFile("./fd1.txt");
    ASSERT_FALSE(fd1 == -1);
    sysEventStat_.StatSummary(fd1);
    close(fd1);
    std::string result1 = GetFileContent("./fd1.txt");
    ASSERT_TRUE(result1.size() > 0);

    int fd2 = OpenTestFile("./fd2.txt");
    sysEventStat_.StatDetail(fd2);
    close(fd2);
    std::string result2 = GetFileContent("./fd2.txt");
    ASSERT_TRUE(result2.size() > 0);

    int fd3 = OpenTestFile("./fd3.txt");
    sysEventStat_.StatInvalidDetail(fd3);
    close(fd3);
    std::string result3 = GetFileContent("./fd3.txt");
    ASSERT_TRUE(result3.size() > 0);

    int fd4 = OpenTestFile("./fd4.txt");
    sysEventStat_.Clear(fd4);
    close(fd4);
    std::string result4 = GetFileContent("./fd4.txt");
    std::cout << "result4:" << result4 << std::endl;
    ASSERT_TRUE(result4 == "cleanallstatinfo");
}

/**
 * @tc.name: SysEventDao003
 * @tc.desc: check event Dao
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(EventServiceActionTest, SysEventDao003, testing::ext::TestSize.Level3)
{
    std::string jsonStr1 = R"~({"domain_":"demo","name_":"SysEventDaoTest_002","type_":1,"tz_":8,"time_":162027129110,
        "pid_":6527,"tid_":6527,"traceid_":"f0ed5160bb2df4b","spanid_":"10","pspanid_":"20","trace_flag_":4,
        "keyBool":1})~";
    auto sysEvent = std::make_shared<SysEvent>("SysEventSource", nullptr, jsonStr1);
    std::cout <<" SysEventDao003:" << 0 << std::endl;
    ASSERT_TRUE(sysEvent->ParseJson() == 0);
    auto sysEventDbMgrPtr = std::make_unique<SysEventDbMgr>();
    sysEventDbMgrPtr->SaveToStore(sysEvent);

    auto sysEventQuery = EventStore::SysEventDao::BuildQuery(EventStore::StoreType::FAULT);
    std::vector<std::string> selections { EventStore::EventCol::NAME };
    EventStore::ResultSet resultSet = (*sysEventQuery).Select(selections).
        Where(EventStore::EventCol::NAME, EventStore::Op::EQ, "SysEventDaoTest_002").Execute();
    int count = 0;
    while (resultSet.HasNext()) {
        EventStore::ResultSet::RecordIter it = resultSet.Next();
        std::cout << "seq=" << it->GetSeq() << std::endl;
        count++;
    }
    ASSERT_TRUE(count > 0);
    std::cout <<" count:" << count << std::endl;
    for (int i = 0; i < 3000; i++) {
        std::string jsonStr = R"~({"domain_":"demo","name_":"SysEventDaoTest_003","type_":1,"tz_":8,
        "time_":1620271291200,"pid_":6527,"tid_":6527,"traceid_":"f0ed5160bb2df4b","spanid_":"10","pspanid_":"20",
        "trace_flag_":4,"keyBool":1,"keyChar":97})~";
        auto sysEventTemp = std::make_shared<SysEvent>("SysEventSource", nullptr, jsonStr);
        sysEventTemp->ParseJson();
        EventStore::SysEventDao::Insert(sysEventTemp);
    }
    sysEventDbMgrPtr->StartCheckStoreTask(nullptr);
    sysEventDbMgrPtr->CheckStore();
    SysEventDbBackup dbBackup(EventStore::StoreType::FAULT);
    ASSERT_TRUE(dbBackup.IsBroken() == 0);
    ASSERT_TRUE(dbBackup.Recover() == 1);
}

} // namespace HiviewDFX
} // namespace OHOS