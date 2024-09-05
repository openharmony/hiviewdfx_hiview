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
#include <string>

#include "event_publish.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

class EventPublishTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
};

/**
 * @tc.name: EventPublishTest001
 * @tc.desc: used to test PushEvent
 * @tc.type: FUNC
*/
HWTEST_F(EventPublishTest, EventPublishTest001, TestSize.Level1)
{
    EventPublish::GetInstance().PushEvent(100, "APP_CRASH", HiSysEvent::EventType::BEHAVIOR, "{\"time\":123}");
    ASSERT_TRUE(true);
}

/**
 * @tc.name: EventPublishTest002
 * @tc.desc: used to test PushEvent
 * @tc.type: FUNC
*/
HWTEST_F(EventPublishTest, EventPublishTest002, TestSize.Level1)
{
    EventPublish::GetInstance().PushEvent(100, "APP_FREEZE", HiSysEvent::EventType::FAULT, "{\"time\":123}");
    ASSERT_TRUE(true);
}

/**
 * @tc.name: EventPublishTest003
 * @tc.desc: used to test PushEvent
 * @tc.type: FUNC
*/
HWTEST_F(EventPublishTest, EventPublishTest003, TestSize.Level1)
{
    EventPublish::GetInstance().PushEvent(100, "ADDRESS_SANITIZER", HiSysEvent::EventType::FAULT, "{\"time\":123}");
    ASSERT_TRUE(true);
}

/**
 * @tc.name: EventPublishTest004
 * @tc.desc: used to test PushEvent
 * @tc.type: FUNC
*/
HWTEST_F(EventPublishTest, EventPublishTest004, TestSize.Level1)
{
    EventPublish::GetInstance().PushEvent(100, "APP_START", HiSysEvent::EventType::BEHAVIOR, "{\"time\":123}");
    ASSERT_TRUE(true);
}
