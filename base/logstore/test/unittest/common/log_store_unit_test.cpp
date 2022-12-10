/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "log_store_unit_test.h"

#include "file_util.h"
#include "hiview_global.h"
#include "log_file.h"
#include "log_store_ex.h"
#include "plugin.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const char LOG_FILE_PATH[] = "/data/log/hiview/log_store_test/";
const char LOG_CONTENT[] = "just for ohos test";
constexpr int SIZE_512 = 512; // test value
constexpr int SIZE_10240 = 10240; // test value
constexpr int LOG_FILE_START_INDEX = 0; // test value

std::string GetLogDir()
{
    std::string workPath = std::string(LOG_FILE_PATH);
    if (workPath.back() != '/') {
        workPath = workPath + "/";
    }
    std::string logDestDir = workPath;
    if (!FileUtil::FileExists(logDestDir)) {
        FileUtil::ForceCreateDirectory(logDestDir, FileUtil::FILE_PERM_770);
    }
    return logDestDir;
}

std::string GenerateLogFileName(int index)
{
    return GetLogDir() + "logfile" + std::to_string(index);
}
}
void LogStoreUnitTest::SetUpTestCase()
{
}

void LogStoreUnitTest::TearDownTestCase()
{
}

void LogStoreUnitTest::SetUp()
{
}

void LogStoreUnitTest::TearDown()
{
    (void)FileUtil::ForceRemoveDirectory(LOG_FILE_PATH);
}

/**
 * @tc.name: LogStoreUnitTest001
 * @tc.desc: Test initialization of LogFile
 * @tc.type: FUNC
 * @tc.require: issueI64QXL
 */
HWTEST_F(LogStoreUnitTest, LogStoreUnitTest001, testing::ext::TestSize.Level3)
{
    const std::string testLogDir = std::string(LOG_FILE_PATH);
    FileUtil::ForceRemoveDirectory(testLogDir);
    ASSERT_TRUE(true);
    LogFile file(testLogDir);
    ASSERT_EQ(false, file.isValid_);
    ASSERT_EQ(false, file.isDir_);
    ASSERT_EQ(0, file.size_);
    ASSERT_EQ(testLogDir, file.path_);
    ASSERT_EQ(FileUtil::ExtractFileName(testLogDir), file.name_);
    FileUtil::ForceCreateDirectory(testLogDir);
    LogFile file1(testLogDir);
    ASSERT_EQ(true, file1.isValid_);
    ASSERT_EQ(true, file1.isDir_);
    ASSERT_EQ(FileUtil::GetFolderSize(testLogDir), file1.size_);
    ASSERT_EQ(testLogDir, file1.path_);
    ASSERT_EQ(FileUtil::ExtractFileName(testLogDir), file1.name_);
    const std::string testLogFile = testLogDir + "logFile";
    (void)FileUtil::SaveStringToFile(testLogFile, LOG_CONTENT);
    LogFile file2(testLogFile);
    ASSERT_EQ(true, file2.isValid_);
    ASSERT_EQ(false, file2.isDir_);
    ASSERT_EQ(std::string(LOG_CONTENT).size(), file2.size_);
    ASSERT_EQ(testLogFile, file2.path_);
    ASSERT_EQ(FileUtil::ExtractFileName(testLogFile), file2.name_);
    (void)FileUtil::ForceRemoveDirectory(testLogDir);
    ASSERT_TRUE(true);
}

/**
 * @tc.name: LogStoreUnitTest002
 * @tc.desc: Test APIs of LogStoreEx
 * @tc.type: FUNC
 * @tc.require: issueI64QXL
 */
HWTEST_F(LogStoreUnitTest, LogStoreUnitTest002, testing::ext::TestSize.Level3)
{
    const std::string logStorePath = std::string(LOG_FILE_PATH);
    FileUtil::ForceRemoveDirectory(logStorePath);
    ASSERT_TRUE(true);
    (void)FileUtil::SaveStringToFile(GenerateLogFileName(LOG_FILE_START_INDEX), LOG_CONTENT);
    LogStoreEx logStoreEx(logStorePath);
    auto ret = logStoreEx.Init();
    ASSERT_EQ(true, ret);
    auto path = logStoreEx.GetPath();
    ASSERT_EQ(logStorePath, path);
    auto allLogFiles = logStoreEx.GetLogFiles();
    ASSERT_EQ(1, allLogFiles.size());
    auto ret1 = logStoreEx.Clear();
    ASSERT_EQ(true, ret1);
    (void)FileUtil::ForceRemoveDirectory(logStorePath);
    auto ret2 = logStoreEx.Clear();
    ASSERT_EQ(false, ret2);
    std::string singleLine;
    for (int index = 0; index < SIZE_512; index++) {
        singleLine += "ohos";
    }
    for (int index = 0; index < SIZE_10240; index++) {
        (void)FileUtil::SaveStringToFile(GenerateLogFileName(index), singleLine);
    }
    (void)logStoreEx.ClearOldestFilesIfNeeded();
    ASSERT_TRUE(true);
    (void)FileUtil::ForceRemoveDirectory(logStorePath);
    ASSERT_TRUE(true);
    (void)FileUtil::ForceRemoveDirectory(logStorePath);
    ASSERT_TRUE(true);
    (void)FileUtil::SaveStringToFile(GenerateLogFileName(LOG_FILE_START_INDEX), LOG_CONTENT);
    logStoreEx.ClearSameLogFilesIfNeeded([] (auto& logFile) {
        return logFile.isValid_;
    }, 0);
    ASSERT_TRUE(true);
    logStoreEx.ClearSameLogFilesIfNeeded([] (auto& logFile) {
        return logFile.isValid_;
    }, 1);
    ASSERT_TRUE(true);
    (void)FileUtil::SaveStringToFile(GenerateLogFileName(LOG_FILE_START_INDEX), LOG_CONTENT);
    auto ret3 = logStoreEx.RemoveLogFile("logfile0");
    ASSERT_EQ(true, ret3);
    auto ret4 = logStoreEx.RemoveLogFile("logfile1");
    ASSERT_EQ(false, ret4);
}
} // namespace HiviewDFX
} // namespace OHOS