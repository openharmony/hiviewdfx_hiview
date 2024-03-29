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
#ifdef HAS_HIPERF
#include <iostream>

#include "perf_collector.h"
#include "file_util.h"
#include "time_util.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace OHOS::HiviewDFX::UCollect;

class PerfCollectorTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
};

/**
 * @tc.name: PerfCollectorTest001
 * @tc.desc: used to test PerfCollector.StartPerf
 * @tc.type: FUNC
*/
HWTEST_F(PerfCollectorTest, PerfCollectorTest001, TestSize.Level1)
{
    std::shared_ptr<UCollectUtil::PerfCollector> perfCollector = UCollectUtil::PerfCollector::Create();
    vector<pid_t> selectPids = {getpid()};
    std::string filedir = "/data/local/tmp/";
    std::string filename = "hiperf-";
    filename += TimeUtil::TimestampFormatToDate(TimeUtil::GetMilliseconds() / TimeUtil::SEC_TO_MILLISEC,
    "%Y%m%d%H%M%S");
    filename += ".data";
    perfCollector->SetOutputFilename(filename);
    perfCollector->SetSelectPids(selectPids);
    perfCollector->SetTimeStopSec(3);
    CollectResult<bool> data = perfCollector->StartPerf(filedir);
    std::cout << "collect perf data result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: PerfCollectorTest002
 * @tc.desc: used to test PerfCollector.StartPerf
 * @tc.type: FUNC
*/
HWTEST_F(PerfCollectorTest, PerfCollectorTest002, TestSize.Level1)
{
    std::shared_ptr<UCollectUtil::PerfCollector> perfCollector = UCollectUtil::PerfCollector::Create();
    vector<pid_t> selectPids = {getpid()};
    std::string filedir = "/data/local/tmp/";
    std::string filename = "hiperf-";
    filename += TimeUtil::TimestampFormatToDate(TimeUtil::GetMilliseconds() / TimeUtil::SEC_TO_MILLISEC,
    "%Y%m%d%H%M%S");
    filename += ".data";
    std::string filepath = filedir + filename;
    perfCollector->SetOutputFilename(filename);
    perfCollector->SetSelectPids(selectPids);
    perfCollector->SetTimeStopSec(3);
    CollectResult<bool> data = perfCollector->StartPerf(filedir);
    std::cout << "collect perf data result" << data.retCode << std::endl;
    ASSERT_EQ(FileUtil::FileExists(filepath), true);
}

/**
 * @tc.name: PerfCollectorTest003
 * @tc.desc: used to test PerfCollector.SetTargetSystemWide
 * @tc.type: FUNC
*/
HWTEST_F(PerfCollectorTest, PerfCollectorTest003, TestSize.Level1)
{
    std::shared_ptr<UCollectUtil::PerfCollector> perfCollector = UCollectUtil::PerfCollector::Create();
    vector<pid_t> selectPids = {getpid()};
    std::string filedir = "/data/local/tmp/";
    std::string filename = "hiperf-";
    filename += TimeUtil::TimestampFormatToDate(TimeUtil::GetMilliseconds() / TimeUtil::SEC_TO_MILLISEC,
    "%Y%m%d%H%M%S");
    filename += ".data";
    std::string filepath = filedir + filename;
    perfCollector->SetOutputFilename(filename);
    perfCollector->SetTargetSystemWide(true);
    perfCollector->SetCallGraph("fp");
    std::vector<std::string> = {"hw-cpu-cycles", "hw-instructions"};
    perfCollector->SetSelectEvents(selectEvents);
    CollectResult<bool> data = perfCollector->Prepare(filedir);
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
    TimeUtil::Sleep(1);
    data = perfCollector->StartRun();
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
    TimeUtil::Sleep(1);
    data = perfCollector->Stop();
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
    std::cout << "collect perf data result" << data.retCode << std::endl;
    ASSERT_EQ(FileUtil::FileExists(filepath), true);
}

/**
 * @tc.name: PerfCollectorTest004
 * @tc.desc: used to test PerfCollector.SetCpuPercent
 * @tc.type: FUNC
*/
HWTEST_F(PerfCollectorTest, PerfCollectorTest004, TestSize.Level1)
{
    std::shared_ptr<UCollectUtil::PerfCollector> perfCollector = UCollectUtil::PerfCollector::Create();
    vector<pid_t> selectPids = {getpid()};
    std::string filedir = "/data/local/tmp/";
    std::string filename = "hiperf-";
    filename += TimeUtil::TimestampFormatToDate(TimeUtil::GetMilliseconds() / TimeUtil::SEC_TO_MILLISEC,
    "%Y%m%d%H%M%S");
    filename += ".data";
    std::string filepath = filedir + filename;
    perfCollector->SetOutputFilename(filename);
    perfCollector->SetSelectPids(selectPids);
    perfCollector->SetFrequency(100);
    perfCollector->SetTimeStopSec(2);
    perfCollector->SetCpuPercent(100);
    CollectResult<bool> data = perfCollector->StartPerf(filedir);
    std::cout << "collect perf data result" << data.retCode << std::endl;
    ASSERT_EQ(FileUtil::FileExists(filepath), true);
}
#endif // HAS_HIPERF
