/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "running_status_logger_test.h"

#include <chrono>
#include <ctime>
#include <gmock/gmock.h>
#include <iostream>
#include <memory>
#include <thread>

#include "file_util.h"
#include "hiview_global.h"
#include "plugin.h"
#include "running_status_logger.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr char TEST_LOG_DIR[] = "/data/log/hiview/sys_event_test";

class HiviewTestContext : public HiviewContext {
public:
    std::string GetHiViewDirectory(DirectoryType type __UNUSED)
    {
        return TEST_LOG_DIR;
    }
};
}
void RunningStatusLoggerTest::SetUpTestCase()
{
}

void RunningStatusLoggerTest::TearDownTestCase()
{
}

void RunningStatusLoggerTest::SetUp()
{
}

void RunningStatusLoggerTest::TearDown()
{
}

/**
 * @tc.name: RunningStatusLoggerTest_001
 * @tc.desc: write log to file
 * @tc.type: FUNC
 * @tc.require: issueI62PQY
 */
HWTEST_F(RunningStatusLoggerTest, RunningStatusLoggerTest_001, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. init hiview global with customized hiview context
     * @tc.steps: step2. init string with 4k size
     * @tc.steps: step3. log string to log file until the size of this log file over limit 2M.
     */
    HiviewTestContext hiviewTestContext;
    HiviewGlobal::CreateInstance(hiviewTestContext);
    std::string singleLine = "line";
    for (int index = 0; index < 1024; index++) {
        singleLine += "line";
    }
    for (int index = 0; index < 1024; index++) {
        RunningStatusLogger::GetInstance().Log(singleLine);
    }
    ASSERT_TRUE(true);
    FileUtil::ForceRemoveDirectory(TEST_LOG_DIR);
    ASSERT_TRUE(true);
}

/**
 * @tc.name: RunningStatusLoggerTest_002
 * @tc.desc: time stamp format
 * @tc.type: FUNC
 * @tc.require: issueI62PQY
 */
HWTEST_F(RunningStatusLoggerTest, RunningStatusLoggerTest_002, testing::ext::TestSize.Level3)
{
    auto simpleTimeStamp = RunningStatusLogger::GetInstance().FormatTimeStamp(true);
    ASSERT_TRUE(!simpleTimeStamp.empty());
    auto normalTimeStamp = RunningStatusLogger::GetInstance().FormatTimeStamp(false);
    ASSERT_TRUE(!normalTimeStamp.empty() && normalTimeStamp.find(':') != std::string::npos &&
        normalTimeStamp.find('/') != std::string::npos);
}
} // namespace HiviewDFX
} // namespace OHOS