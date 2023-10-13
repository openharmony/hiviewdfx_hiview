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
#include <climits>
#include <iostream>
#include <unistd.h>

#include "cpu_collector.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace OHOS::HiviewDFX::UCollect;

class CpuCollectorTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
};

/**
 * @tc.name: CpuCollectorTest001
 * @tc.desc: used to test CpuCollector.CollectSysCpuLoad
 * @tc.type: FUNC
*/
HWTEST_F(CpuCollectorTest, CpuCollectorTest001, TestSize.Level1)
{
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    CollectResult<SysCpuLoad> data = collector->CollectSysCpuLoad();
    std::cout << "collect system cpu load result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: CpuCollectorTest002
 * @tc.desc: used to test CpuCollector.CollectSysCpuUsage
 * @tc.type: FUNC
*/
HWTEST_F(CpuCollectorTest, CpuCollectorTest002, TestSize.Level1)
{
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    CollectResult<SysCpuUsage> data = collector->CollectSysCpuUsage();
    std::cout << "collect system cpu usage result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: CpuCollectorTest003
 * @tc.desc: used to test CpuCollector.CollectProcessCpuUsage
 * @tc.type: FUNC
*/
HWTEST_F(CpuCollectorTest, CpuCollectorTest003, TestSize.Level1)
{
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    CollectResult<ProcessCpuUsage> data = collector->CollectProcessCpuUsage(1000);
    std::cout << "collect process cpu usage result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: CpuCollectorTest004
 * @tc.desc: used to test CpuCollector.CollectProcessCpuLoad
 * @tc.type: FUNC
*/
HWTEST_F(CpuCollectorTest, CpuCollectorTest004, TestSize.Level1)
{
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    CollectResult<ProcessCpuLoad> data = collector->CollectProcessCpuLoad(1000);
    std::cout << "collect process cpu load result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: CpuCollectorTest005
 * @tc.desc: used to test CpuCollector.CollectCpuFreqStat
 * @tc.type: FUNC
*/
HWTEST_F(CpuCollectorTest, CpuCollectorTest005, TestSize.Level1)
{
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    CollectResult<CpuFreqStat> data = collector->CollectCpuFreqStat();
    std::cout << "collect system cpu freq stat result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: CpuCollectorTest006
 * @tc.desc: used to test CpuCollector.CollectCpuFrequency
 * @tc.type: FUNC
*/
HWTEST_F(CpuCollectorTest, CpuCollectorTest006, TestSize.Level1)
{
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    CollectResult<std::vector<CpuFreq>> data = collector->CollectCpuFrequency();
    std::cout << "collect system cpu frequency result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: CpuCollectorTest007
 * @tc.desc: used to test CpuCollector.CollectProcessCpuUsages
 * @tc.type: FUNC
*/
HWTEST_F(CpuCollectorTest, CpuCollectorTest007, TestSize.Level1)
{
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    CollectResult<std::vector<ProcessCpuUsage>> data = collector->CollectProcessCpuUsages();
    std::cout << "collect process cpu usages result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: CpuCollectorTest008
 * @tc.desc: used to test CpuCollector.CollectProcessCpuLoads
 * @tc.type: FUNC
*/
HWTEST_F(CpuCollectorTest, CpuCollectorTest008, TestSize.Level1)
{
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    CollectResult<std::vector<ProcessCpuLoad>> data = collector->CollectProcessCpuLoads();
    std::cout << "collect process cpu loads result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: CpuCollectorTest009
 * @tc.desc: used to test CpuCollector.CollectProcessCpuStatInfo
 * @tc.type: FUNC
*/
HWTEST_F(CpuCollectorTest, CpuCollectorTest009, TestSize.Level1)
{
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    constexpr int initPid = 1;
    auto collectResult = collector->CollectProcessCpuStatInfo(initPid);
    ASSERT_TRUE(collectResult.retCode == UcError::SUCCESS);
    ASSERT_GT(collectResult.data.startTime, 0);
    ASSERT_GT(collectResult.data.endTime, 0);
    ASSERT_EQ(collectResult.data.pid, initPid);
    ASSERT_FALSE(collectResult.data.procName.empty());

    sleep(1); // 1s
    auto nextCollectResult = collector->CollectProcessCpuStatInfo(initPid);
    ASSERT_TRUE(nextCollectResult.retCode == UcError::SUCCESS);
    ASSERT_EQ(nextCollectResult.data.startTime, collectResult.data.startTime);
    ASSERT_GT(nextCollectResult.data.endTime, collectResult.data.endTime);
}

/**
 * @tc.name: CpuCollectorTest010
 * @tc.desc: used to test CpuCollector.CollectProcessCpuStatInfo
 * @tc.type: FUNC
*/
HWTEST_F(CpuCollectorTest, CpuCollectorTest010, TestSize.Level1)
{
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    constexpr int notExistPid = INT_MAX;
    auto collectResult = collector->CollectProcessCpuStatInfo(notExistPid);
    ASSERT_TRUE(collectResult.retCode != UcError::SUCCESS);
}

/**
 * @tc.name: CpuCollectorTest011
 * @tc.desc: used to test CpuCollector.CollectProcessCpuStatInfo
 * @tc.type: FUNC
*/
HWTEST_F(CpuCollectorTest, CpuCollectorTest011, TestSize.Level1)
{
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    constexpr int invalidPid = -1;
    auto collectResult = collector->CollectProcessCpuStatInfo(invalidPid);
    ASSERT_TRUE(collectResult.retCode != UcError::SUCCESS);
}

/**
 * @tc.name: CpuCollectorTest012
 * @tc.desc: used to test CpuCollector.CollectProcessCpuStatInfos
 * @tc.type: FUNC
*/
HWTEST_F(CpuCollectorTest, CpuCollectorTest012, TestSize.Level1)
{
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    auto collectResult = collector->CollectProcessCpuStatInfos(true);
    ASSERT_TRUE(collectResult.retCode == UcError::SUCCESS);
    ASSERT_FALSE(collectResult.data.empty());

    sleep(1); // 1s
    auto nextCollectResult = collector->CollectProcessCpuStatInfos();
    ASSERT_TRUE(nextCollectResult.retCode == UcError::SUCCESS);
    ASSERT_FALSE(nextCollectResult.data.empty());

    std::cout << "next collection startTime=" << nextCollectResult.data[0].startTime << std::endl;
    std::cout << "next collection endTime=" << nextCollectResult.data[0].endTime << std::endl;
    ASSERT_EQ(nextCollectResult.data[0].startTime, collectResult.data[0].endTime);
    ASSERT_GT(nextCollectResult.data[0].endTime, nextCollectResult.data[0].startTime);
}

/**
 * @tc.name: CpuCollectorTest013
 * @tc.desc: used to test CpuCollector.CollectProcessCpuStatInfos
 * @tc.type: FUNC
*/
HWTEST_F(CpuCollectorTest, CpuCollectorTest013, TestSize.Level1)
{
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    auto collectResult = collector->CollectProcessCpuStatInfos(true);
    ASSERT_TRUE(collectResult.retCode == UcError::SUCCESS);
    ASSERT_FALSE(collectResult.data.empty());

    sleep(1); // 1s
    auto nextCollectResult = collector->CollectProcessCpuStatInfos(true);
    ASSERT_TRUE(nextCollectResult.retCode == UcError::SUCCESS);
    ASSERT_FALSE(nextCollectResult.data.empty());

    ASSERT_GT(nextCollectResult.data[0].startTime, collectResult.data[0].startTime);
    ASSERT_GT(nextCollectResult.data[0].endTime, collectResult.data[0].endTime);
}

/**
 * @tc.name: CpuCollectorTest014
 * @tc.desc: used to test CpuCollector.CollectProcessCpuStatInfos
 * @tc.type: FUNC
*/
HWTEST_F(CpuCollectorTest, CpuCollectorTest014, TestSize.Level1)
{
    std::shared_ptr<CpuCollector> collector = CpuCollector::Create();
    auto firstCollectResult = collector->CollectProcessCpuStatInfos(false);
    ASSERT_TRUE(firstCollectResult.retCode == UcError::SUCCESS);
    ASSERT_FALSE(firstCollectResult.data.empty());

    sleep(1); // 1s
    auto secondCollectResult = collector->CollectProcessCpuStatInfos(false);
    ASSERT_TRUE(secondCollectResult.retCode == UcError::SUCCESS);
    ASSERT_FALSE(secondCollectResult.data.empty());

    ASSERT_EQ(firstCollectResult.data[0].startTime, secondCollectResult.data[0].startTime);
    ASSERT_LT(firstCollectResult.data[0].endTime, secondCollectResult.data[0].endTime);
}
