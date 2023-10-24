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

#include "memory_collector.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace OHOS::HiviewDFX::UCollect;

class MemoryCollectorTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
};

/**
 * @tc.name: MemoryCollectorTest001
 * @tc.desc: used to test MemoryCollector.CollectProcessMemory
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest001, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<ProcessMemory> data = collector->CollectProcessMemory(1000);
    std::cout << "collect process memory result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: MemoryCollectorTest002
 * @tc.desc: used to test MemoryCollector.CollectSysMemory
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest002, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<SysMemory> data = collector->CollectSysMemory();
    std::cout << "collect system memory result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: MemoryCollectorTest003
 * @tc.desc: used to test MemoryCollector.CollectRawMemInfo
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest003, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::string> data = collector->CollectRawMemInfo();
    std::cout << "collect raw memory info result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: MemoryCollectorTest004
 * @tc.desc: used to test MemoryCollector.CollectAllProcessMemory
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest004, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::vector<ProcessMemory>> data = collector->CollectAllProcessMemory();
    std::cout << "collect all process memory result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: MemoryCollectorTest005
 * @tc.desc: used to test MemoryCollector.ExportAllProcessMemory
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest005, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::string> data = collector->ExportAllProcessMemory();
    std::cout << "export all process memory result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: MemoryCollectorTest006
 * @tc.desc: used to test MemoryCollector.CollectRawSlabInfo
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest006, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::string> data = collector->CollectRawSlabInfo();
    std::cout << "collect raw slab info result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: MemoryCollectorTest007
 * @tc.desc: used to test MemoryCollector.CollectRawPageTypeInfo
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest007, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::string> data = collector->CollectRawPageTypeInfo();
    std::cout << "collect raw pagetype info result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: MemoryCollectorTest008
 * @tc.desc: used to test MemoryCollector.CollectRawDMA
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest008, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::string> data = collector->CollectRawDMA();
    std::cout << "collect raw DMA result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: MemoryCollectorTest009
 * @tc.desc: used to test MemoryCollector.CollectAllAIProcess
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest009, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::vector<AIProcessMem>> data = collector->CollectAllAIProcess();
    std::cout << "collect all AI process result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: MemoryCollectorTest010
 * @tc.desc: used to test MemoryCollector.ExportAllAIProcess
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest010, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::string> data = collector->ExportAllAIProcess();
    std::cout << "export all AI process result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: MemoryCollectorTest011
 * @tc.desc: used to test MemoryCollector.CollectRawSmaps
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest011, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::string> data = collector->CollectRawSmaps(1000);
    std::cout << "collect raw smaps info result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: MemoryCollectorTest012
 * @tc.desc: used to test MemoryCollector.CollectHprof
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest012, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::string> data = collector->CollectHprof(1000);
    std::cout << "collect heap snapshot result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}