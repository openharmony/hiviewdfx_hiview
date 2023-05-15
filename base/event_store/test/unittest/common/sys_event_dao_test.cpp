/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "sys_event_dao_test.h"

#include <chrono>
#include <ctime>
#include <iostream>
#include <memory>
#include <thread>

#include <gmock/gmock.h>
#include "event.h"
#include "hiview_platform.h"
#include "sys_event.h"
#include "sys_event_dao.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const char TEST_LEVEL[] = "MINOR";
}
void SysEventDaoTest::SetUpTestCase()
{
    OHOS::HiviewDFX::HiviewPlatform &platform = HiviewPlatform::GetInstance();
    std::string defaultDir = "/data/test/test_data/hiview_platform_config";
    if (!platform.InitEnvironment(defaultDir)) {
        std::cout << "fail to init environment" << std::endl;
    } else {
        std::cout << "init environment successful" << std::endl;
    }
}

void SysEventDaoTest::TearDownTestCase()
{
}

void SysEventDaoTest::SetUp()
{
}

void SysEventDaoTest::TearDown()
{
}

/**
 * @tc.name: TestSysEventDaoInsert_001
 * @tc.desc: save event to doc store
 * @tc.type: FUNC
 * @tc.require: AR000FT2Q3
 * @tc.author: zhouhaifeng
 */
HWTEST_F(SysEventDaoTest, TestSysEventDaoInsert_001, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. create pipeline event and set event id
     * @tc.steps: step2. invoke OnEvent func
     * @tc.expected: all ASSERT_TRUE work through.
     */
    std::string jsonStr = R"~({"domain_":"demo","name_":"SysEventDaoTest_001","type_":1,"tz_":8,"time_":1620271291188,
        "pid_":6527,"tid_":6527,"traceid_":"f0ed5160bb2df4b","spanid_":"10","pspanid_":"20","trace_flag_":4,"keyBool":1,
        "keyChar":97,"keyShort":-100,"keyInt":-200,"KeyLong":-300,"KeyLongLong":-400,"keyUnsignedChar":97,
        "keyUnsignedShort":100,"keyUnsignedInt":200,"keyUnsignedLong":300,"keyUnsignedLongLong":400,"keyFloat":1.1,
        "keyDouble":2.2,"keyString1":"abc","keyString2":"efg","keyBools":[1,1,0],"keyChars":[97,98,99],
        "keyUnsignedChars":[97,98,99],"keyShorts":[-100,-200,-300],"keyUnsignedShorts":[100,200,300],
        "keyInts":[-1000,-2000,-3000],"keyUnsignedInts":[1000,2000,3000],"keyLongs":[-10000,-20000,-30000],
        "keyUnsignedLongs":[10000,20000,30000],"keyLongLongs":[-100000,-200000,-300000],
        "keyUnsignedLongLongs":[100000,200000,300000],"keyFloats":[1.1,2.2,3.3],
        "keyDoubles":[10.1,20.2,30.3],"keyStrings":["a","b","c"]})~";
    auto sysEvent = std::make_shared<SysEvent>("SysEventSource", nullptr, jsonStr);
    sysEvent->SetLevel(TEST_LEVEL);
    sysEvent->SetEventSeq(0);
    int retCode = EventStore::SysEventDao::Insert(sysEvent);
    ASSERT_TRUE(retCode == 0);
}

/**
 * @tc.name: TestEventDaoQuery_002
 * @tc.desc: query event from doc store
 * @tc.type: FUNC
 * @tc.require: AR000FT2PO
 * @tc.author: zhouhaifeng
 */
