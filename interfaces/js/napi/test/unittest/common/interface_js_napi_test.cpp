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
#include "interface_js_napi_test.h"
#include "string_util.h"
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

void InterfaceJsNapiTest::SetUp()
{
    /**
     * @tc.setup: create an event loop and multiple event handlers
     */
    printf("SetUp.\n");
}

void InterfaceJsNapiTest::TearDown()
{
    /**
     * @tc.teardown: destroy the event loop we have created
     */
    printf("TearDown.\n");
}

/**
 * @tc.name: HideSnInfo001
 * @tc.desc: hide the sn in the file
 * @tc.type: FUNC
 */
HWTEST_F(InterfaceJsNapiTest, HideSnInfo001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. hide sn for full file name
     */
    const std::string fullFileName =
    "AAA-BBBB-CCC-DDD 0.0.0.0EE(FFFFFFFFFFFFFF)_01234567$AAAAAAAAAaaaaaa+BBBBBBBBBbbbbbbbbb=_123456789123456_A.zip";
    std::string hideSnFileName = StringUtil::HideSnInfo(fullFileName);

    const std::string expectFileName =
    "AAA-BBBB-CCC-DDD 0.0.0.0EE(FFFFFFFFFFFFFF)_******_123456789123456_A.zip";
    EXPECT_EQ(hideSnFileName, expectFileName);
}