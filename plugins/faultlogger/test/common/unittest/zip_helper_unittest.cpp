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
#include <gtest/gtest.h>
#include <unistd.h>

#include "zip_helper.h"

using namespace testing::ext;
namespace OHOS {
namespace HiviewDFX {
class ZipHeloerUnittest : public testing::Test {};
/**
 * @tc.name: SplitStringTest001
 * @tc.desc: Test SplitString
 * @tc.type: FUNC
 */
HWTEST_F(ZipHeloerUnittest, SplitStringTest001, testing::ext::TestSize.Level1)
{
    std::string input = "data/test";
    std::string regex = "/";
    auto resultArray = SplitString(input, regex);
    ASSERT_EQ(resultArray[0], "data");
    ASSERT_EQ(resultArray[1], "test");
}

/**
 * @tc.name: HashStringTest001
 * @tc.desc: Test HashString
 * @tc.type: FUNC
 */
HWTEST_F(ZipHeloerUnittest, HashStringTest001, testing::ext::TestSize.Level1)
{
    std::string input = "data/test";
    auto result = HashString(input);
    ASSERT_GT(result, 0);
}

/**
 * @tc.name: IsModuleNameValidTest001
 * @tc.desc: Test IsModuleNameValid error branch
 * @tc.type: FUNC
 */
HWTEST_F(ZipHeloerUnittest, IsModuleNameValidTest001, testing::ext::TestSize.Level1)
{
    std::string name;
    auto result = IsModuleNameValid(name);
    ASSERT_EQ(result, false);
    name = "@data/test";
    result = IsModuleNameValid(name);
    ASSERT_EQ(result, true);
}
} // namespace HiviewDFX
} // namespace OHOS
