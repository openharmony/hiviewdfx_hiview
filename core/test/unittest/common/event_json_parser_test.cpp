/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "event_json_parser_test.h"

#include <iostream>

#include "event_json_parser.h"
#include "hiview_platform.h"

namespace OHOS {
namespace HiviewDFX {
void EventJsonParserTest::SetUpTestCase() {}

void EventJsonParserTest::TearDownTestCase() {}

void EventJsonParserTest::SetUp()
{
    HiviewPlatform &platform = HiviewPlatform::GetInstance();
    if (!platform.InitEnvironment()) {
        std::cout << "fail to init environment" << std::endl;
    } else {
        std::cout << "init environment successful" << std::endl;
    }
}

void EventJsonParserTest::TearDown() {}

/**
 * @tc.name: EventJsonParserTest001
 * @tc.desc: parse a event and check Json info
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(EventJsonParserTest, EventJsonParserTest001, testing::ext::TestSize.Level0)
{
    printf("start EventJsonParserTest001\n");
    std::string defFile = "/system/etc/hiview/hisysevent.def";
    std::vector<std::string> defFiles;
    defFiles.emplace_back(defFile);
    auto sysEventParser = std::make_unique<EventJsonParser>(defFiles);

    std::shared_ptr<SysEvent> sysEvent = nullptr;
    ASSERT_FALSE(sysEventParser->HandleEventJson(sysEvent));
    constexpr char invalidJsonStr[] = "{\"domain_\":\"HIVIEWDFX\",\"type_\":4,\
        \"PARAM_A\":\"param a\",\"PARAM_B\":\"param b\"}";
    sysEvent = std::make_shared<SysEvent>("SysEventService", nullptr, invalidJsonStr);
    ASSERT_FALSE(sysEventParser->HandleEventJson(sysEvent));
    constexpr char jsonStr[] = "{\"domain_\":\"HIVIEWDFX\",\"name_\":\"PLUGIN_LOAD\",\"type_\":4,\
        \"PARAM_A\":\"param a\",\"PARAM_B\":\"param b\"}";
    sysEvent = std::make_shared<SysEvent>("SysEventService", nullptr, jsonStr);

    ASSERT_TRUE(sysEventParser->HandleEventJson(sysEvent));
    ASSERT_TRUE(sysEventParser->GetTagByDomainAndName("abc", "abc") == "");
    ASSERT_TRUE(sysEventParser->GetTagByDomainAndName("DEMO", "abc") == "");
    ASSERT_TRUE(sysEventParser->GetTypeByDomainAndName("DEMO", "abc") == 0);

    DuplicateIdFilter filter;
    ASSERT_FALSE(filter.IsDuplicateEvent(0));
    ASSERT_FALSE(filter.IsDuplicateEvent(1));
    ASSERT_TRUE(filter.IsDuplicateEvent(1));
}
} // namespace HiviewDFX
} // namespace OHOS
