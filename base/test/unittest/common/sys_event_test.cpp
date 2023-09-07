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
#include "sys_event_test.h"

#include <ctime>
#include <iostream>
#include <limits>
#include <memory>
#include <regex>
#include <vector>

#include "sys_event.h"

namespace OHOS {
namespace HiviewDFX {
void SysEventTest::SetUpTestCase()
{
}

void SysEventTest::TearDownTestCase()
{
}

void SysEventTest::SetUp()
{
}

void SysEventTest::TearDown()
{
}

/**
 * @tc.name: TestSendBaseType001
 * @tc.desc: create base sys event
 * @tc.type: FUNC
 * @tc.require: AR000FT2Q2 AR000FT2Q3
 */
HWTEST_F(SysEventTest, TestSendBaseType001, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. create base sys event
     */
    std::cout << "TestSendBaseType001 test base type" << std::endl;
    SysEventCreator sysEventCreator("DEMO", "EVENT_NAME", SysEventCreator::FAULT);
    sysEventCreator.SetKeyValue("KEY", 1);
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("test", nullptr, sysEventCreator);
    std::regex expValue(R"~(\{"domain_":"DEMO","name_":"EVENT_NAME","type_":1,"time_":\d+,"tz_":"[\+\-]\d+",)~"
        R"~("pid_":\d+,"tid_":\d+,"uid_":\d+,"id_":"\d+","KEY":1\})~");
    std::cout << "size=" << sysEvent->AsJsonStr().size() << ", jsonStr:" << sysEvent->AsJsonStr() << std::endl;
    std::smatch baseMatch;
    auto eventJsonStr = sysEvent->AsJsonStr();
    bool isMatch = std::regex_match(eventJsonStr, baseMatch, expValue);
    ASSERT_TRUE(isMatch);
}

/**
 * @tc.name: TestSendIntVectorType002
 * @tc.desc: create base sys event
 * @tc.type: FUNC
 * @tc.require: AR000FT2Q2 AR000FT2Q3
 */
HWTEST_F(SysEventTest, TestSendIntVectorType002, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. create base sys event
     */
    std::cout << "TestSendIntVectorType002 test vector<int> type" << std::endl;
    SysEventCreator sysEventCreator("DEMO", "EVENT_NAME", SysEventCreator::FAULT);
    std::vector<int> values = {1, 2, 3};
    sysEventCreator.SetKeyValue("KEY", values);
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("test", nullptr, sysEventCreator);
    std::regex expValue(R"~(\{"domain_":"DEMO","name_":"EVENT_NAME","type_":1,"time_":\d+,"tz_":"[\+\-]\d+",)~"
        R"~("pid_":\d+,"tid_":\d+,"uid_":\d+,"id_":"\d+","KEY":\[1,2,3\]\})~");
    std::cout << "size=" << sysEvent->AsJsonStr().size() << ", jsonStr:" << sysEvent->AsJsonStr() << std::endl;
    std::smatch baseMatch;
    auto eventJsonStr = sysEvent->AsJsonStr();
    bool isMatch = std::regex_match(eventJsonStr, baseMatch, expValue);
    ASSERT_TRUE(isMatch);
}

/**
 * @tc.name: TestSysEventValueParse001
 * @tc.desc: Parse customized value as int64_t type from sys event
 * @tc.type: FUNC
 * @tc.require: issueI7V7ZA
 */
HWTEST_F(SysEventTest, TestSysEventValueParse001, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. create base sys event
     */
    std::string jsonStr = R"~({"domain_":"DEMO","name_":"VALUE_PARSE001","type_":1,"tz_":"+0800","time_":1620271291188,
        "pid_":6527,"tid_":6527,"traceid_":"f0ed5160bb2df4b","spanid_":"10","pspanid_":"20","trace_flag_":4,)~";
    jsonStr.append(R"~("INT_VAL1":-1,"INT_VAL2":1,"INT_VAL3":)~");
    jsonStr.append(std::to_string(std::numeric_limits<int64_t>::min()));
    jsonStr.append(R"~(,"INT_VAL4":)~");
    jsonStr.append(std::to_string(std::numeric_limits<int64_t>::max()));
    jsonStr.append(R"~(,"UINT_VAL1":10,"UINT_VAL2":1000,"UINT_VAL3":)~");
    jsonStr.append(std::to_string(std::numeric_limits<uint64_t>::min()));
    jsonStr.append(R"~(,"UINT_VAL4":)~");
    jsonStr.append(std::to_string(std::numeric_limits<uint64_t>::max()));
    jsonStr.append("}");
    auto sysEvent = std::make_shared<SysEvent>("SysEventSource", nullptr, jsonStr);
    int64_t dest = sysEvent->GetEventIntValue("INT_VAL1");
    ASSERT_EQ(dest, -1); // test value
    dest = sysEvent->GetEventIntValue("INT_VAL2");
    ASSERT_EQ(dest, 1); // test value
    dest = sysEvent->GetEventIntValue("INT_VAL3");
    ASSERT_EQ(dest, std::numeric_limits<int64_t>::min());
    dest = sysEvent->GetEventIntValue("INT_VAL4");
    ASSERT_EQ(dest, std::numeric_limits<int64_t>::max());
    dest = sysEvent->GetEventIntValue("UINT_VAL1");
    ASSERT_EQ(dest, 10); // test value
    dest = sysEvent->GetEventIntValue("UINT_VAL2");
    ASSERT_EQ(dest, 1000); // test value
    dest = sysEvent->GetEventIntValue("UINT_VAL3");
    ASSERT_EQ(dest, std::numeric_limits<uint64_t>::min());
    dest = sysEvent->GetEventIntValue("UINT_VAL4");
    ASSERT_EQ(dest, 0); // test value
}

/**
 * @tc.name: TestSysEventValueParse002
 * @tc.desc: Parse customized value as uint64_t type from sys event
 * @tc.type: FUNC
 * @tc.require: issueI7V7ZA
 */
HWTEST_F(SysEventTest, TestSysEventValueParse002, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. create base sys event
     */
    std::string jsonStr = R"~({"domain_":"DEMO","name_":"VALUE_PARSE001","type_":1,"tz_":"+0800","time_":1620271291188,
        "pid_":6527,"tid_":6527,"traceid_":"f0ed5160bb2df4b","spanid_":"10","pspanid_":"20","trace_flag_":4,)~";
    jsonStr.append(R"~("INT_VAL1":-1,"INT_VAL2":1,"INT_VAL3":)~");
    jsonStr.append(std::to_string(std::numeric_limits<int64_t>::min()));
    jsonStr.append(R"~(,"INT_VAL4":)~");
    jsonStr.append(std::to_string(std::numeric_limits<int64_t>::max()));
    jsonStr.append(R"~(,"UINT_VAL1":10,"UINT_VAL2":1000,"UINT_VAL3":)~");
    jsonStr.append(std::to_string(std::numeric_limits<uint64_t>::min()));
    jsonStr.append(R"~(,"UINT_VAL4":)~");
    jsonStr.append(std::to_string(std::numeric_limits<uint64_t>::max()));
    jsonStr.append("}");
    auto sysEvent = std::make_shared<SysEvent>("SysEventSource", nullptr, jsonStr);
    uint64_t dest = sysEvent->GetEventUintValue("INT_VAL1");
    ASSERT_EQ(dest, 0); // test value
    dest = sysEvent->GetEventUintValue("INT_VAL2");
    ASSERT_EQ(dest, 1); // test value
    dest = sysEvent->GetEventUintValue("INT_VAL3");
    ASSERT_EQ(dest, 0); // test value
    dest = sysEvent->GetEventUintValue("INT_VAL4");
    ASSERT_EQ(dest, std::numeric_limits<int64_t>::max());
    dest = sysEvent->GetEventUintValue("UINT_VAL1");
    ASSERT_EQ(dest, 10); // test value
    dest = sysEvent->GetEventUintValue("UINT_VAL2");
    ASSERT_EQ(dest, 1000); // test value
    dest = sysEvent->GetEventUintValue("UINT_VAL3");
    ASSERT_EQ(dest, std::numeric_limits<uint64_t>::min());
    dest = sysEvent->GetEventUintValue("UINT_VAL4");
    ASSERT_EQ(dest, std::numeric_limits<uint64_t>::max());
}

