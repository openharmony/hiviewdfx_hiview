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

#include "sys_event_user_test.h"

#include "sys_event_user.h"

namespace OHOS {
namespace HiviewDFX {
void SysEventUserTest::SetUpTestCase()
{
}

void SysEventUserTest::TearDownTestCase()
{
}

void SysEventUserTest::SetUp()
{
}

void SysEventUserTest::TearDown()
{
}

/**
 * @tc.name: SysEventUserTest001
 * @tc.desc: Test SetName/GetName api
 * @tc.type: FUNC
 * @tc.require: issueI8LYYL
 */
HWTEST_F(SysEventUserTest, SysEventUserTest001, testing::ext::TestSize.Level3)
{
    using namespace EventThreshold;
    std::string name("com.ohos.test");
    SysEventUser user(name, ProcessType::HAP);
    std::string ret;
    user.GetName(ret);
    ASSERT_EQ(name, ret);
    std::string name2("com.ohos.test1");
    user.SetName(name2);
    user.GetName(ret);
    ASSERT_EQ(name2, ret);
}

/**
 * @tc.name: SysEventUserTest002
 * @tc.desc: Test SetProcessType/GetProcessType api
 * @tc.type: FUNC
 * @tc.require: issueI8LYYL
 */
HWTEST_F(SysEventUserTest, SysEventUserTest002, testing::ext::TestSize.Level3)
{
    using namespace EventThreshold;
    std::string name("com.ohos.test");
    SysEventUser user(name, ProcessType::HAP);
    ASSERT_EQ(ProcessType::HAP, user.GetProcessType());
    user.SetProcessType(ProcessType::HAP);
    ASSERT_EQ(ProcessType::HAP, user.GetProcessType());
}

/**
 * @tc.name: SysEventUserTest003
 * @tc.desc: Test SetQueryRuleLimit/GetQueryRuleLimit api
 * @tc.type: FUNC
 * @tc.require: issueI8LYYL
 */
HWTEST_F(SysEventUserTest, SysEventUserTest003, testing::ext::TestSize.Level3)
{
    using namespace EventThreshold;
    std::string name("com.ohos.test");
    SysEventUser user(name, ProcessType::HAP);
    int16_t queryLimit = 100; // test value
    Configs configs {
        .queryRuleLimit = queryLimit,
    };
    user.SetConfigs(configs);
    Configs ret;
    user.GetConfigs(ret);
    ASSERT_EQ(ret.queryRuleLimit, queryLimit);
}
} // namespace HiviewDFX
} // namespace OHOS