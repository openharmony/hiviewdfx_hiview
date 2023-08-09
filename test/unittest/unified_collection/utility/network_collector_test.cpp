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

#include "network_collector.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::UCollectUtil;

class NetworkCollectorTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
};

/**
 * @tc.name: NetworkCollectorTest001
 * @tc.desc: used to test NetworkCollector.CollectRate
 * @tc.type: FUNC
*/
HWTEST_F(NetworkCollectorTest, NetworkCollectorTest001, TestSize.Level1)
{
    std::shared_ptr<NetworkCollector> collector = NetworkCollector::Create();
    CollectResult<NetworkRate> data = collector->CollectRate();
    std::cout << "collect network rate result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: NetworkCollectorTest002
 * @tc.desc: used to test NetworkCollector.CollectSysPackets
 * @tc.type: FUNC
*/
HWTEST_F(NetworkCollectorTest, NetworkCollectorTest002, TestSize.Level1)
{
    std::shared_ptr<NetworkCollector> collector = NetworkCollector::Create();
    CollectResult<NetworkPackets> data = collector->CollectSysPackets();
    std::cout << "collect network packets result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}