/**
 * @tc.name: TestSysEventValueParse003
 * @tc.desc: Parse customized value as double type from sys event
 * @tc.type: FUNC
 * @tc.require: issueI7V7ZA
 */
HWTEST_F(SysEventTest, TestSysEventValueParse003, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. create base sys event
     */
    std::string jsonStr = R"~({"domain_":"DEMO","name_":"VALUE_PARSE001","type_":1,"tz_":"+0800","time_":1620271291188,
        "pid_":6527,"tid_":6527,"traceid_":"f0ed5160bb2df4b","spanid_":"10","pspanid_":"20","trace_flag_":4,
        "FLOAT_VAL1":-1.0,"FLOAT_VAL2":1.0})~";
    auto sysEvent = std::make_shared<SysEvent>("SysEventSource", nullptr, jsonStr);
    double dest = sysEvent->GetEventDoubleValue("FLOAT_VAL1");
    ASSERT_EQ(dest, -1); // test value
    dest = sysEvent->GetEventDoubleValue("FLOAT_VAL2");
    ASSERT_EQ(dest, 1); // test value
}

/**
 * @tc.name: TestSysEventValueParse004
 * @tc.desc: Parse base value as int64_t type from sys event
 * @tc.type: FUNC
 * @tc.require: issueI7V7ZA
 */
