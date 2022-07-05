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
#ifndef EVENT_LOGGER_EVENT_POOL_TEST_H
#define EVENT_LOGGER_EVENT_POOL_TEST_H
#include <memory>
#include <string>
#include <sys/types.h>

#include <gtest/gtest.h>

#include "event_thread_pool.h"
class EventPoolTest : public testing::Test {
public:
    EventPoolTest()
    {
        printf("EventPoolTest::EventPoolTest()\n");
    }
    ~EventPoolTest()
    {
        printf("EventPoolTest::~EventPoolTest()\n");
    }
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    static int const maxCount = 5;
    static std::unique_ptr<OHOS::HiviewDFX::EventThreadPool> eventThreadPool;
};
#endif // EVENT_LOGGER_EVENT_POOL_TEST_H