/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <string>
#include <vector>

#include "smart_parser.h"
#include "tbox.h"
#include "log_util.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace HiviewDFX {

/**
 * @tc.name: SmartParserRgmManagerSysfreezeTest001
 * @tc.desc: Smart parser rgm_manager sysfreeze log, phase = 0.
 * @tc.type: FUNC
 */
HWTEST(SmartParserRgmManagerSysfreezeTest, SmartParserRgmManagerSysfreezeTest001, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. parser log
     */
    const std::string logPath = std::string{LogUtil::SMART_PARSER_TEST_DIR}  +
        "/SmartParserRgmManagerSysfreezeTest001/sysfreeze-rgm_manager-5002-20250710185758-1752145078457.tmp";
    const std::string eventType = "SYS_FREEZE";
    auto eventInfos = SmartParser::Analysis(logPath, SMART_PARSER_PATH, eventType);

    /**
     * @tc.steps: step2. check result
     */
    ASSERT_EQ(eventInfos["FIRST_FRAME"], "0");
}

/**
 * @tc.name: SmartParserRgmManagerSysfreezeTest002
 * @tc.desc: Smart parser rgm_manager sysfreeze log, phase = 100.
 * @tc.type: FUNC
 */
HWTEST(SmartParserRgmManagerSysfreezeTest, SmartParserRgmManagerSysfreezeTest002, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. parser log
     */
    const std::string logPath = std::string{LogUtil::SMART_PARSER_TEST_DIR}  +
        "/SmartParserRgmManagerSysfreezeTest002/sysfreeze-rgm_manager-5002-20250710185758-1752145078457.tmp";
    const std::string eventType = "SYS_FREEZE";
    auto eventInfos = SmartParser::Analysis(logPath, SMART_PARSER_PATH, eventType);

    /**
     * @tc.steps: step2. check result
     */
    ASSERT_EQ(eventInfos["FIRST_FRAME"], "100");
}

/**
 * @tc.name: SmartParserRgmManagerSysfreezeTest003
 * @tc.desc: Smart parser rgm_manager sysfreeze log, phase = 101.
 * @tc.type: FUNC
 */
HWTEST(SmartParserRgmManagerSysfreezeTest, SmartParserRgmManagerSysfreezeTest003, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. parser log
     */
    const std::string logPath = std::string{LogUtil::SMART_PARSER_TEST_DIR}  +
        "/SmartParserRgmManagerSysfreezeTest003/sysfreeze-rgm_manager-5002-20250710185758-1752145078457.tmp";
    const std::string eventType = "SYS_FREEZE";
    auto eventInfos = SmartParser::Analysis(logPath, SMART_PARSER_PATH, eventType);

    /**
     * @tc.steps: step2. check result
     */
    ASSERT_EQ(eventInfos["FIRST_FRAME"], "101");
}

/**
 * @tc.name: SmartParserRgmManagerSysfreezeTest004
 * @tc.desc: Smart parser rgm_manager sysfreeze log, phase = 102.
 * @tc.type: FUNC
 */
HWTEST(SmartParserRgmManagerSysfreezeTest, SmartParserRgmManagerSysfreezeTest004, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. parser log
     */
    const std::string logPath = std::string{LogUtil::SMART_PARSER_TEST_DIR}  +
        "/SmartParserRgmManagerSysfreezeTest004/sysfreeze-rgm_manager-5002-20250710185758-1752145078457.tmp";
    const std::string eventType = "SYS_FREEZE";
    auto eventInfos = SmartParser::Analysis(logPath, SMART_PARSER_PATH, eventType);

    /**
     * @tc.steps: step2. check result
     */
    ASSERT_EQ(eventInfos["FIRST_FRAME"], "102");
}

/**
 * @tc.name: SmartParserRgmManagerSysfreezeTest005
 * @tc.desc: Smart parser rgm_manager sysfreeze log, phase = 700.
 * @tc.type: FUNC
 */
HWTEST(SmartParserRgmManagerSysfreezeTest, SmartParserRgmManagerSysfreezeTest005, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. parser log
     */
    const std::string logPath = std::string{LogUtil::SMART_PARSER_TEST_DIR}  +
        "/SmartParserRgmManagerSysfreezeTest005/sysfreeze-rgm_manager-5002-20250710185758-1752145078457.tmp";
    const std::string eventType = "SYS_FREEZE";
    auto eventInfos = SmartParser::Analysis(logPath, SMART_PARSER_PATH, eventType);

    /**
     * @tc.steps: step2. check result
     */
    ASSERT_EQ(eventInfos["FIRST_FRAME"], "700");
}

/**
 * @tc.name: SmartParserRgmManagerSysfreezeTest006
 * @tc.desc: Smart parser rgm_manager sysfreeze log, phase = 1000.
 * @tc.type: FUNC
 */
HWTEST(SmartParserRgmManagerSysfreezeTest, SmartParserRgmManagerSysfreezeTest006, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. parser log
     */
    const std::string logPath = std::string{LogUtil::SMART_PARSER_TEST_DIR}  +
        "/SmartParserRgmManagerSysfreezeTest006/sysfreeze-rgm_manager-5002-20250710185758-1752145078457.tmp";
    const std::string eventType = "SYS_FREEZE";
    auto eventInfos = SmartParser::Analysis(logPath, SMART_PARSER_PATH, eventType);

    /**
     * @tc.steps: step2. check result
     */
    ASSERT_EQ(eventInfos["FIRST_FRAME"], "1000");
}

/**
 * @tc.name: SmartParserRgmManagerSysfreezeTest007
 * @tc.desc: Smart parser rgm_manager sysfreeze log, phase = 2000.
 * @tc.type: FUNC
 */
HWTEST(SmartParserRgmManagerSysfreezeTest, SmartParserRgmManagerSysfreezeTest007, testing::ext::TestSize.Level3)
{
    /**
     * @tc.steps: step1. parser log
     */
    const std::string logPath = std::string{LogUtil::SMART_PARSER_TEST_DIR}  +
        "/SmartParserRgmManagerSysfreezeTest007/sysfreeze-rgm_manager-5002-20250710185758-1752145078457.tmp";
    const std::string eventType = "SYS_FREEZE";
    auto eventInfos = SmartParser::Analysis(logPath, SMART_PARSER_PATH, eventType);

    /**
     * @tc.steps: step2. check result
     */
    ASSERT_EQ(eventInfos["FIRST_FRAME"], "2000");
}

} // namespace HiviewDFX
} // namespace OHOS