HWTEST_F(SysEventDaoTest, TestEventDaoQuery_002, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. create pipeline event and set event id
     * @tc.steps: step2. invoke OnEvent func
     * @tc.expected: all ASSERT_TRUE work through.
     */
    std::string jsonStr1 = R"~({"domain_":"demo","name_":"SysEventDaoTest_002","type_":1,"tz_":8,"time_":162027129100,
        "pid_":6527,"tid_":6527,"traceid_":"f0ed5160bb2df4b","spanid_":"10","pspanid_":"20","trace_flag_":4,"keyBool":1,
        "keyChar":97,"keyShort":-100,"keyInt":-200,"KeyLong":-300,"KeyLongLong":-400,"keyUnsignedChar":97,
        "keyUnsignedShort":100,"keyUnsignedInt":200,"keyUnsignedLong":300,"keyUnsignedLongLong":400,"keyFloat":1.1,
        "keyDouble":2.2,"keyString1":"abc","keyString2":"efg","keyBools":[1,1,0],"keyChars":[97,98,99],
        "keyUnsignedChars":[97,98,99],"keyShorts":[-100,-200,-300],"keyUnsignedShorts":[100,200,300],
        "keyInts":[-1000,-2000,-3000],"keyUnsignedInts":[1000,2000,3000],"keyLongs":[-10000,-20000,-30000],
        "keyUnsignedLongs":[10000,20000,30000],"keyLongLongs":[-100000,-200000,-300000],
        "keyUnsignedLongLongs":[100000,200000,300000],"keyFloats":[1.1,2.2,3.3],
        "keyDoubles":[10.1,20.2,30.3],"keyStrings":["a","b","c"]})~";
    auto sysEvent1 = std::make_shared<SysEvent>("SysEventSource", nullptr, jsonStr1);
    sysEvent1->SetLevel(TEST_LEVEL);
    sysEvent1->SetEventSeq(1); // 1: test seq
    int retCode1 = EventStore::SysEventDao::Insert(sysEvent1);
    ASSERT_TRUE(retCode1 == 0);

    std::string jsonStr2 = R"~({"domain_":"demo","name_":"SysEventDaoTest_002","type_":1,"tz_":8,"time_":162027129110,
        "pid_":6527,"tid_":6527,"traceid_":"f0ed5160bb2df4b","spanid_":"10","pspanid_":"20","trace_flag_":4,"keyBool":1,
        "keyChar":97,"keyShort":-100,"keyInt":-200,"KeyLong":-300,"KeyLongLong":-400,"keyUnsignedChar":97,
        "keyUnsignedShort":100,"keyUnsignedInt":200,"keyUnsignedLong":300,"keyUnsignedLongLong":400,"keyFloat":1.1,
        "keyDouble":2.2,"keyString1":"abc","keyString2":"efg","keyBools":[1,1,0],"keyChars":[97,98,99],
        "keyUnsignedChars":[97,98,99],"keyShorts":[-100,-200,-300],"keyUnsignedShorts":[100,200,300],
        "keyInts":[-1000,-2000,-3000],"keyUnsignedInts":[1000,2000,3000],"keyLongs":[-10000,-20000,-30000],
        "keyUnsignedLongs":[10000,20000,30000],"keyLongLongs":[-100000,-200000,-300000],
        "keyUnsignedLongLongs":[100000,200000,300000],"keyFloats":[1.1,2.2,3.3],
        "keyDoubles":[10.1,20.2,30.3],"keyStrings":["a","b","c"]})~";
    auto sysEvent2 = std::make_shared<SysEvent>("SysEventSource", nullptr, jsonStr2);
    sysEvent2->SetLevel(TEST_LEVEL);
    sysEvent2->SetEventSeq(3); // 3: test seq
    int retCode2 = EventStore::SysEventDao::Insert(sysEvent2);
    ASSERT_TRUE(retCode2 == 0);

    auto sysEventQuery = EventStore::SysEventDao::BuildQuery("demo", {"SysEventDaoTest_002"});
    std::vector<std::string> selections { EventStore::EventCol::TS };
    EventStore::ResultSet resultSet = sysEventQuery->Select(selections).
        Where(EventStore::EventCol::TS, EventStore::Op::EQ, 162027129100).Execute();
    int count = 0;
    while (resultSet.HasNext()) {
        count++;
        EventStore::ResultSet::RecordIter it = resultSet.Next();
        ASSERT_TRUE(it->GetSeq() == sysEvent1->GetEventSeq());
        std::cout << "seq=" << it->GetSeq() << ", json=" << it->AsJsonStr() << std::endl;
    }
    ASSERT_TRUE(count == 1);
}

/**
 * @tc.name: TestEventDaoQuery_004
 * @tc.desc: test embed sql
 * @tc.type: FUNC
 * @tc.require: AR000FT2Q5
 * @tc.author: zhouhaifeng
 */
HWTEST_F(SysEventDaoTest, TestEventDaoQuery_004, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. create pipeline event and set event id
     * @tc.steps: step2. invoke OnEvent func
     * @tc.expected: all ASSERT_TRUE work through.
     */
    auto sysEventQuery = EventStore::SysEventDao::BuildQuery("dA", {"e11", "e12", "e13"});
    EventStore::Cond timeCond(EventStore::EventCol::TS, EventStore::Op::GE, 100);
    timeCond.And(EventStore::EventCol::TS, EventStore::Op::LT, 999);
    sysEventQuery->Where(timeCond);
    EventStore::ResultSet resultSet = sysEventQuery->Execute();
    int count = 0;
    while (resultSet.HasNext()) {
        count++;
    }
    ASSERT_TRUE(count == 0);
}

/**
 * @tc.name: TestEventDaoQuery_005
 * @tc.desc: test embed sql
 * @tc.type: FUNC
 * @tc.require: AR000FT2Q3
 * @tc.author: zhouhaifeng
 */
HWTEST_F(SysEventDaoTest, TestEventDaoQuery_005, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. create pipeline event and set event id
     * @tc.steps: step2. invoke OnEvent func
     * @tc.expected: all ASSERT_TRUE work through.
     */
    auto sysEventQuery = EventStore::SysEventDao::BuildQuery("d1", {"e1"});
    sysEventQuery->And(EventStore::EventCol::TS, EventStore::Op::EQ, 100)
        .And(EventStore::EventCol::TS, EventStore::Op::EQ, 100.1f);
    EventStore::ResultSet resultSet = sysEventQuery->Execute();
    int count = 0;
    while (resultSet.HasNext()) {
        count++;
    }
    ASSERT_TRUE(count == 0);
}

