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

#include "event_checker_test.h"

#include "parameter_ex.h"
#include "compliant_event_checker.h"

namespace OHOS {
namespace HiviewDFX {

void EventCheckerTest::SetUpTestCase() {}

void EventCheckerTest::TearDownTestCase() {}

void EventCheckerTest::SetUp() {}

void EventCheckerTest::TearDown() {}

/**
 * @tc.name: EventCheckerTest001
 * @tc.desc: Test isCompliantRule apis of RuleChecker
 * @tc.type: FUNC
 * @tc.require: issueIAPJKN
 */
HWTEST_F(EventCheckerTest, EventCheckerTest001, testing::ext::TestSize.Level3)
{
    CompliantEventChecker eventChecker;
    if (Parameter::GetInteger("const.secure", 1) == 0) { // 1 and 0 is test value
        ASSERT_TRUE(eventChecker.IsCompliantEvent("AAFWK", ""));
        ASSERT_TRUE(eventChecker.IsCompliantEvent("AAFWK", "START_ABILITY"));
        ASSERT_TRUE(eventChecker.IsCompliantEvent("ACE", "INTERACTION_COMPLETED_LATENCY"));
        ASSERT_TRUE(eventChecker.IsCompliantEvent("AV_CODEC", "CODEC_START_INFO"));
        ASSERT_TRUE(eventChecker.IsCompliantEvent("GRAPHIC", "INTERACTION_COMPLETED_LATENCY"));
        ASSERT_TRUE(eventChecker.IsCompliantEvent("LOCATION", "GNSS_STATE"));
        ASSERT_TRUE(eventChecker.IsCompliantEvent("PERFORMANCE", ""));
        ASSERT_TRUE(eventChecker.IsCompliantEvent("RELIBILITY", ""));
        ASSERT_TRUE(eventChecker.IsCompliantEvent("STABILITY", "JS_ERROR"));
        ASSERT_TRUE(eventChecker.IsCompliantEvent("STABILITY", "JS_ERROR_INVALID"));
        ASSERT_TRUE(eventChecker.IsCompliantEvent("WORKSCHEDULER", "WORK_ADD"));
        ASSERT_TRUE(eventChecker.IsCompliantEvent("STABILITY", "WORK_ADD_INVALID"));
    } else {
        ASSERT_TRUE(eventChecker.IsCompliantEvent("AAFWK", ""));
        ASSERT_FALSE(eventChecker.IsCompliantEvent("AAFWK", "START_ABILITY"));
        ASSERT_FALSE(eventChecker.IsCompliantEvent("ACE", "INTERACTION_COMPLETED_LATENCY"));
        ASSERT_FALSE(eventChecker.IsCompliantEvent("AV_CODEC", "CODEC_START_INFO"));
        ASSERT_FALSE(eventChecker.IsCompliantEvent("GRAPHIC", "INTERACTION_COMPLETED_LATENCY"));
        ASSERT_FALSE(eventChecker.IsCompliantEvent("LOCATION", "GNSS_STATE"));
        ASSERT_FALSE(eventChecker.IsCompliantEvent("PERFORMANCE", ""));
        ASSERT_FALSE(eventChecker.IsCompliantEvent("RELIBILITY", ""));
        ASSERT_FALSE(eventChecker.IsCompliantEvent("STABILITY", "JS_ERROR"));
        ASSERT_TRUE(eventChecker.IsCompliantEvent("STABILITY", "JS_ERROR_INVALID"));
        ASSERT_FALSE(eventChecker.IsCompliantEvent("WORKSCHEDULER", "WORK_ADD"));
        ASSERT_TRUE(eventChecker.IsCompliantEvent("STABILITY", "WORK_ADD_INVALID"));
    }
}
} //
} //