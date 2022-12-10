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
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr char TEST_LOG_DIR[] = "/data/log/hiview/sys_event_test";
constexpr int TIME_STAMP_OFFSET = 24 * 60 * 60; // one day
const char TIME_FORMAT[] = "%Y%m%d";
constexpr int SIZE_512 = 512; // test value
constexpr int SIZE_10 = 10; // test value
constexpr int SIZE_10240 = 10240; // test value
std::string FormatTimeStamp()
{
    time_t lt;
    (void)time(&lt);
    return TimeUtil::TimestampFormatToDate(lt - TIME_STAMP_OFFSET, TIME_FORMAT);
}

std::string GetLogDir()
{
    std::string workPath = HiviewGlobal::GetInstance()->GetHiViewDirectory(
        HiviewContext::DirectoryType::WORK_DIRECTORY);
    if (workPath.back() != '/') {
        workPath = workPath + "/";
    }
    std::string logDestDir = workPath + "sys_event/";
    if (!FileUtil::FileExists(logDestDir)) {
        FileUtil::ForceCreateDirectory(logDestDir, FileUtil::FILE_PERM_770);
    }
    return logDestDir;
}

std::string GenerateInvalidFileName(int index)
{
    return GetLogDir() + "runningstatus_2024_0" + std::to_string(index);
}

std::string GenerateYesterdayLogFileName()
{
    return GetLogDir() + "runningstatus_" + FormatTimeStamp() + "_01";
}

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
    (void)FileUtil::ForceRemoveDirectory(TEST_LOG_DIR);
}

/**
 * @tc.name: RunningStatusLoggerTest_001
 * @tc.desc: write logs with size less than 2M into a local file
 * @tc.type: FUNC
 * @tc.require: issueI62PQY
 */
HWTEST_F(RunningStatusLoggerTest, RunningStatusLoggerTest_001, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. init hiview global with customized hiview context
     * @tc.steps: step2. init string with 2k size
     * @tc.steps: step3. log string with total size less than 2M to local file
     */
    FileUtil::ForceRemoveDirectory(TEST_LOG_DIR);
    ASSERT_TRUE(true);
    HiviewTestContext hiviewTestContext;
    HiviewGlobal::CreateInstance(hiviewTestContext);
    std::string singleLine;
    for (int index = 0; index < SIZE_512; index++) {
        singleLine += "ohos";
    }
    for (int index = 0; index < SIZE_512; index++) {
        RunningStatusLogger::GetInstance().Log(singleLine);
    }
    ASSERT_TRUE(true);
}

/**
 * @tc.name: RunningStatusLoggerTest_002
 * @tc.desc: write logs with size less more 2M into local files
 * @tc.type: FUNC
 * @tc.require: issueI64Q4L
 */
HWTEST_F(RunningStatusLoggerTest, RunningStatusLoggerTest_002, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. init hiview global with customized hiview context
     * @tc.steps: step2. init string with 2K size
     * @tc.steps: step3. log string with total size less than 2M to local file
     * @tc.steps: step4. keep logging string with size more than 2M to local file
     */
    FileUtil::ForceRemoveDirectory(TEST_LOG_DIR);
    ASSERT_TRUE(true);
    HiviewTestContext hiviewTestContext;
    HiviewGlobal::CreateInstance(hiviewTestContext);
    std::string singleLine;
    for (int index = 0; index < SIZE_512; index++) {
        singleLine += "ohos";
    }
    for (int index = 0; index < SIZE_512; index++) {
        RunningStatusLogger::GetInstance().Log(singleLine);
    }
    ASSERT_TRUE(true);
    for (int index = 0; index < SIZE_512; index++) {
        RunningStatusLogger::GetInstance().Log(singleLine);
    }
    ASSERT_TRUE(true);
}


/**
 * @tc.name: RunningStatusLoggerTest_003
 * @tc.desc: write logs with size more than 20M into local files
 * @tc.type: FUNC
 * @tc.require: issueI64Q4L
 */