HWTEST_F(SysEventTest, TestSysEventValueParse004, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. create base sys event
     */
    std::string jsonStr = R"~({"domain_":"DEMO","name_":"VALUE_PARSE001","type_":1,"tz_":"+0800","time_":1620271291188,
        "pid_":6527,"tid_":-6527,"traceid_":"f0ed5160bb2df4b","spanid_":"10","pspanid_":"20","trace_flag_":4,
        "FLOAT_VAL1":-1.0,"FLOAT_VAL2":1.0,)~";
    jsonStr.append(R"~("FLOAT_VAL3":)~");
    jsonStr.append(std::to_string((static_cast<double>(std::numeric_limits<int64_t>::min()) - 1)));
    jsonStr.append(R"~("FLOAT_VAL4":)~");
    jsonStr.append(std::to_string((static_cast<double>(std::numeric_limits<int64_t>::max()) + 1)));
    jsonStr.append("}");
    auto sysEvent = std::make_shared<SysEvent>("SysEventSource", nullptr, jsonStr);
    int64_t dest = sysEvent->GetEventIntValue("domain_");
    ASSERT_EQ(dest, 0); // test value
    dest = sysEvent->GetEventIntValue("type_");
    ASSERT_EQ(dest, 1); // test value
    dest = sysEvent->GetEventIntValue("time_");
    ASSERT_EQ(dest, 1620271291188); // test value
    dest = sysEvent->GetEventIntValue("tz_");
    ASSERT_EQ(dest, 27); // test value
    dest = sysEvent->GetEventIntValue("pid_");
    ASSERT_EQ(dest, 6527); // test value
    dest = sysEvent->GetEventIntValue("traceid_");
    ASSERT_EQ(dest, 1085038850905136971); // test value: f0ed5160bb2df4b
    dest = sysEvent->GetEventIntValue("trace_flag_");
    ASSERT_EQ(dest, 4); // test value
    dest = sysEvent->GetEventIntValue("FLOAT_VAL1");
    ASSERT_EQ(dest, -1); // test value
    dest = sysEvent->GetEventIntValue("FLOAT_VAL2");
    ASSERT_EQ(dest, 1); // test value
    dest = sysEvent->GetEventIntValue("FLOAT_VAL3");
    ASSERT_EQ(dest, 0); // test value
    dest = sysEvent->GetEventIntValue("FLOAT_VAL4");
    ASSERT_EQ(dest, 0); // test value
}

/**
 * @tc.name: TestSysEventValueParse005
 * @tc.desc: Parse base value as uint64_t type from sys event
 * @tc.type: FUNC
 * @tc.require: issueI7V7ZA
 */
HWTEST_F(SysEventTest, TestSysEventValueParse005, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. create base sys event
     */
    std::string jsonStr = R"~({"domain_":"DEMO","name_":"VALUE_PARSE001","type_":1,"tz_":"+0800","time_":1620271291188,
        "pid_":6527,"tid_":6527,"traceid_":"f0ed5160bb2df4b","spanid_":"10","pspanid_":"20","trace_flag_":4,
        "FLOAT_VAL1":-1.0,"FLOAT_VAL2":1.0,)~";
    jsonStr.append(R"~("FLOAT_VAL3":)~");
    jsonStr.append(std::to_string((static_cast<double>(std::numeric_limits<uint64_t>::min()) - 1)));
    jsonStr.append(R"~("FLOAT_VAL4":)~");
    jsonStr.append(std::to_string((static_cast<double>(std::numeric_limits<uint64_t>::max()) + 1)));
    jsonStr.append("}");
    auto sysEvent = std::make_shared<SysEvent>("SysEventSource", nullptr, jsonStr);
    uint64_t dest = sysEvent->GetEventUintValue("domain_");
    ASSERT_EQ(dest, 0); // test value
    dest = sysEvent->GetEventUintValue("type_");
    ASSERT_EQ(dest, 1); // test value
    dest = sysEvent->GetEventUintValue("time_");
    ASSERT_EQ(dest, 1620271291188); // test value
    dest = sysEvent->GetEventUintValue("tz_");
    ASSERT_EQ(dest, 27); // test value
    dest = sysEvent->GetEventUintValue("pid_");
    ASSERT_EQ(dest, 6527); // test value
    dest = sysEvent->GetEventUintValue("traceid_");
    ASSERT_EQ(dest, 1085038850905136971); // test value: f0ed5160bb2df4b
    dest = sysEvent->GetEventUintValue("trace_flag_");
    ASSERT_EQ(dest, 4); // test value
    dest = sysEvent->GetEventUintValue("FLOAT_VAL1");
    ASSERT_EQ(dest, 0); // test value
    dest = sysEvent->GetEventUintValue("FLOAT_VAL2");
    ASSERT_EQ(dest, 1); // test value
    dest = sysEvent->GetEventUintValue("FLOAT_VAL3");
    ASSERT_EQ(dest, 0); // test value
    dest = sysEvent->GetEventUintValue("FLOAT_VAL4");
    ASSERT_EQ(dest, 0); // test value
}

