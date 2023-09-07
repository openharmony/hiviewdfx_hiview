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
#include <iostream>
#include "trace_collector.h"
#include "trace_manager.h"
#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::UCollectUtil;

TraceManager traceManager;

class TraceCollectorTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
};

/**
 * @tc.name: TraceCollectorTest001
 * @tc.desc: used to test TraceCollector for xperf dump
 * @tc.type: FUNC
*/
HWTEST_F(TraceCollectorTest, TraceCollectorTest001, TestSize.Level1)
{
    const std::vector<std::string> tagGroups = {"scene_performance"};
    TraceCollector::Caller caller = TraceCollector::Caller::XPERF;
    std::shared_ptr<TraceCollector> collector = TraceCollector::Create();
    ASSERT_TRUE(traceManager.OpenSnapshotTrace(tagGroups) == 0);
    sleep(5);
    std::cout << "caller : " << caller << std::endl;
    CollectResult<std::vector<std::string>> resultDumpTrace = collector->DumpTrace(caller);
    std::vector<std::string> items = resultDumpTrace.data;
    std::cout << "collect DumpTrace result size : " << items.size() << std::endl;
    for (auto it = items.begin(); it != items.end(); it++) {
        std::cout << "collect DumpTrace result path : " << it->c_str() << std::endl;
    }
    ASSERT_TRUE(resultDumpTrace.retCode == UcError::SUCCESS);
    ASSERT_TRUE(resultDumpTrace.data.size() > 0);
    ASSERT_TRUE(traceManager.CloseTrace() == 0);
}

/**
 * @tc.name: TraceCollectorTest002
 * @tc.desc: used to test TraceCollector for xpower dump
 * @tc.type: FUNC
*/
HWTEST_F(TraceCollectorTest, TraceCollectorTest002, TestSize.Level1)
{
    const std::vector<std::string> tagGroups = {"scene_performance"};
    TraceCollector::Caller caller = TraceCollector::Caller::XPOWER;
    std::shared_ptr<TraceCollector> collector = TraceCollector::Create();
    ASSERT_TRUE(traceManager.OpenSnapshotTrace(tagGroups) == 0);
    sleep(5);
    std::cout << "caller : " << caller << std::endl;
    CollectResult<std::vector<std::string>> resultDumpTrace = collector->DumpTrace(caller);
    std::vector<std::string> items = resultDumpTrace.data;
    std::cout << "collect DumpTrace result size : " << items.size() << std::endl;
    for (auto it = items.begin(); it != items.end(); it++) {
        std::cout << "collect DumpTrace result path : " << it->c_str() << std::endl;
    }
    ASSERT_TRUE(resultDumpTrace.retCode == UcError::SUCCESS);
    ASSERT_TRUE(resultDumpTrace.data.size() > 0);
    ASSERT_TRUE(traceManager.CloseTrace() == 0);
}

/**
 * @tc.name: TraceCollectorTest003
 * @tc.desc: used to test TraceCollector for reliability dump
 * @tc.type: FUNC
*/
HWTEST_F(TraceCollectorTest, TraceCollectorTest003, TestSize.Level1)
{
    const std::vector<std::string> tagGroups = {"scene_performance"};
    std::shared_ptr<TraceCollector> collector = TraceCollector::Create();
    TraceCollector::Caller caller = TraceCollector::Caller::RELIABILITY;
    ASSERT_TRUE(traceManager.OpenSnapshotTrace(tagGroups) == 0);
    sleep(5);
    std::cout << "caller : " << caller << std::endl;
    CollectResult<std::vector<std::string>> resultDumpTrace = collector->DumpTrace(caller);
    std::vector<std::string> items = resultDumpTrace.data;
    std::cout << "collect DumpTrace result size : " << items.size() << std::endl;
    for (auto it = items.begin(); it != items.end(); it++) {
        std::cout << "collect DumpTrace result path : " << it->c_str() << std::endl;
    }
    ASSERT_TRUE(resultDumpTrace.retCode == UcError::SUCCESS);
    ASSERT_TRUE(resultDumpTrace.data.size() > 0);
    ASSERT_TRUE(traceManager.CloseTrace() == 0);
}

