/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef HIVIEW_BASE_EVENT_BASE_TEST_UNITTEST_COMMON
#define HIVIEW_BASE_EVENT_BASE_TEST_UNITTEST_COMMON

#include <gtest/gtest.h>

#include "event.h"

namespace OHOS {
namespace HiviewDFX {
class EventBaseTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

class TestEventHandler : public EventHandler {
public:
    bool OnEvent(std::shared_ptr<OHOS::HiviewDFX::Event>& event) override;
};

class TestEventListener : public EventListener {
public:
    void OnUnorderedEvent(const Event& msg) override;
    std::string GetListenerName() override;
};
}  // namespace HiviewDFX
}  // namespace OHOS

#endif  // HIVIEW_BASE_EVENT_BASE_TEST_UNITTEST_COMMON
