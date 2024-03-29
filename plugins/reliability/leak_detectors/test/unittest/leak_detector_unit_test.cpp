/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "leak_detector_unit_test.h"

#include <unordered_map>
#include <iostream>
#include "fault_detector_manager.h"
#include "parameters.h"
#include "native_leak_config.h"
#include "test_util.h"

#include "logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("LeakDetectorUnitTest");

using namespace std;
using namespace testing::ext;

constexpr int BUFFER_LENGTH = 128;

void LeakDetectorUnitTest::SetUpTestCase(void)
{
    system::SetParameter("hiview.memleak.test", "enable");
}

void LeakDetectorUnitTest::TearDownTestCase(void)
{
    system::SetParameter("hiview.memleak.test", "disable");
    TestUtil::KillProcess("hiview");
}

void LeakDetectorUnitTest::SetUp(void) {}

void LeakDetectorUnitTest::TearDown(void) {}

/**
 * @tc.name: LeakDetectorUnitTest001
 * @tc.desc: check config parser
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: wuzhenyang
 */
HWTEST_F(LeakDetectorUnitTest, LeakDetectorUnitTest001, TestSize.Level1)
{
    unordered_map<string, uint64_t> configList;
    bool ret = NativeLeakConfig::GetThresholdList(configList);
    ASSERT_TRUE(ret);
    auto it = configList.find("DEFAULT");
    ASSERT_NE(it, configList.end());
}

} // namespace HiviewDFX
} // namespace OHOS

