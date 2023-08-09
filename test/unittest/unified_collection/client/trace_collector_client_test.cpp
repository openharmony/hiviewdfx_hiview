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

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::UCollectClient;

class TraceCollectorTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
};

/**
 * @tc.name: TraceCollectorTest001
 * @tc.desc: used to test TraceCollector
 * @tc.type: FUNC
*/
HWTEST_F(TraceCollectorTest, TraceCollectorTest001, TestSize.Level1)
{
    std::shared_ptr<TraceCollector> collector = TraceCollector::Create();
    CollectResult<int32_t> onRes = collector->TraceOn();
    std::cout << "collect trace on result" << onRes.retCode << std::endl;
    ASSERT_TRUE(onRes.retCode == UcError::SUCCESS);

    CollectResult<std::string> dumpRes = collector->DumpTrace("traceName");
    std::cout << "collect dump trace result" << dumpRes.retCode << std::endl;
    ASSERT_TRUE(dumpRes.retCode == UcError::SUCCESS);

    CollectResult<std::string> offRes = collector->TraceOff();
    std::cout << "collect trace on result" << offRes.retCode << std::endl;
    ASSERT_TRUE(offRes.retCode == UcError::SUCCESS);
}