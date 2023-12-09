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

#include "event_threshold_mgr_test.h"

#include "event_threshold_manager.h"

namespace OHOS {
namespace HiviewDFX {
void EventThresholdManagerTest::SetUpTestCase()
{
}

void EventThresholdManagerTest::TearDownTestCase()
{
}

void EventThresholdManagerTest::SetUp()
{
}

void EventThresholdManagerTest::TearDown()
{
}

/**
 * @tc.name: EventThresholdManagerTest001
 * @tc.desc: Test GetEventQueryWhiteList api
 * @tc.type: FUNC
 * @tc.require: issueI8LYYL
 */
HWTEST_F(EventThresholdManagerTest, EventThresholdManagerTest001, testing::ext::TestSize.Level3)
{
    using namespace EventThreshold;
    EventThresholdManager& manager = EventThresholdManager::GetInstance();
    size_t ret = manager.GetQueryRuleLimit("com.ohos.test", ProcessType::HAP);
    ASSERT_EQ(ret, manager.GetDefaultQueryRuleLimit());
}
} // namespace HiviewDFX
} // namespace OHOS