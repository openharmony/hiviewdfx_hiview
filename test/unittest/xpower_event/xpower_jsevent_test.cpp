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
#include "xpower_jsevent_test.h"
#include "test.h"
#include "xpower_event_js.h"
#include "xpower_event_common.h"
#include <parameters.h>

using namespace testing::ext;
using namespace OHOS::HiviewDFX;


class NapiXPowerEventTest : public NativeEngineTest {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "NapiXPowerEventTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "NapiXPowerEventTest TearDownTestCase";
    }

    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: ReportXPowerJsEventTest001
 * @tc.desc: Test undefined type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiXPowerEventTest, ReportXPowerJsEventTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_undefined);
}

/**
 * @tc.name: ReportXPowerJsEventTest002
 * @tc.desc: used to test ReportXPowerJsEventTest
 * @tc.type: FUNC
 */
HWTEST_F(NapiXPowerEventTest, ReportXPowerJsEventTest002, testing::ext::TestSize.Level1)
{
    printf("ReportXPowerEventTest001.\n");
    napi_env env = (napi_env)engine_;
    bool succ = OHOS::system::SetParameter(PROP_XPOWER_OPTIMIZE_ENABLE, "0");
    ASSERT_TRUE(succ);
    sleep(1);
    int param = OHOS::system::GetIntParameter(PROP_XPOWER_OPTIMIZE_ENABLE, 0);
    ASSERT_EQ(param, 0);
    int ret = ReportXPowerJsStackSysEvent(env, "XPOWER_HIVIEW_JSAPI_TEST", "info=1,succ=true");
    ASSERT_EQ(ret, ERR_PROP_NOT_ENABLE);

    printf("ReportXPowerEventTest001.\n");
    succ = OHOS::system::SetParameter(PROP_XPOWER_OPTIMIZE_ENABLE, "1");
    ASSERT_TRUE(succ);
    sleep(1);
    param = OHOS::system::GetIntParameter(PROP_XPOWER_OPTIMIZE_ENABLE, 0);
    ASSERT_EQ(param, 1);
    ret = ReportXPowerJsStackSysEvent(env, "XPOWER_HIVIEW_JSAPI_TEST", "info=2,succ=true");
    ASSERT_EQ(ret, ERR_SUCCESS);

    printf("enable parameter and test default info.\n");
    ret = ReportXPowerJsStackSysEvent(env, "XPOWER_HIVIEW_JSAPI_TEST");
    ASSERT_EQ(ret, ERR_SUCCESS);
}
