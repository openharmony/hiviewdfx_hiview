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
#include <iostream>

#include <gtest/gtest.h>

#include "area_policy.h"
#include "privacy_controller.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

class PrivacyControllerTest : public testing::Test {
public:
    void SetUp() {}
    void TearDown() {}
};

namespace {
const std::string CONFIG_PATH1 = "/data/test/hiview/privacy_control/area_policy1.json";
const std::string CONFIG_PATH2 = "/data/test/hiview/privacy_control/area_policy2.json";
const std::string CONFIG_PATH3 = "/data/test/hiview/privacy_control/area_policy3.json";
const std::string CONFIG_PATH4 = "/data/test/hiview/privacy_control/area_policy4.json";
const std::string CONFIG_PATH5 = "/data/test/hiview/privacy_control/area_policy5.json";

const std::string TEST_DOMAIN = "DEFAULT_DOMAIN";
const std::string TEST_NAME = "DEFAULT_NAME";

const std::string DEBUG_LEVEL = "DEBUG";
const std::string INFO_LEVEL = "INFO";
const std::string MINOR_LEVEL = "MINOR";
const std::string MAJOR_LEVEL = "MAJOR";
const std::string CRITICAL_LEVEL = "CRITICAL";

constexpr uint8_t SECRET_PRIVACY = 1;
constexpr uint8_t SENSITIVE_PRIVACY = 2;
constexpr uint8_t PROTECTED_PRIVACY = 3;
constexpr uint8_t PUBLIC_PRIVACY = 4;

std::shared_ptr<Event> CreateEvent(const std::string& domain, const std::string& name,
    SysEventCreator::EventType type = SysEventCreator::FAULT)
{
    SysEventCreator sysEventCreator(domain, name, type);
    auto event = std::make_shared<SysEvent>("", nullptr, sysEventCreator);
    event->SetLevel(DEBUG_LEVEL);
    event->SetPrivacy(SECRET_PRIVACY);
    return event;
}

std::shared_ptr<SysEvent> CreateSysEvent(const std::string& domain, const std::string& name,
    SysEventCreator::EventType type = SysEventCreator::FAULT)
{
    auto event = CreateEvent(domain, name, type);
    return std::static_pointer_cast<SysEvent>(event);
}
}

/**
 * @tc.name: PrivacyControllerTest001
 * @tc.desc: plugin test.
 * @tc.type: FUNC
 * @tc.require: issueI9TJGM
 */
HWTEST_F(PrivacyControllerTest, PrivacyControllerTest001, TestSize.Level0)
{
    PrivacyController plugin;
    std::shared_ptr<Event> event = nullptr;
    ASSERT_FALSE(plugin.OnEvent(event));

    auto event1 = CreateEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::FAULT);
    ASSERT_TRUE(plugin.OnEvent(event1));

    auto event2 = CreateEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::STATISTIC);
    ASSERT_TRUE(plugin.OnEvent(event2));

    auto event3 = CreateEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::SECURITY);
    ASSERT_TRUE(plugin.OnEvent(event3));

    auto event4 = CreateEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::BEHAVIOR);
    ASSERT_TRUE(plugin.OnEvent(event4));
}

/**
 * @tc.name: PrivacyControllerTest002
 * @tc.desc: default policy test.
 * @tc.type: FUNC
 * @tc.require: issueI9TJGM
 */
HWTEST_F(PrivacyControllerTest, PrivacyControllerTest002, TestSize.Level0)
{
    AreaPolicy areaPolicy(CONFIG_PATH1);

    auto event1 = CreateSysEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::FAULT);
    ASSERT_TRUE(areaPolicy.IsAllowed(event1));

    auto event2 = CreateSysEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::STATISTIC);
    ASSERT_TRUE(areaPolicy.IsAllowed(event2));

    auto event3 = CreateSysEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::SECURITY);
    ASSERT_TRUE(areaPolicy.IsAllowed(event3));

    auto event4 = CreateSysEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::BEHAVIOR);
    ASSERT_TRUE(areaPolicy.IsAllowed(event4));
}

