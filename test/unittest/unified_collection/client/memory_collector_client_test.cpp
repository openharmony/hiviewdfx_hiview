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
using namespace OHOS::HiviewDFX::UCollectClient;
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