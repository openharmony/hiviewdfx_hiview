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

#include "gwpasan_collector.h"

using namespace testing::ext;
namespace OHOS {
namespace HiviewDFX {
class AsanUnittest : public testing::Test {
public:
    void SetUp()
    {
        chmod("/data/log/faultlog/", 0777); // 0777: add other user write permission
        chmod("/data/log/faultlog/faultlogger/", 0777); // 0777: add other user write permission
        sleep(1);
    };
    void TearDown()
    {
        chmod("/data/log/faultlog/", 0770); // 0770: restore permission
        chmod("/data/log/faultlog/faultlogger/", 0770); // 0770: restore permission
    };
};

/**
 * @tc.name: AsanTest001
 * @tc.desc: Test calling WriteGwpAsanLog Func
 * @tc.type: FUNC
 */
HWTEST_F(AsanUnittest, WriteGwpAsanLogTest001, testing::ext::TestSize.Level1)
{
    char gwpAsanBuf[] = "Test GWP-ASAN, End GWP-ASan report";
    WriteGwpAsanLog(gwpAsanBuf, strlen(gwpAsanBuf));
    char asanBuf[] = "Test ASAN, End of process memory map";
    WriteGwpAsanLog(asanBuf, strlen(asanBuf));
    char tsanBuf[] = "Test TSAN, End TSAN report";
    WriteGwpAsanLog(tsanBuf, strlen(tsanBuf));
    ASSERT_TRUE(true);
}

/**
 * @tc.name: AsanTest002
 * @tc.desc: Test calling GetApplicationVersion Func
 * @tc.type: FUNC
 */
HWTEST_F(AsanUnittest, GetApplicationVersionTest001, testing::ext::TestSize.Level1)
{
    int32_t uid = getuid();
    std::string procName = GetApplicationNameById(uid);
    std::string appVersion = GetApplicationVersion(uid, procName);
    ASSERT_TRUE(appVersion == "");
}
} // namespace HiviewDFX
} // namespace OHOS