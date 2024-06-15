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
#include "param_update_test.h"

#include "file_util.h"
#include "param_manager.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
    const std::string TEST_CONFIG_FILE = "/data/system/hiview/test.txt";
    const std::string TEST_ANCO_CONFIG_PATH = "/data/system/hiview/anco/";
}

void ParamUpdateTest::SetUp()
{
    /**
     * @tc.setup: create an event loop and multiple event handlers
     */
    std::string configDir("/data/test/test_data/hiview_platform_config");
    if (!platform.InitEnvironment(configDir)) {
        std::cout << "fail to init environment" << std::endl;
    } else {
        std::cout << "init environment successful" << std::endl;
    }
}

void ParamUpdateTest::TearDown()
{
    /**
     * @tc.teardown: destroy the event loop we have created
     */
    FileUtil::RemoveFile(TEST_CONFIG_FILE);
    FileUtil::ForceRemoveDirectory(TEST_ANCO_CONFIG_PATH);
}

/**
 * @tc.name: PlatformConfigParse001
 * @tc.desc: parse a correct config file and check result
 * @tc.type: FUNC
 * @tc.require: AR000DPTT5
 */
HWTEST_F(ParamUpdateTest, ParamUpdateTest001, TestSize.Level1)
{
    ParamManager::InitParam();
    ASSERT_TRUE(FileUtil::FileExists(TEST_CONFIG_FILE));
}