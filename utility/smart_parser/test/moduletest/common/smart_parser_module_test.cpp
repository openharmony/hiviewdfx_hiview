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

#include "smart_parser_module_test.h"

#include <cstdio>
#include <ctime>
#include <errors.h>
#include <map>
#include <string>

#include "feature_analysis.h"
#include "file_util.h"
#include "log_util.h"
#include "smart_parser.h"
#include "string_util.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
using namespace testing::ext;
static const std::string TEST_CONFIG = "/data/test/test_data/SmartParser/common/";

void SmartParserModuleTest::SetUpTestCase(void) {}

void SmartParserModuleTest::TearDownTestCase(void) {}

void SmartParserModuleTest::SetUp(void) {}

void SmartParserModuleTest::TearDown(void) {}

/**
 * @tc.name: SmartParserTest001
 * @tc.desc: process cpp_crash fault, this case match FeatureListForReliability.Json
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: liuwei
 */
HWTEST_F(SmartParserModuleTest, SmartParserTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set taskSheet fault log path and eventid.
     */
    std::string faultFile = LogUtil::SMART_PARSER_TEST_DIR +
                            "/SmartParserTest001/cppcrash-com.ohos.launcher-20010025-19700324235211";
    std::string trustStack = LogUtil::SMART_PARSER_TEST_DIR + "/SmartParserTest001/trace.txt";

    ASSERT_EQ(FileUtil::FileExists(faultFile), true);
    ASSERT_EQ(FileUtil::FileExists(trustStack), true);

    /**
     * @tc.steps: step2. smart parser process fault log
     */
    auto eventInfos = SmartParser::Analysis(faultFile, TEST_CONFIG, "CPP_CRASH");

    /**
     * @tc.steps: step3. check the result of eventinfo for fault.
     * @tc.expected: step3. equal to correct answer.
     */
    EXPECT_STREQ(eventInfos["PNAME"].c_str(), "com.ohos.launcher");
    EXPECT_EQ(eventInfos["END_STACK"].size() > 0, true);
    std::stringstream buff;
    LogUtil::ReadFileBuff(trustStack, buff);
    std::vector<std::string> trace;
    StringUtil::SplitStr(eventInfos["END_STACK"], LogUtil::SPLIT_PATTERN, trace, false, false);
    std::string line;
    size_t num = 0;
    while (getline(buff, line) && num < trace.size()) {
        EXPECT_STREQ(line.c_str(), trace[num++].c_str());
    }
}

/**
 * @tc.name: SmartParserTest002
 * @tc.desc: process JS_ERROR fault, this case match FeatureListForReliability.Json
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: liuwei
 */
HWTEST_F(SmartParserModuleTest, SmartParserTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set taskSheet fault log path and eventid.
     */
    std::string faultFile = LogUtil::SMART_PARSER_TEST_DIR +
                            "/SmartParserTest002/jscrash-com.example.jsinject-20010041-19700424183123";
    std::string trustStack = LogUtil::SMART_PARSER_TEST_DIR + "/SmartParserTest002/trace.txt";
    ASSERT_EQ(FileUtil::FileExists(faultFile), true);
    ASSERT_EQ(FileUtil::FileExists(trustStack), true);

    /**
     * @tc.steps: step2. smart parser process fault log
     */
    auto eventInfos = SmartParser::Analysis(faultFile, TEST_CONFIG, "JS_ERROR");

    /**
     * @tc.steps: step3. check the result of eventinfo for fault.
     * @tc.expected: step3. equal to correct answer.
     */
    EXPECT_STREQ(eventInfos["PNAME"].c_str(), "com.example.jsinject");
    EXPECT_EQ(eventInfos["END_STACK"].size() > 0, true);
    std::stringstream buff;
    LogUtil::ReadFileBuff(trustStack, buff);
    std::vector<std::string> trace;
    StringUtil::SplitStr(eventInfos["END_STACK"], LogUtil::SPLIT_PATTERN, trace, false, false);
    std::string line;
    size_t num = 0;
    while (getline(buff, line) && num < trace.size()) {
        EXPECT_STREQ(line.c_str(), trace[num++].c_str());
    }
}

/**
 * @tc.name: SmartParserTest003
 * @tc.desc: process freeze fault, this case match FeatureListForReliability.Json
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: liuwei
 */
HWTEST_F(SmartParserModuleTest, SmartParserTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set taskSheet fault log path and eventid.
     */
    std::string faultFile = LogUtil::SMART_PARSER_TEST_DIR +
                            "/SmartParserTest003/appfreeze-com.example.jsinject-20010039-19700326211815";
    std::string traceFile = LogUtil::SMART_PARSER_TEST_DIR + "/SmartParserTest003/trace.txt";
    ASSERT_EQ(FileUtil::FileExists(faultFile), true);
    ASSERT_EQ(FileUtil::FileExists(traceFile), true);

    /**
     * @tc.steps: step2. smart parser process crash fault log
     */
    auto eventInfos = SmartParser::Analysis(faultFile, TEST_CONFIG, "APP_FREEZE");

    /**
     * @tc.steps: step3. check the result of eventinfo for fault.
     * @tc.expected: step3. equal to correct answer.
     */
    EXPECT_EQ(eventInfos["END_STACK"].size() > 0, true);
    std::string content;
    if (!FileUtil::LoadStringFromFile(traceFile, content)) {
        printf("read logFile: %s failed", traceFile.c_str());
        return;
    }
    std::stringstream buff(content);
    std::vector<std::string> trace;
    StringUtil::SplitStr(eventInfos["END_STACK"], LogUtil::SPLIT_PATTERN, trace, false, false);
    std::string line;
    size_t num = 0;
    while (getline(buff, line) && num < trace.size()) {
        EXPECT_STREQ(trace[num++].c_str(), line.c_str());
    }
}

