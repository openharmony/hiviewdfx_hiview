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

#include "dfx_bundle_util.h"

using namespace testing::ext;
namespace OHOS {
namespace HiviewDFX {
class DfxBundleUtilUnittest : public testing::Test {};

/**
 * @tc.name: IsModuleNameValidTest001
 * @tc.desc: Test IsModuleNameValid error branch
 * @tc.type: FUNC
 */
HWTEST_F(DfxBundleUtilUnittest, IsModuleNameValidTest001, testing::ext::TestSize.Level1)
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