/**
 * @tc.name: TestSysEventValueParse006
 * @tc.desc: Parse base value as double type from sys event
 * @tc.type: FUNC
 * @tc.require: issueI7V7ZA
 */
HWTEST_F(SysEventTest, TestSysEventValueParse006, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. create base sys event
     */
    std::string jsonStr = R"~({"domain_":"DEMO","name_":"VALUE_PARSE001","type_":1,"tz_":"+0800","time_":1620271291188,
        "pid_":6527,"tid_":6527,"traceid_":"f0ed5160bb2df4b","spanid_":"10","pspanid_":"20","trace_flag_":4,
        "FLOAT_VAL1":-1.0,"FLOAT_VAL2":1.0})~";
    auto sysEvent = std::make_shared<SysEvent>("SysEventSource", nullptr, jsonStr);
    double dest = sysEvent->GetEventDoubleValue("domain_");
    ASSERT_EQ(dest, 0.0); // test value
    dest = sysEvent->GetEventDoubleValue("type_");
    ASSERT_EQ(dest, 1); // test value
    dest = sysEvent->GetEventDoubleValue("time_");
    ASSERT_EQ(dest, 1620271291188); // test value
    dest = sysEvent->GetEventDoubleValue("tz_");
    ASSERT_EQ(dest, 27); // test value
    dest = sysEvent->GetEventDoubleValue("pid_");
    ASSERT_EQ(dest, 6527); // test value
    dest = sysEvent->GetEventDoubleValue("trace_flag_");
    ASSERT_EQ(dest, 4); // test value
    dest = sysEvent->GetEventDoubleValue("FLOAT_VAL1");
    ASSERT_EQ(dest, -1); // test value
    dest = sysEvent->GetEventDoubleValue("FLOAT_VAL2");
    ASSERT_EQ(dest, 1); // test value
}

/**
 * @tc.name: TestSysEventValueParse007
 * @tc.desc: Parse base value as string type from sys event
 * @tc.type: FUNC
 * @tc.require: issueI7V7ZA
 */
HWTEST_F(SysEventTest, TestSysEventValueParse007, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. create base sys event
     */
    std::string jsonStr = R"~({"domain_":"DEMO","name_":"VALUE_PARSE001","type_":1,"tz_":"+0800","time_":1620271291188,
        "pid_":6527,"tid_":6527,"traceid_":"f0ed5160bb2df4b","spanid_":"10","pspanid_":"20","trace_flag_":4,
        "FLOAT_VAL1":-1.0,"FLOAT_VAL2":1.0})~";
    auto sysEvent = std::make_shared<SysEvent>("SysEventSource", nullptr, jsonStr);
    std::string dest = sysEvent->GetEventValue("domain_");
    ASSERT_EQ(dest, "DEMO"); // test value
    dest = sysEvent->GetEventValue("type_");
    ASSERT_EQ(dest, ""); // test value
    dest = sysEvent->GetEventValue("time_");
    ASSERT_EQ(dest, ""); // test value
    dest = sysEvent->GetEventValue("tz_");
    ASSERT_EQ(dest, "+0800"); // test value
    dest = sysEvent->GetEventValue("pid_");
    ASSERT_EQ(dest, ""); // test value
    dest = sysEvent->GetEventValue("traceid_");
    ASSERT_EQ(dest, "f0ed5160bb2df4b"); // test value
    dest = sysEvent->GetEventValue("trace_flag_");
    ASSERT_EQ(dest, ""); // test value
    dest = sysEvent->GetEventValue("FLOAT_VAL1");
    ASSERT_EQ(dest, ""); // test value
    dest = sysEvent->GetEventValue("FLOAT_VAL2");
    ASSERT_EQ(dest, ""); // test value
}
} // HiviewDFX
} // OHOS