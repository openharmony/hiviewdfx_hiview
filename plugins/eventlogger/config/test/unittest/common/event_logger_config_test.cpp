/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "event_logger_config_test.h"

#include <iostream>
#include <memory>

#include "event_logger_config.h"
using namespace testing::ext;
using namespace OHOS::HiviewDFX;

void EventLoggerConfigTest::SetUp()
{
    /**
     * @tc.setup: create an event loop and multiple event handlers
     */
    printf("SetUp.\n");
}

void EventLoggerConfigTest::TearDown()
{
    /**
     * @tc.teardown: destroy the event loop we have created
     */
    printf("TearDown.\n");
}

/**
 * @tc.name: EventLoggerConfigTest002
 * @tc.desc: parse a incorrect config file and check result
 * @tc.type: FUNC
 * @tc.require: AR000FT621
 */
HWTEST_F(EventLoggerConfigTest, EventLoggerConfigTest002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create event handler and events
     */
    auto config = std::make_unique<EventLoggerConfig>("/data/test/test_data/event_logger_config");
    EventLoggerConfig::EventLoggerConfigData configOut;
    config->FindConfigLine(0, "FWK_BLOCK", configOut);
}
