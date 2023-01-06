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

#include "utility_common_utils_test.h"

#include <unistd.h>

#include "calc_fingerprint.h"
#include "file_util.h"
#include "log_parse.h"
#include "tbox.h"

using namespace std;

namespace OHOS {
namespace HiviewDFX {
namespace {
const char LOG_FILE_PATH[] = "/data/test/hiview_utility_test/";
constexpr int SUFFIX_0 = 0;

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

void UtilityCommonUtilsTest::SetUpTestCase() {}

void UtilityCommonUtilsTest::TearDownTestCase() {}

void UtilityCommonUtilsTest::SetUp() {}

void UtilityCommonUtilsTest::TearDown()
{
    (void)FileUtil::ForceRemoveDirectory(LOG_FILE_PATH);
}

/**
 * @tc.name: CalcFingerprintTest001
 * @tc.desc: Test CalcFileSha interface method of class CalcFingerprint
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(UtilityCommonUtilsTest, CalcFingerprintTest001, testing::ext::TestSize.Level3)
{
    (void)FileUtil::ForceRemoveDirectory(LOG_FILE_PATH);
    std::string caseName = "CalcFingerprintTest001";
    CalcFingerprint calcFingerprint;
    char hash[SHA256_DIGEST_LENGTH] = {0};
    auto ret = calcFingerprint.CalcFileSha("", hash, SHA256_DIGEST_LENGTH);
    ASSERT_EQ(EINVAL, ret);
    ret = calcFingerprint.CalcFileSha(GenerateLogFileName(caseName, SUFFIX_0), hash, SHA256_DIGEST_LENGTH);
    ASSERT_EQ(ENOENT, ret);
    ret = calcFingerprint.CalcFileSha("//....../asdsa", hash, SHA256_DIGEST_LENGTH);
    ASSERT_EQ(EINVAL, ret);
    FileUtil::SaveStringToFile(GenerateLogFileName(caseName, SUFFIX_0), "test");
    ret = calcFingerprint.CalcFileSha(GenerateLogFileName(caseName, SUFFIX_0), nullptr, SHA256_DIGEST_LENGTH);
    ASSERT_EQ(EINVAL, ret);
    int invalidLen = 2;
    ret = calcFingerprint.CalcFileSha(GenerateLogFileName(caseName, SUFFIX_0), hash, invalidLen);
    ASSERT_EQ(ENOMEM, ret);
}

/**
 * @tc.name: CalcFingerprintTest002
 * @tc.desc: Test CalcBufferSha interface method of class CalcFingerprint
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(UtilityCommonUtilsTest, CalcFingerprintTest002, testing::ext::TestSize.Level3)
{
    (void)FileUtil::ForceRemoveDirectory(LOG_FILE_PATH);
    CalcFingerprint calcFingerprint;
    char hash[SHA256_DIGEST_LENGTH] = {0};
    std::string buffer1 = "";
    auto ret = calcFingerprint.CalcBufferSha(buffer1, buffer1.size(), hash, SHA256_DIGEST_LENGTH);
    ASSERT_EQ(EINVAL, ret);
    std::string buffer2 = "123";
    ret = calcFingerprint.CalcBufferSha(buffer2, buffer2.size(), nullptr, SHA256_DIGEST_LENGTH);
    ASSERT_EQ(EINVAL, ret);
}

/* @tc.name: LogParseTest001
 * @tc.desc: Test IsIgnoreLibrary interface method of class LogParse
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(UtilityCommonUtilsTest, LogParseTest001, testing::ext::TestSize.Level3)
{
    LogParse logParse;
    auto ret = logParse.IsIgnoreLibrary("watchdog");
    ASSERT_TRUE(ret);
    ret = logParse.IsIgnoreLibrary("ohos");
    ASSERT_TRUE(!ret);
}

/* @tc.name: LogParseTest002
 * @tc.desc: Test MatchExceptionLibrary interface method of class LogParse
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(UtilityCommonUtilsTest, LogParseTest002, testing::ext::TestSize.Level3)
{
    LogParse logParse;
    auto ret = logParse.MatchExceptionLibrary("IllegalAccessExceptionWrapper");
    ASSERT_EQ("IllegalAccessException", ret);
    ret = logParse.MatchExceptionLibrary("NoException");
    ASSERT_EQ(LogParse::UNMATCHED_EXCEPTION, ret);
}

/* @tc.name: TboxTest001
 * @tc.desc: Test interfaces method of class Tbox
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(UtilityCommonUtilsTest, TboxTest001, testing::ext::TestSize.Level3)
{
    auto tboxSp = make_shared<Tbox>(); // trigger con/destrcutor
    std::string val = "123";
    size_t mask = 1;
    int mode1 = FP_FILE;
    auto ret = Tbox::CalcFingerPrint(val, mask, mode1);
    ASSERT_TRUE(!ret.empty());
    int mode2 = FP_BUFFER;
    ret = Tbox::CalcFingerPrint(val, mask, mode2);
    ASSERT_TRUE(!ret.empty());
    int invalidMode = -1;
    ret = Tbox::CalcFingerPrint(val, mask, invalidMode);
    ASSERT_TRUE(ret == "0");
    std::string stackName = "-_";
    auto ret2 = Tbox::IsCallStack(stackName);
    ASSERT_TRUE(ret2);
    std::string stackName2 = "123";
    auto ret3 = Tbox::IsCallStack(stackName2);
    ASSERT_TRUE(ret3);
    auto ret4 = Tbox::HasCausedBy("Caused by:Invalid description");
    ASSERT_TRUE(ret4);
    auto ret5 = Tbox::HasCausedBy("Suppressed:Invalid description");
    ASSERT_TRUE(ret5);
    auto ret6 = Tbox::HasCausedBy("Invalid description");
    ASSERT_TRUE(!ret6);
}

/* @tc.name: TboxTest002
 * @tc.desc: Test GetStackName method of class Tbox
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(UtilityCommonUtilsTest, TboxTest002, testing::ext::TestSize.Level3)
{
    auto ret = Tbox::GetStackName("    at DES)");
    ASSERT_EQ("DES", ret);
    ret = Tbox::GetStackName("   - DES(");
    ASSERT_EQ("unknown", ret);
    ret = Tbox::GetStackName("   - DE+S(");
    ASSERT_EQ("DE)", ret);
}

/* @tc.name: TboxTest003
 * @tc.desc: Test WaitForDoneFile method of class Tbox
 * @tc.type: FUNC
 * @tc.require: issueI65DUW
 */
HWTEST_F(UtilityCommonUtilsTest, TboxTest003, testing::ext::TestSize.Level3)
{
    (void)FileUtil::ForceRemoveDirectory(LOG_FILE_PATH);
    std::string caseName = "TboxTest003";
    auto ret = Tbox::WaitForDoneFile(GenerateLogFileName(caseName, SUFFIX_0), 1);
    ASSERT_TRUE(!ret);
    FileUtil::SaveStringToFile(GenerateLogFileName(caseName, SUFFIX_0), "test");
    ret = Tbox::WaitForDoneFile(GenerateLogFileName(caseName, SUFFIX_0), 1);
    ASSERT_TRUE(ret);
}
}
}