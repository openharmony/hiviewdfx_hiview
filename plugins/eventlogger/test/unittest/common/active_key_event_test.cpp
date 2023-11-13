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
#include "active_key_event_test.h"

#include <ctime>
#include <memory>
#include <vector>
#include <string>

#include "active_key_event.h"
#include "event_thread_pool.h"
#include "log_store_ex.h"
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

void ActiveKeyEventTest::SetUp()
{
    printf("SetUp.\n");
}

void ActiveKeyEventTest::TearDown()
{
    printf("TearDown.\n");
}

void ActiveKeyEventTest::SetUpTestCase()
{
}

void ActiveKeyEventTest::TearDownTestCase()
{
}

/**
 * @tc.name: ActiveKeyEventTest
 * @tc.desc: ActiveKeyEventTest Init
 * @tc.type: FUNC
 */
HWTEST_F(ActiveKeyEventTest, ActiveKeyEventTest, TestSize.Level3)
{
    std::shared_ptr<EventThreadPool> eventPool = std::make_shared<EventThreadPool>(5, "EventThreadPool");
    EXPECT_TRUE(eventPool != nullptr);
    const std::string logStorePath = "/data/log/test/";
    std::shared_ptr<LogStoreEx> logStoreEx = std::make_shared<LogStoreEx>(logStorePath, true);
    auto ret = logStoreEx->Init();
    EXPECT_EQ(ret, true);
    std::shared_ptr<ActiveKeyEvent> activeKeyEvent = std::make_shared<ActiveKeyEvent>();
    activeKeyEvent->Init(eventPool, logStoreEx);
}
