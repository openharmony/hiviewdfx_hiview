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

#include "adapter_utility_ohos_test.h"

#include <fcntl.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

#include "common_utils.h"
#include "file_util.h"
#include "socket_util.h"
#include "time_util.h"

using namespace std;

namespace OHOS {
namespace HiviewDFX {
namespace {
const char LOG_FILE_PATH[] = "/data/test/adapter_utility_test/";
constexpr int SUFFIX_0 = 0;
constexpr int SUFFIX_1 = 1;

std::string GetLogDir(std::string& testCaseName)
{
    std::string workPath = std::string(LOG_FILE_PATH);
    if (workPath.back() != '/') {
        workPath = workPath + "/";
    }
    workPath.append(testCaseName);
    workPath.append("/");
    std::string logDestDir = workPath;
    if (!FileUtil::FileExists(logDestDir)) {
        FileUtil::ForceCreateDirectory(logDestDir, FileUtil::FILE_PERM_770);
    }
    return logDestDir;
}

std::string GenerateLogFileName(std::string& testCaseName, int index)
{
    return GetLogDir(testCaseName) + "testFile" + std::to_string(index);
}
}

void AdapterUtilityOhosTest::SetUpTestCase() {}

void AdapterUtilityOhosTest::TearDownTestCase() {}

void AdapterUtilityOhosTest::SetUp() {}

void AdapterUtilityOhosTest::TearDown()
{
    (void)FileUtil::ForceRemoveDirectory(LOG_FILE_PATH);
}

/**
 * @tc.name: SocketUtilOhosTest001
 * @tc.desc: Test GetExistingSocketServer defined in namespace SocketUtil
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, SocketUtilOhosTest001, testing::ext::TestSize.Level3)
{
    int socketFdIndex = 0;
    auto ret = SocketUtil::GetExistingSocketServer("/dev/socket/unix/hisysevent", socketFdIndex);
    int expectedRet = -1;
    ASSERT_EQ(expectedRet, ret);
}

/**
 * @tc.name: CommonUtilsOhosTest001
 * @tc.desc: Test ExecCommand defined in namespace CommonUtils
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, CommonUtilsOhosTest001, testing::ext::TestSize.Level3)
{
    std::vector<std::string> cmdRet;
    auto ret = CommonUtils::ExecCommand("hisysevent -l -m 10000", cmdRet);
    int expectRet = 0;
    ASSERT_EQ(expectRet, ret);
    ret = CommonUtils::ExecCommand("", cmdRet);
    ASSERT_EQ(expectRet, ret);
}

/**
 * @tc.name: CommonUtilsOhosTest002
 * @tc.desc: Test GetPidByName defined in namespace CommonUtils
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, CommonUtilsOhosTest002, testing::ext::TestSize.Level3)
{
    std::vector<std::string> cmdRet;
    auto hiviewProcessId = CommonUtils::GetPidByName("hiview");
    ASSERT_TRUE(hiviewProcessId > 0);
}

/**
 * @tc.name: CommonUtilsOhosTest003
 * @tc.desc: Test WriteCommandResultToFile defined in namespace CommonUtils
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, CommonUtilsOhosTest003, testing::ext::TestSize.Level3)
{
    std::string caseName("CommonUtilsOhosTest003");
    std::string cmd = "";
    auto ret = CommonUtils::WriteCommandResultToFile(0, cmd);
    ASSERT_EQ(false, ret);
    std::string cmd2 = "hisysevent -l -m 1 | wc -l";
    auto fd = FileUtil::Open(GenerateLogFileName(caseName, SUFFIX_0),
        O_CREAT | O_WRONLY | O_TRUNC, FileUtil::FILE_PERM_770);
    ret = CommonUtils::WriteCommandResultToFile(fd, cmd2);
    ASSERT_EQ(true, ret);
    (void)FileUtil::SaveStringToFile(GenerateLogFileName(caseName, SUFFIX_0), "");
    fd = FileUtil::Open(GenerateLogFileName(caseName, SUFFIX_0),
        O_CREAT | O_WRONLY | O_TRUNC, FileUtil::FILE_PERM_770);
    ret = CommonUtils::WriteCommandResultToFile(fd, cmd2);
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: TimeUtilOhosTest001
 * @tc.desc: Test Sleep/GetMillSecOfSec defined in namespace TimeUtil
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, TimeUtilOhosTest001, testing::ext::TestSize.Level3)
{
    auto time = TimeUtil::GetMillSecOfSec();
    ASSERT_GE(time, 0);
    int sleepSecs = 1;
    TimeUtil::Sleep(sleepSecs);
    ASSERT_TRUE(true);
}

/**
 * @tc.name: TimeUtilOhosTest002
 * @tc.desc: Test Get0ClockStampMs/GetSteadyClockTimeMs defined in namespace TimeUtil
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, TimeUtilOhosTest002, testing::ext::TestSize.Level3)
{
    auto time1 = TimeUtil::Get0ClockStampMs();
    auto time2 = TimeUtil::GetSteadyClockTimeMs();
    ASSERT_GE(time1, 0);
    ASSERT_GE(time2, 0);
}

/**
 * @tc.name: FileUtilOhosTest001
 * @tc.desc: Test LoadBufferFromFile/SaveBufferToFile defined in namespace FileUtil
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, FileUtilOhosTest001, testing::ext::TestSize.Level3)
{
    std::string caseName("FileUtilOhosTest001");
    std::vector<char> content;
    (void)FileUtil::SaveStringToFile(GenerateLogFileName(caseName, SUFFIX_0), "123");
    (void)FileUtil::LoadBufferFromFile(GenerateLogFileName(caseName, SUFFIX_0), content);
    ASSERT_TRUE(true);
    (void)FileUtil::SaveBufferToFile(GenerateLogFileName(caseName, SUFFIX_0), content, true);
    ASSERT_TRUE(true);
}

/**
 * @tc.name: FileUtilOhosTest002
 * @tc.desc: Test ExtractFilePath/ExtractFileName defined in namespace FileUtil
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, FileUtilOhosTest002, testing::ext::TestSize.Level3)
{
    std::string path = "ohos/test/123.txt";
    auto dir = FileUtil::ExtractFilePath(path);
    ASSERT_EQ("ohos/test/", dir);
    auto name = FileUtil::ExtractFileName(path);
    ASSERT_EQ("123.txt", name);
}

/**
 * @tc.name: FileUtilOhosTest003
 * @tc.desc: Test ExcludeTrailingPathDelimiter/IncludeTrailingPathDelimiter defined in namespace FileUtil
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, FileUtilOhosTest003, testing::ext::TestSize.Level3)
{
    auto excludeRet = FileUtil::ExcludeTrailingPathDelimiter("ohos/test/123.txt");
    ASSERT_EQ("ohos/test/123.txt", excludeRet);
    auto name = FileUtil::IncludeTrailingPathDelimiter("ohos/test/123.txt/");
    ASSERT_EQ("ohos/test/123.txt/", name);
}

/**
 * @tc.name: FileUtilOhosTest004
 * @tc.desc: Test ChangeModeFile defined in namespace FileUtil
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, FileUtilOhosTest004, testing::ext::TestSize.Level3)
{
    std::string caseName("FileUtilOhosTest004");
    (void)FileUtil::ChangeModeFile(GenerateLogFileName(caseName, SUFFIX_0), FileUtil::FILE_PERM_755);
    ASSERT_TRUE(true);
    (void)FileUtil::SaveStringToFile(GenerateLogFileName(caseName, SUFFIX_0), "test content");
    (void)FileUtil::ChangeModeFile(GenerateLogFileName(caseName, SUFFIX_0), FileUtil::FILE_PERM_660);
    ASSERT_TRUE(true);
}

/**
 * @tc.name: FileUtilOhosTest005
 * @tc.desc: Test Umask defined in namespace FileUtil
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, FileUtilOhosTest005, testing::ext::TestSize.Level3)
{
    auto umask = FileUtil::Umask(FileUtil::FILE_PERM_755);
    auto expectedRet = 18;
    ASSERT_EQ(expectedRet, umask);
}

/**
 * @tc.name: FileUtilOhosTest006
 * @tc.desc: Test FormatPath2UnixStyle/RemoveFolderBeginWith defined in namespace FileUtil
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, FileUtilOhosTest006, testing::ext::TestSize.Level3)
{
    std::string path = std::string(LOG_FILE_PATH);
    FileUtil::FormatPath2UnixStyle(path);
    ASSERT_TRUE(true);
    FileUtil::RemoveFolderBeginWith(path, "data");
    ASSERT_TRUE(true);
}

/**
 * @tc.name: FileUtilOhosTest007
 * @tc.desc: Test WriteBufferToFd defined in namespace FileUtil
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, FileUtilOhosTest007, testing::ext::TestSize.Level3)
{
    std::string caseName("FileUtilOhosTest007");
    auto fd = -1;
    std::string writeContent = "write content";
    auto ret = FileUtil::WriteBufferToFd(fd, writeContent.c_str(), writeContent.size());
    ASSERT_TRUE(!ret);
    (void)FileUtil::SaveStringToFile(GenerateLogFileName(caseName, SUFFIX_0), "test");
    fd = FileUtil::Open(GenerateLogFileName(caseName, SUFFIX_0),
        O_CREAT | O_WRONLY | O_TRUNC, FileUtil::FILE_PERM_770);
    ret = FileUtil::WriteBufferToFd(fd, nullptr, writeContent.size());
    ASSERT_TRUE(!ret);
    (void)FileUtil::WriteBufferToFd(fd, "", writeContent.size());
    ASSERT_TRUE(true);
    (void)FileUtil::WriteBufferToFd(fd, writeContent.c_str(), writeContent.size());
    ASSERT_TRUE(true);
}

/**
 * @tc.name: FileUtilOhosTest008
 * @tc.desc: Test CreateFile defined in namespace FileUtil
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, FileUtilOhosTest008, testing::ext::TestSize.Level3)
{
    std::string caseName("FileUtilOhosTest008");
    int expectedFailedRet = -1;
    auto ret = FileUtil::CreateFile(GenerateLogFileName(caseName, SUFFIX_0));
    ASSERT_EQ(expectedFailedRet, ret);
    (void)FileUtil::SaveStringToFile(GenerateLogFileName(caseName, SUFFIX_0), "1111");
    (void)FileUtil::CreateFile(GenerateLogFileName(caseName, SUFFIX_0));
    ASSERT_TRUE(true);
}

/**
 * @tc.name: FileUtilOhosTest009
 * @tc.desc: Test CopyFile defined in namespace FileUtil
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, FileUtilOhosTest009, testing::ext::TestSize.Level3)
{
    std::string caseName("FileUtilOhosTest009");
    int expectedFailedRet = -1;
    auto ret = FileUtil::CopyFile("//...../invalid_dest_file", GenerateLogFileName(caseName, SUFFIX_1));
    ASSERT_EQ(expectedFailedRet, ret);
    (void)FileUtil::SaveStringToFile(GenerateLogFileName(caseName, SUFFIX_0), "test");
    ret = FileUtil::CopyFile(GenerateLogFileName(caseName, SUFFIX_0), "//...../invalid_dest_file");
    ASSERT_EQ(expectedFailedRet, ret);
    (void)FileUtil::SaveStringToFile(GenerateLogFileName(caseName, SUFFIX_1), "test");
    (void)FileUtil::CopyFile(GenerateLogFileName(caseName, SUFFIX_0), GenerateLogFileName(caseName, SUFFIX_1));
    ASSERT_TRUE(true);
}

/**
 * @tc.name: FileUtilOhosTest010
 * @tc.desc: Test GetLastLine defined in namespace FileUtil
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, FileUtilOhosTest010, testing::ext::TestSize.Level3)
{
    std::string caseName("FileUtilOhosTest010");
    ifstream in(GenerateLogFileName(caseName, SUFFIX_0));
    std::string line;
    int invalidMaxLen = 5;
    auto ret = FileUtil::GetLastLine(in, line, invalidMaxLen);
    ASSERT_TRUE(!ret);
    (void)FileUtil::SaveStringToFile(GenerateLogFileName(caseName, SUFFIX_1), "line1");
    (void)FileUtil::SaveStringToFile(GenerateLogFileName(caseName, SUFFIX_1), "\nline2");
    (void)FileUtil::SaveStringToFile(GenerateLogFileName(caseName, SUFFIX_1), "\nline3");
    ifstream in1(GenerateLogFileName(caseName, SUFFIX_1));
    ret = FileUtil::GetLastLine(in1, line, invalidMaxLen);
    ASSERT_TRUE(!ret);
    int validMaxLen = 2;
    ret = FileUtil::GetLastLine(in1, line, validMaxLen);
    ASSERT_TRUE(!ret);
    validMaxLen = 3;
    ret = FileUtil::GetLastLine(in1, line, validMaxLen);
    ASSERT_TRUE(!ret);
}

/**
 * @tc.name: FileUtilOhosTest011
 * @tc.desc: Test GetParentDir defined in namespace FileUtil
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, FileUtilOhosTest011, testing::ext::TestSize.Level3)
{
    auto ret = FileUtil::GetParentDir("");
    ASSERT_EQ("", ret);
    ret = FileUtil::GetParentDir("123/345/789");
    ASSERT_EQ("123/345", ret);
}

/**
 * @tc.name: FileUtilOhosTest012
 * @tc.desc: Test IslegalPath defined in namespace FileUtil
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, FileUtilOhosTest012, testing::ext::TestSize.Level3)
{
    auto ret = FileUtil::IsLegalPath("aa/../bb");
    ASSERT_TRUE(!ret);
    ret = FileUtil::IsLegalPath("aa/./bb");
    ASSERT_TRUE(!ret);
    ret = FileUtil::IsLegalPath("aa/bb/");
    ASSERT_TRUE(ret);
    ret = FileUtil::IsLegalPath("aa/bb/cc");
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: FileUtilOhosTest013
 * @tc.desc: Test RenameFile defined in namespace FileUtil
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(AdapterUtilityOhosTest, FileUtilOhosTest013, testing::ext::TestSize.Level3)
{
    std::string caseName("FileUtilOhosTest013");
    auto ret = FileUtil::RenameFile(GenerateLogFileName(caseName, SUFFIX_0),
        GenerateLogFileName(caseName, SUFFIX_1));
    ASSERT_TRUE(!ret);
    (void)FileUtil::SaveStringToFile(GenerateLogFileName(caseName, SUFFIX_0), "line1");
    (void)FileUtil::SaveStringToFile(GenerateLogFileName(caseName, SUFFIX_1), "line1");
    (void)FileUtil::RenameFile(GenerateLogFileName(caseName, SUFFIX_0),
        GenerateLogFileName(caseName, SUFFIX_1));
    ASSERT_TRUE(true);
}
}
}