/**
 * @tc.name: SmartParserTest004
 * @tc.desc: process PANIC fault, this case match FeatureAnalysisForRebootsys.Json.
 *           1. fault log should can be read;
 *           2. FeatureAnalysisForRebootsys.Json should match the json file in perforce.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: liuwei
 */
HWTEST_F(SmartParserModuleTest, SmartParserTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set taskSheet fault log path and eventid.
     */
    std::string faultFile = LogUtil::SMART_PARSER_TEST_DIR + "/SmartParserTest004/dmesg-ramoops-0";
    std::string traceFile = LogUtil::SMART_PARSER_TEST_DIR + "/SmartParserTest004/trace.txt";
    ASSERT_EQ(FileUtil::FileExists(faultFile), true);
    ASSERT_EQ(FileUtil::FileExists(traceFile), true);
    std::stringstream buff;
    LogUtil::ReadFileBuff(traceFile, buff);

    /**
     * @tc.steps: step2. smart parser process crash fault log
     */
    auto eventInfos = SmartParser::Analysis(faultFile, TEST_CONFIG, "PANIC");

    std::vector<std::string> trace;
    StringUtil::SplitStr(eventInfos["END_STACK"], LogUtil::SPLIT_PATTERN, trace, false, false);
    std::string line;
    size_t num = 0;
    while (getline(buff, line) && num < trace.size()) {
        EXPECT_STREQ(trace[num++].c_str(), line.c_str());
    }
}

/**
 * @tc.name: SmartParserTest005
 * @tc.desc: process HWWATCHDOG fault, this case match FeatureAnalysisForRebootsys.Json.
 *           1. fault log should can be read;
 *           2. FeatureAnalysisForRebootsys.Json should match the json file in perforce.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: liuwei
 */
HWTEST_F(SmartParserModuleTest, SmartParserTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set taskSheet fault log path and eventid.
     */
    std::string faultFile = LogUtil::SMART_PARSER_TEST_DIR + "/SmartParserTest005/dmesg-ramoops-0";
    std::string traceFile = LogUtil::SMART_PARSER_TEST_DIR + "/SmartParserTest005/trace.txt";
    ASSERT_EQ(FileUtil::FileExists(faultFile), true);
    ASSERT_EQ(FileUtil::FileExists(traceFile), true);
    std::stringstream buff;
    LogUtil::ReadFileBuff(traceFile, buff);

    /**
     * @tc.steps: step2. smart parser process crash fault log
     */
    auto eventInfos = SmartParser::Analysis(faultFile, TEST_CONFIG, "HWWATCHDOG");

    std::vector<std::string> trace;
    StringUtil::SplitStr(eventInfos["END_STACK"], LogUtil::SPLIT_PATTERN, trace, false, false);
    std::string line;
    size_t num = 0;
    while (getline(buff, line) && num < trace.size()) {
        EXPECT_STREQ(trace[num++].c_str(), line.c_str());
    }
}

/**
 * @tc.name: SmartParserTest006
 * @tc.desc: process HWWATCHDOG fault, this case match FeatureAnalysisForRebootsys.Json.
 *           1. fault log should can be read;
 *           2. FeatureAnalysisForRebootsys.Json should match the json file in perforce.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: liuwei
 */
HWTEST_F(SmartParserModuleTest, SmartParserTest006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set taskSheet fault log path and eventid.
     */
    std::string faultFile = LogUtil::SMART_PARSER_TEST_DIR + "/SmartParserTest005/dmesg-ramoops-1";
    ASSERT_EQ(FileUtil::FileExists(faultFile), false);

    /**
     * @tc.steps: step2. smart parser process crash fault log
     */
    auto eventInfos = SmartParser::Analysis(faultFile, TEST_CONFIG, "HWWATCHDOG");
    ASSERT_EQ(eventInfos.empty(), true);
}

/**
 * @tc.name: SmartParserTest007
 * @tc.desc: process test fault, this case match FeatureAnalysisForRebootsys.Json.
 *           1. fault log should can be read;
 *           2. FeatureAnalysisForRebootsys.Json should match the json file in perforce.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author: liuwei
 */
HWTEST_F(SmartParserModuleTest, SmartParserTest007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Set taskSheet fault log path and eventid.
     */
    auto eventInfos = SmartParser::Analysis("", "/system/etc/hiview/reliability", "TEST");
    ASSERT_EQ(eventInfos.empty(), true);
    eventInfos = SmartParser::Analysis("", "", "TEST");
    ASSERT_EQ(eventInfos.empty(), true);
    eventInfos = SmartParser::Analysis("", "", "");
    ASSERT_EQ(eventInfos.empty(), true);
    eventInfos = SmartParser::Analysis("test", "test", "test");
    ASSERT_EQ(eventInfos.empty(), true);
}
}  // namespace HiviewDFX
}  // namespace OHOS
