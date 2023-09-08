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