/**
 * @tc.name: TraceCollectorTest004
 * @tc.desc: used to test TraceCollector for command
 * @tc.type: FUNC
*/
HWTEST_F(TraceCollectorTest, TraceCollectorTest004, TestSize.Level1)
{
    const std::string args = "tags:sched clockType:boot bufferSize:1024 overwrite:1";
    std::shared_ptr<TraceCollector> collector = TraceCollector::Create();
    ASSERT_TRUE(traceManager.OpenRecordingTrace(args) == 0);

    CollectResult<int32_t> resultTraceOn = collector->TraceOn();
    std::cout << "collect TraceOn result " << resultTraceOn.retCode << std::endl;
    ASSERT_TRUE(resultTraceOn.retCode == UcError::SUCCESS);
    sleep(10);
    CollectResult<std::vector<std::string>> resultTraceOff = collector->TraceOff();
    std::vector<std::string> items = resultTraceOff.data;
    for (auto it = items.begin(); it != items.end(); it++) {
        std::cout << "collect TraceOff result path : " << it->c_str() << std::endl;
    }
    ASSERT_TRUE(resultTraceOff.retCode == UcError::SUCCESS);
    ASSERT_TRUE(resultTraceOff.data.size() > 0);
    ASSERT_TRUE(traceManager.CloseTrace() == 0);
}

/**
 * @tc.name: TraceCollectorTest005
 * @tc.desc: used to test TraceCollector for BetaClub dump
 * @tc.type: FUNC
*/
HWTEST_F(TraceCollectorTest, TraceCollectorTest005, TestSize.Level1)
{
    const std::vector<std::string> tagGroups = {"scene_performance"};
    TraceCollector::Caller caller = TraceCollector::Caller::BETACLUB;
    std::shared_ptr<TraceCollector> collector = TraceCollector::Create();
    ASSERT_TRUE(traceManager.OpenSnapshotTrace(tagGroups) == 0);
    sleep(5);
    std::cout << "caller : " << caller << std::endl;
    CollectResult<std::vector<std::string>> resultDumpTrace = collector->DumpTrace(caller);
    std::vector<std::string> items = resultDumpTrace.data;
    std::cout << "collect DumpTrace result size : " << items.size() << std::endl;
    for (auto it = items.begin(); it != items.end(); it++) {
        std::cout << "collect DumpTrace result path : " << it->c_str() << std::endl;
    }
    ASSERT_TRUE(resultDumpTrace.retCode == UcError::SUCCESS);
    ASSERT_TRUE(resultDumpTrace.data.size() > 0);
    ASSERT_TRUE(traceManager.CloseTrace() == 0);
}

/**
 * @tc.name: TraceCollectorTest006
 * @tc.desc: used to test TraceCollector for Other dump
 * @tc.type: FUNC
*/
HWTEST_F(TraceCollectorTest, TraceCollectorTest006, TestSize.Level1)
{
    const std::vector<std::string> tagGroups = {"scene_performance"};
    TraceCollector::Caller caller = TraceCollector::Caller::OTHER;
    std::shared_ptr<TraceCollector> collector = TraceCollector::Create();
    ASSERT_TRUE(traceManager.OpenSnapshotTrace(tagGroups) == 0);
    sleep(5);
    std::cout << "caller : " << caller << std::endl;
    CollectResult<std::vector<std::string>> resultDumpTrace = collector->DumpTrace(caller);
    std::vector<std::string> items = resultDumpTrace.data;
    std::cout << "collect DumpTrace result size : " << items.size() << std::endl;
    for (auto it = items.begin(); it != items.end(); it++) {
        std::cout << "collect DumpTrace result path : " << it->c_str() << std::endl;
    }
    ASSERT_TRUE(resultDumpTrace.retCode == UcError::SUCCESS);
    ASSERT_TRUE(resultDumpTrace.data.size() > 0);
    ASSERT_TRUE(traceManager.CloseTrace() == 0);
}
