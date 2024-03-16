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

#include "leak_detector_module_test.h"

#include <queue>
#include "test_util.h"
#include "parameters.h"
#include "fault_detector_util.h"

#include "logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("LeakDetectorModuleTest");
using namespace std;
using namespace testing::ext;

constexpr int OVERTIME_TO_GENERATE_FILES = 60; // 60s
constexpr int OVERTIME_TO_START_PROCESS = 5; // 5s
constexpr int OVERTIME_TO_LEAK = 30; // 30s
const string MY_PROCESS = "LeakDetectorModuleTest";
const string THRESHOLD_LINE = "DEFAULT 1";
const string NATIVE_THRESHOLD_PATH = "/system/etc/hiview/memory_leak_threshold";
const string NATIVE_THRESHOLD_BAK_PATH = "/data/local/tmp/memory_leak_threshold_bak";

void LeakDetectorModuleTest::SetUpTestCase(void)
{
    system::SetParameter("hiview.memleak.test", "enable");
    TestUtil::CopyFile(NATIVE_THRESHOLD_PATH, NATIVE_THRESHOLD_BAK_PATH);
    TestUtil::WriteFile(NATIVE_THRESHOLD_PATH, THRESHOLD_LINE);
    TestUtil::KillProcess("hiview");
}

void LeakDetectorModuleTest::TearDownTestCase(void)
{
    TestUtil::CopyFile(NATIVE_THRESHOLD_BAK_PATH, NATIVE_THRESHOLD_PATH);
    TestUtil::DeleteFile(NATIVE_THRESHOLD_BAK_PATH);
    system::SetParameter("hiview.memleak.test", "disable");
    TestUtil::KillProcess("hiview");
}

void LeakDetectorModuleTest::SetUp(void)
{
    TestUtil::ClearDir(MEMORY_LEAK_PATH);
}

void LeakDetectorModuleTest::TearDown(void)
{
}

/**
 * @tc.name: LeakDetectorModuleTest001
 * @tc.desc: check whether the sample files generated.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: wuzhenyang
 */
HWTEST_F(LeakDetectorModuleTest, LeakDetectorModuleTest001, TestSize.Level1)
{
    string sampleFile = TestUtil::GetSampleFile(MY_PROCESS);
    HIVIEW_LOGI("sampleFile:%{public}s", sampleFile.c_str());
    ASSERT_FALSE(sampleFile.empty());

    LeakDetectorModuleTest::WaitFile(sampleFile);
    HIVIEW_LOGI("start assert FileExists sampleFile");
    ASSERT_TRUE(TestUtil::FileExists(sampleFile));
}

void LeakDetectorModuleTest::WaitFile(string file)
{
    int checkCnt = 0;
    while (checkCnt++ < OVERTIME_TO_GENERATE_FILES) {
        if (TestUtil::FileExists(file)) {
            HIVIEW_LOGI("WaitFile success, file:%{public}s", file.c_str());
            return;
        }
        sleep(1);
    }
    HIVIEW_LOGI("WaitFile failed, checkCnt:%{public}d", checkCnt);
}

} // namespace HiviewDFX
} // namespace OHOS
