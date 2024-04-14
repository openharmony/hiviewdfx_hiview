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

#include "event_export_config_test.h"

#include <vector>

#include "export_config_manager.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
// all value is from resource file: test_event_export_config.json and test_events.json
constexpr char TEST_CONFIG_DIR[] = "/data/test/test_data/";
constexpr char TEST_CONFIG_FILE[] = "/data/test/test_data/test_event_export_config.json";
constexpr char TEST_MODULE_NAME[] = "test";
constexpr char TEST_SETTING_DB_PARAM_NAME[] = "test_param_key";
constexpr char TEST_SETTING_DB_PARAM_ENABLED_VAL[] = "1";
constexpr char TEST_SETTING_DB_PARAM_DISABLED_VAL[] = "0";
constexpr size_t TEST_MODULE_CNT = 1;
constexpr int64_t TEST_CAPACITY = 100;
constexpr int64_t TEST_SIZE = 2;
constexpr int64_t TEST_CYCLE = 3600;
constexpr int64_t TEST_FILE_STORE_DAY_CNT = 7;
constexpr size_t TEST_EXPORT_DOMAIN_CNT = 2;
constexpr size_t TEST_EXPORT_NAME_CNT = 3;
}
void EventExportConfigParseTest::SetUpTestCase()
{
}

void EventExportConfigParseTest::TearDownTestCase()
{
}

void EventExportConfigParseTest::SetUp()
{
}

void EventExportConfigParseTest::TearDown()
{
}

/**
 * @tc.name: EventExportConfigParseTest001
 * @tc.desc: EventConfigManager test
 * @tc.type: FUNC
 * @tc.require: issueI9E8HA
 */
HWTEST_F(EventExportConfigParseTest, EventExportConfigParseTest001, testing::ext::TestSize.Level3)
{
    ExportConfigManager manager(TEST_CONFIG_DIR);
    std::vector<std::string> moduleNames;
    manager.GetModuleNames(moduleNames);
    ASSERT_EQ(moduleNames.size(), TEST_MODULE_CNT);
    auto testModuleName = moduleNames.at(0);
    ASSERT_EQ(testModuleName, TEST_MODULE_NAME);
    auto exportConfig = manager.GetExportConfig(testModuleName);
    ASSERT_NE(exportConfig, nullptr);
    std::vector<std::shared_ptr<ExportConfig>> configs;
    manager.GetExportConfigs(configs);
    ASSERT_EQ(configs.size(), TEST_MODULE_CNT);
}

/**
 * @tc.name: EventExportConfigParseTest002
 * @tc.desc: EventConfigParser test
 * @tc.type: FUNC
 * @tc.require: issueI9E8HA
 */
HWTEST_F(EventExportConfigParseTest, EventExportConfigParseTest002, testing::ext::TestSize.Level3)
{
    ExportConfigParser parser(TEST_CONFIG_FILE);
    auto exportConfig = parser.Parse();
    ASSERT_NE(exportConfig, nullptr);
    ASSERT_EQ(exportConfig->moduleName, ""); // parser not set module name to config
    ASSERT_EQ(exportConfig->exportDir, "/data/test/test_data");
    ASSERT_EQ(exportConfig->maxCapcity, TEST_CAPACITY);
    ASSERT_EQ(exportConfig->maxSize, TEST_SIZE);
    ASSERT_EQ(exportConfig->taskCycle, TEST_CYCLE);
    ASSERT_EQ(exportConfig->dayCnt, TEST_FILE_STORE_DAY_CNT);
}

/**
 * @tc.name: EventExportConfigParseTest003
 * @tc.desc: EventExportList test
 * @tc.type: FUNC
 * @tc.require: issueI9E8HA
 */
HWTEST_F(EventExportConfigParseTest, EventExportConfigParseTest003, testing::ext::TestSize.Level3)
{
    ExportConfigParser parser(TEST_CONFIG_FILE);
    auto exportConfig = parser.Parse();
    ASSERT_NE(exportConfig, nullptr);
    ASSERT_EQ(exportConfig->eventList.size(), TEST_EXPORT_DOMAIN_CNT);
    auto iter = exportConfig->eventList.find("DOMAIN1");
    ASSERT_NE(iter, exportConfig->eventList.end());
    ASSERT_EQ(iter->second.size(), TEST_EXPORT_NAME_CNT);
    iter = exportConfig->eventList.find("DOMAIN2");
    ASSERT_NE(iter, exportConfig->eventList.end());
    ASSERT_EQ(iter->second.size(), TEST_EXPORT_NAME_CNT);
}

/**
 * @tc.name: EventExportConfigParseTest004
 * @tc.desc: SettingDbParam test
 * @tc.type: FUNC
 * @tc.require: issueI9E8HA
 */
HWTEST_F(EventExportConfigParseTest, EventExportConfigParseTest004, testing::ext::TestSize.Level3)
{
    ExportConfigParser parser(TEST_CONFIG_FILE);
    auto exportConfig = parser.Parse();
    ASSERT_NE(exportConfig, nullptr);
    ASSERT_EQ(exportConfig->settingDbParam.paramName, TEST_SETTING_DB_PARAM_NAME);
    ASSERT_EQ(exportConfig->settingDbParam.enabledVal, TEST_SETTING_DB_PARAM_ENABLED_VAL);
    ASSERT_EQ(exportConfig->settingDbParam.disabledVal, TEST_SETTING_DB_PARAM_DISABLED_VAL);
}
} // namespace HiviewDFX
} // namespace OHOS