/**
 * @tc.name: PrivacyControllerTest003
 * @tc.desc: allowLevel policy test.
 * @tc.type: FUNC
 * @tc.require: issueI9TJGM
 */
HWTEST_F(PrivacyControllerTest, PrivacyControllerTest003, TestSize.Level0)
{
    AreaPolicy areaPolicy(CONFIG_PATH2);
    auto event = CreateSysEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::FAULT);
    event->SetLevel(DEBUG_LEVEL);
    ASSERT_FALSE(areaPolicy.IsAllowed(event));

    event->SetLevel(INFO_LEVEL);
    ASSERT_TRUE(areaPolicy.IsAllowed(event));

    event->SetLevel(MINOR_LEVEL);
    ASSERT_TRUE(areaPolicy.IsAllowed(event));

    event->SetLevel(MAJOR_LEVEL);
    ASSERT_TRUE(areaPolicy.IsAllowed(event));

    event->SetLevel(CRITICAL_LEVEL);
    ASSERT_TRUE(areaPolicy.IsAllowed(event));
}

/**
 * @tc.name: PrivacyControllerTest004
 * @tc.desc: allowPrivacy policy test.
 * @tc.type: FUNC
 * @tc.require: issueI9TJGM
 */
HWTEST_F(PrivacyControllerTest, PrivacyControllerTest004, TestSize.Level0)
{
    AreaPolicy areaPolicy(CONFIG_PATH3);
    auto event = CreateSysEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::FAULT);
    event->SetPrivacy(SECRET_PRIVACY);
    ASSERT_FALSE(areaPolicy.IsAllowed(event));

    event->SetPrivacy(SENSITIVE_PRIVACY);
    ASSERT_FALSE(areaPolicy.IsAllowed(event));

    event->SetPrivacy(PROTECTED_PRIVACY);
    ASSERT_TRUE(areaPolicy.IsAllowed(event));

    event->SetPrivacy(PUBLIC_PRIVACY);
    ASSERT_TRUE(areaPolicy.IsAllowed(event));
}

/**
 * @tc.name: PrivacyControllerTest005
 * @tc.desc: allowSysUe policy test.
 * @tc.type: FUNC
 * @tc.require: issueI9TJGM
 */
HWTEST_F(PrivacyControllerTest, PrivacyControllerTest005, TestSize.Level0)
{
    AreaPolicy areaPolicy(CONFIG_PATH4);
    auto event1 = CreateSysEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::FAULT);
    ASSERT_TRUE(areaPolicy.IsAllowed(event1));

    auto event2 = CreateSysEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::STATISTIC);
    ASSERT_TRUE(areaPolicy.IsAllowed(event2));

    auto event3 = CreateSysEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::SECURITY);
    ASSERT_TRUE(areaPolicy.IsAllowed(event3));

    auto event4 = CreateSysEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::BEHAVIOR);
    ASSERT_FALSE(areaPolicy.IsAllowed(event4));
}

/**
 * @tc.name: PrivacyControllerTest006
 * @tc.desc: allowUe policy test.
 * @tc.type: FUNC
 * @tc.require: issueI9TJGM
 */
HWTEST_F(PrivacyControllerTest, PrivacyControllerTest006, TestSize.Level0)
{
    AreaPolicy areaPolicy(CONFIG_PATH5);
    const std::string testUeDomain = "DOMAIN_UE";
    auto event = CreateSysEvent(testUeDomain, TEST_NAME, SysEventCreator::BEHAVIOR);
    ASSERT_FALSE(areaPolicy.IsAllowed(event));

    auto event1 = CreateSysEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::FAULT);
    ASSERT_TRUE(areaPolicy.IsAllowed(event1));

    auto event2 = CreateSysEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::STATISTIC);
    ASSERT_TRUE(areaPolicy.IsAllowed(event2));

    auto event3 = CreateSysEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::SECURITY);
    ASSERT_TRUE(areaPolicy.IsAllowed(event3));

    auto event4 = CreateSysEvent(TEST_DOMAIN, TEST_NAME, SysEventCreator::BEHAVIOR);
    ASSERT_TRUE(areaPolicy.IsAllowed(event4));
}