HWTEST_F(RunningStatusLoggerTest, RunningStatusLoggerTest_003, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. init hiview global with customized hiview context
     * @tc.steps: step2. init string with 2K size
     * @tc.steps: step3. log string with size more than 20M to local files
     * @tc.steps: step4. keep logging string to local file
     */
    FileUtil::ForceRemoveDirectory(TEST_LOG_DIR);
    ASSERT_TRUE(true);
    HiviewTestContext hiviewTestContext;
    HiviewGlobal::CreateInstance(hiviewTestContext);
    std::string singleLine;
    for (int index = 0; index < SIZE_512; index++) {
        singleLine += "ohos";
    }
    for (int index = 0; index < SIZE_10240; index++) {
        RunningStatusLogger::GetInstance().Log(singleLine);
    }
    ASSERT_TRUE(true);
    for (int index = 0; index < SIZE_512; index++) {
        RunningStatusLogger::GetInstance().Log(singleLine);
    }
    ASSERT_TRUE(true);
}

/**
 * @tc.name: RunningStatusLoggerTest_004
 * @tc.desc: write logs with newer timestamp
 * @tc.type: FUNC
 * @tc.require: issueI64Q4L
 */
HWTEST_F(RunningStatusLoggerTest, RunningStatusLoggerTest_004, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. init hiview global with customized hiview context
     * @tc.steps: step2. init string with 2K size
     * @tc.steps: step3. log string to a local file with yesterday's time stamp
     * @tc.steps: step4. keep logging string to local file
     */
    FileUtil::ForceRemoveDirectory(TEST_LOG_DIR);
    ASSERT_TRUE(true);
    HiviewTestContext hiviewTestContext;
    HiviewGlobal::CreateInstance(hiviewTestContext);
    std::string singleLine;
    for (int index = 0; index < SIZE_512; index++) {
        singleLine += "ohos";
    }
    (void)FileUtil::SaveStringToFile(GenerateYesterdayLogFileName(), singleLine);
    for (int index = 0; index < SIZE_10; index++) {
        RunningStatusLogger::GetInstance().Log(singleLine);
    }
    ASSERT_TRUE(true);
}

/**
 * @tc.name: RunningStatusLoggerTest_005
 * @tc.desc: time stamp format
 * @tc.type: FUNC
 * @tc.require: issueI62PQY
 */
HWTEST_F(RunningStatusLoggerTest, RunningStatusLoggerTest_005, testing::ext::TestSize.Level3)
{
    auto simpleTimeStamp = RunningStatusLogger::GetInstance().FormatTimeStamp(true);
    ASSERT_TRUE(!simpleTimeStamp.empty());
    auto normalTimeStamp = RunningStatusLogger::GetInstance().FormatTimeStamp(false);
    ASSERT_TRUE(!normalTimeStamp.empty() && normalTimeStamp.find(':') != std::string::npos &&
        normalTimeStamp.find('/') != std::string::npos);
}

/**
 * @tc.name: RunningStatusLoggerTest_007
 * @tc.desc: write logs with newer timestamp
 * @tc.type: FUNC
 * @tc.require: issueI64Q4L
 */
HWTEST_F(RunningStatusLoggerTest, RunningStatusLoggerTest_007, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. init hiview global with customized hiview context
      * @tc.steps: step2. init string with 2K size
     * @tc.steps: step3. init 3 local files with invalid name
     * @tc.steps: step4. log string to a local file with yesterday's time stamp
     * @tc.steps: step5. keep logging string to local file
     */
    FileUtil::ForceRemoveDirectory(TEST_LOG_DIR);
    ASSERT_TRUE(true);
    HiviewTestContext hiviewTestContext;
    HiviewGlobal::CreateInstance(hiviewTestContext);
    std::string singleLine;
    for (int index = 0; index < SIZE_512; index++) {
        singleLine += "ohos";
    }
    for (int index = 0; index < SIZE_10; index++) {
        (void)FileUtil::SaveStringToFile(GenerateInvalidFileName(index), singleLine);
    }
    RunningStatusLogger::GetInstance().Log(singleLine);
    ASSERT_TRUE(true);
}

} // namespace HiviewDFX
} // namespace OHOS