/**
 * @tc.name: TestEventDaoQuery_008
 * @tc.desc: test query in different threads (more than 4) at the same time.
 * @tc.type: FUNC
 * @tc.require: issueI5L2RV
 */
HWTEST_F(SysEventDaoTest, TestEventDaoQuery_008, testing::ext::TestSize.Level3)
{
    int threadCount = 5;
    EventStore::DbQueryStatus queryStatus = EventStore::DbQueryStatus::SUCCEED;
    for (int i = 0; i < threadCount; i++) {
        std::thread t([&queryStatus] () {
            auto sysEventQuery = EventStore::SysEventDao::BuildQuery("d1", {"e1"});
            int queryCount = 10;
            (void)(sysEventQuery->Execute(queryCount, { true, true }, std::make_pair(EventStore::INNER_PROCESS_ID, ""),
                [&queryStatus] (EventStore::DbQueryStatus status) {
                    if (status != EventStore::DbQueryStatus::SUCCEED) {
                        queryStatus = status;
                    }
                }));
        });
        t.detach();
    }
    sleep(8);
    ASSERT_TRUE(queryStatus == EventStore::DbQueryStatus::CONCURRENT ||
        queryStatus == EventStore::DbQueryStatus::TOO_FREQENTLY);
}

/**
 * @tc.name: TestEventDaoQuery_009
 * @tc.desc: test query with over limit
 * @tc.type: FUNC
 * @tc.require: issueI5L2RV
 */
HWTEST_F(SysEventDaoTest, TestEventDaoQuery_009, testing::ext::TestSize.Level3)
{
    auto sysEventQuery = EventStore::SysEventDao::BuildQuery("d1", {"e1"});
    EventStore::DbQueryStatus queryStatus = EventStore::DbQueryStatus::SUCCEED;
    int queryCount = 51;
    (void)(sysEventQuery->Execute(queryCount, { true, true }, std::make_pair(EventStore::INNER_PROCESS_ID, ""),
        [&queryStatus] (EventStore::DbQueryStatus status) {
            if (status != EventStore::DbQueryStatus::SUCCEED) {
                queryStatus = status;
            }
        }));
    ASSERT_TRUE(queryStatus == EventStore::DbQueryStatus::OVER_LIMIT);
}

/**
 * @tc.name: TestEventDaoQuery_010
 * @tc.desc: test query in high frequency, query twice in 1 second
 * @tc.type: FUNC
 * @tc.require: issueI5L2RV
 */
HWTEST_F(SysEventDaoTest, TestEventDaoQuery_010, testing::ext::TestSize.Level3)
{
    auto sysEventQuery = EventStore::SysEventDao::BuildQuery("d1", {"e1"});
    EventStore::DbQueryStatus queryStatus = EventStore::DbQueryStatus::SUCCEED;
    int queryCount = 10;
    (void)sysEventQuery->Execute(queryCount, { true, true }, std::make_pair(EventStore::INNER_PROCESS_ID, ""),
        [&queryStatus] (EventStore::DbQueryStatus status) {
            if (status != EventStore::DbQueryStatus::SUCCEED) {
                queryStatus = status;
            }
        });
    (void)(sysEventQuery->Execute(queryCount, { true, true }, std::make_pair(EventStore::INNER_PROCESS_ID, ""),
        [&queryStatus] (EventStore::DbQueryStatus status) {
            if (status != EventStore::DbQueryStatus::SUCCEED) {
                queryStatus = status;
            }
        }));
    ASSERT_TRUE(queryStatus == EventStore::DbQueryStatus::TOO_FREQENTLY);
}

/**
 * @tc.name: TestEventDaoQuery_011
 * @tc.desc: test query in high frequency with ejdb newest configuration for defensing event storm
 * @tc.type: FUNC
 * @tc.require: issueI5LFCZ
 */
HWTEST_F(SysEventDaoTest, TestEventDaoQuery_011, testing::ext::TestSize.Level3)
{
    auto sysEventQuery = EventStore::SysEventDao::BuildQuery("d1", {"e1"});
    EventStore::DbQueryStatus queryStatus = EventStore::DbQueryStatus::SUCCEED;
    int queryCount = 10;
    (void)sysEventQuery->Execute(queryCount, { true, true }, std::make_pair(EventStore::INNER_PROCESS_ID, ""),
        [&queryStatus] (EventStore::DbQueryStatus status) {
            if (status != EventStore::DbQueryStatus::SUCCEED) {
                queryStatus = status;
            }
        });
    (void)(sysEventQuery->Execute(queryCount, { true, true }, std::make_pair(EventStore::INNER_PROCESS_ID, ""),
        [&queryStatus] (EventStore::DbQueryStatus status) {
            if (status != EventStore::DbQueryStatus::SUCCEED) {
                queryStatus = status;
            }
        }));
    ASSERT_TRUE(queryStatus == EventStore::DbQueryStatus::TOO_FREQENTLY);
}
} // namespace HiviewDFX
} // namespace OHOS