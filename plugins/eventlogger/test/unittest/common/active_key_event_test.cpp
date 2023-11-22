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

#define private public
#define protected public
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
 * @tc.name: ActiveKeyEventTest_001
 * @tc.desc: ActiveKeyEventTest Init
 * @tc.type: FUNC
 */
static HWTEST_F(ActiveKeyEventTest, ActiveKeyEventTest_001, TestSize.Level3)
{
    std::shared_ptr<EventThreadPool> eventPool = std::make_shared<EventThreadPool>(5, "EventThreadPool");
    EXPECT_TRUE(eventPool != nullptr);
    std::string logStorePath = "/data/log/test/";
    std::shared_ptr<LogStoreEx> logStoreEx = std::make_shared<LogStoreEx>(logStorePath, true);
    auto ret = logStoreEx->Init();
    EXPECT_EQ(ret, true);
    std::shared_ptr<ActiveKeyEvent> activeKeyEvent = std::make_shared<ActiveKeyEvent>();
    activeKeyEvent->Init(eventPool, logStoreEx);
}

/**
 * @tc.name: ActiveKeyEventTest_002
 * @tc.desc: ActiveKeyEventTest Init
 * @tc.type: FUNC
 */
static HWTEST_F(ActiveKeyEventTest, ActiveKeyEventTest_002, TestSize.Level3)
{
    std::shared_ptr<EventThreadPool> eventPool = nullptr;
    EXPECT_TRUE(eventPool == nullptr);
    std::string logStorePath = "/data/log/test/";
    std::shared_ptr<LogStoreEx> logStoreEx = std::make_shared<LogStoreEx>(logStorePath, true);
    auto ret = logStoreEx->Init();
    EXPECT_EQ(ret, true);
    std::shared_ptr<ActiveKeyEvent> activeKeyEvent = std::make_shared<ActiveKeyEvent>();
    activeKeyEvent->Init(eventPool, logStoreEx);
}

/**
 * @tc.name: ActiveKeyEventTest_003
 * @tc.desc: ActiveKeyEventTest InitSubscribe
 * @tc.type: FUNC
 */
static HWTEST_F(ActiveKeyEventTest, ActiveKeyEventTest_003, TestSize.Level3)
{
    std::shared_ptr<ActiveKeyEvent> activeKeyEvent = std::make_shared<ActiveKeyEvent>();
    activeKeyEvent->InitSubscribe(OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP, OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN, 5);
}

/**
 * @tc.name: ActiveKeyEventTest_004
 * @tc.desc: ActiveKeyEventTest CombinationKeyCallback
 * @tc.type: FUNC
 */
static HWTEST_F(ActiveKeyEventTest, ActiveKeyEventTest_004, TestSize.Level3)
{
    std::shared_ptr<ActiveKeyEvent> activeKeyEvent = std::make_shared<ActiveKeyEvent>();
    activeKeyEvent->triggeringTime_ = (uint64_t)ActiveKeyEvent::SystemTimeMillisecond();
    std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent = OHOS::MMI::KeyEvent::Create();
    activeKeyEvent->CombinationKeyCallback(keyEvent);
}

/**
 * @tc.name: ActiveKeyEventTest_005
 * @tc.desc: ActiveKeyEventTest CombinationKeyCallback
 * @tc.type: FUNC
 */
static HWTEST_F(ActiveKeyEventTest, ActiveKeyEventTest_005, TestSize.Level3)
{
    std::shared_ptr<EventThreadPool> eventPool = std::make_shared<EventThreadPool>(5, "EventThreadPool");
    std::shared_ptr<ActiveKeyEvent> activeKeyEvent = std::make_shared<ActiveKeyEvent>();
    activeKeyEvent->eventPool_ = eventPool;
    activeKeyEvent->triggeringTime_ = 0;
    auto keyEvent = OHOS::MMI::KeyEvent::Create();
    activeKeyEvent->CombinationKeyCallback(keyEvent);
}

/**
 * @tc.name: ActiveKeyEventTest_006
 * @tc.desc: ActiveKeyEventTest CombinationKeyCallback
 * @tc.type: FUNC
 */
static HWTEST_F(ActiveKeyEventTest, ActiveKeyEventTest_006, TestSize.Level3)
{
    std::shared_ptr<EventThreadPool> eventPool = nullptr;
    std::shared_ptr<ActiveKeyEvent> activeKeyEvent = std::make_shared<ActiveKeyEvent>();
    activeKeyEvent->eventPool_ = eventPool;
    activeKeyEvent->triggeringTime_ = 0;
    auto keyEvent = OHOS::MMI::KeyEvent::Create();
    activeKeyEvent->CombinationKeyCallback(keyEvent);
}

/**
 * @tc.name: ActiveKeyEventTest_007
 * @tc.desc: ActiveKeyEventTest CombinationKeyCallback
 * @tc.type: FUNC
 */
static HWTEST_F(ActiveKeyEventTest, ActiveKeyEventTest_007, TestSize.Level3)
{
    std::shared_ptr<EventThreadPool> eventPool = std::make_shared<EventThreadPool>(5, "EventThreadPool");
    std::string logStorePath = "/data/log/test/";
    std::shared_ptr<LogStoreEx> logStoreEx = std::make_shared<LogStoreEx>(logStorePath, true);
    logStoreEx->Init();
    std::shared_ptr<ActiveKeyEvent> activeKeyEvent = std::make_shared<ActiveKeyEvent>();
    activeKeyEvent->Init(eventPool, logStoreEx);
    activeKeyEvent->triggeringTime_ = 0;
    auto keyEvent = OHOS::MMI::KeyEvent::Create();
    activeKeyEvent->CombinationKeyCallback(keyEvent);
}

/**
 * @tc.name: ActiveKeyEventTest_008
 * @tc.desc: ActiveKeyEventTest CombinationKeyHandle
 * @tc.type: FUNC
 */
static HWTEST_F(ActiveKeyEventTest, ActiveKeyEventTest_008, TestSize.Level3)
{
    std::shared_ptr<EventThreadPool> eventPool = nullptr;
    std::string logStorePath = "/data/log/test/";
    std::shared_ptr<LogStoreEx> logStoreEx = std::make_shared<LogStoreEx>(logStorePath, true);
    logStoreEx->Init();
    std::shared_ptr<ActiveKeyEvent> activeKeyEvent = std::make_shared<ActiveKeyEvent>();
    activeKeyEvent->Init(eventPool, logStoreEx);
    activeKeyEvent->triggeringTime_ = 0;
    auto keyEvent = OHOS::MMI::KeyEvent::Create();
    activeKeyEvent->CombinationKeyHandle(keyEvent);
}

