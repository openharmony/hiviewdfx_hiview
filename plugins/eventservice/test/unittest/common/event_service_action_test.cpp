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
#include "sys_event_service.h"

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
 * @tc.require: issueI62WJT
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
 * @tc.name: SysEventService
 * @tc.desc: check sysEvent service
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(EventServiceActionTest, SysEventService004, testing::ext::TestSize.Level3)
{
    auto testPlugin = std::make_shared<SysEventService>();
    std::shared_ptr<Event> nullEvent = nullptr;
    std::cout << "ASSERT1:" << testPlugin->OnEvent(nullEvent) << std::endl;
    ASSERT_FALSE(testPlugin->OnEvent(nullEvent));
    testPlugin->OnLoad();
    testPlugin->OnUnload();
    std::string jsonStr = R"~({"domain_":"demo","name_":"SysEventDaoTest_003","type_":1,"tz_":8,
        "time_":1620271291200,"pid_":6527,"tid_":6527,"traceid_":"f0ed5160bb2df4b","spanid_":"10","pspanid_":"20",
        "trace_flag_":4,"keyBool":1,"keyChar":97})~";
    auto sysEventTemp = std::make_shared<SysEvent>("SysEventSource", nullptr, jsonStr);
    sysEventTemp->ParseJson();
    std::shared_ptr<Event> event1 = std::static_pointer_cast<Event>(sysEventTemp);
    event1->messageType_ = Event::MessageType::UE_EVENT;
    ASSERT_FALSE(testPlugin->OnEvent(event1));
    SysEventCreator sysEventCreator("domain1", "test1", SysEventCreator::FAULT);
    auto sysEvent1 = std::make_shared<SysEvent>("SysEventSource", nullptr, sysEventCreator);
    std::shared_ptr<Event> event2 = std::static_pointer_cast<Event>(sysEvent1);
    std::cout << "ASSERT3:" << testPlugin->OnEvent(event2) << std::endl;
    int fd5 = OpenTestFile("./fd5.txt");
    testPlugin->Dump(fd5, {"start", "detail"});
    testPlugin->Dump(fd5, {"start", "invalid"});
    testPlugin->Dump(fd5, {"start", "clear"});
    testPlugin->Dump(fd5, {"start", "aaa"});
    testPlugin->Dump(fd5, {"start"});
    std::string result = GetFileContent("./fd5.txt");
    ASSERT_TRUE(result.size() > 0);
}
} // namespace HiviewDFX
} // namespace OHOS