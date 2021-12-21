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

#include "event_service_action_test.h"

#include "event.h"
#include "hiview_platform.h"
#include "logger.h"
#include "sys_event.h"
#include "sys_event_service.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("EventServiceActionTest");
void EventServiceActionTest::SetUpTestCase()
{
    OHOS::HiviewDFX::HiviewPlatform &platform = HiviewPlatform::GetInstance();
    if (!platform.InitEnvironment("")) {
        HIVIEW_LOGE("Fail to init environment.");
    }
}

void EventServiceActionTest::SetUp() {}

void EventServiceActionTest::TearDown() {}

/**
 * @tc.name: EventServiceActionTest001
 * @tc.desc: parse a event and check Json info
 * @tc.type: FUNC
 * @tc.require: SR000GGSVB
 */
HWTEST_F(EventServiceActionTest, EventServiceActionTest001, testing::ext::TestSize.Level3)
{
    printf("start EventServiceActionTest\n");
    constexpr char JSON_STR[] = "{\"domain_\":\"HIVIEWDFX\",\"name_\":\"APP_ACTIVATION_DATA\",\"type_\":1,\
        \"PNAMEID\":\"jng\",\"STACKSS\":\"{1,3,4,6}\"}";
    auto eventService = std::make_unique<SysEventService>();
    auto sysEvent = std::make_shared<SysEvent>("SysEventService", nullptr, JSON_STR);
    auto event = std::dynamic_pointer_cast<Event>(sysEvent);
    event->messageType_ = Event::MessageType::SYS_EVENT;
    eventService->OnLoad();
    ASSERT_TRUE(eventService->OnEvent(event));
}
} // namespace HiviewDFX
} // namespace OHOS