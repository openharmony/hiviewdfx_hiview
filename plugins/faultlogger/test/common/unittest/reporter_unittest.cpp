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

#include "reporter.h"

using namespace testing::ext;
namespace OHOS {
namespace HiviewDFX {
class ReporterUnittest : public testing::Test {};

/**
 * @tc.name: InitTest001
 * @tc.desc: Test Init
 * @tc.type: FUNC
 */
HWTEST_F(ReporterUnittest, InitTest001, testing::ext::TestSize.Level1)
{
    SanitizerdType type = static_cast<SanitizerdType>(-1);
    int result = Init(type);
    ASSERT_EQ(result, -1);
    type = SanitizerdType::ASAN_LOG_RPT;
    result = Init(type);
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: Upload001
 * @tc.desc: Test Upload
 * @tc.type: FUNC
 */
HWTEST_F(ReporterUnittest, Upload001, testing::ext::TestSize.Level1)
{
    T_SANITIZERD_PARAMS params;
    Upload(&params);
    SanitizerdType type = static_cast<SanitizerdType>(-1);
    params.type = type;
    Upload(&params);
}
} // namespace HiviewDFX
} // namespace OHOS
