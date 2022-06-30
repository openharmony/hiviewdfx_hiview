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
#include "event_pool_test.h"

#include <random>

#include "time_util.h"
using namespace testing::ext;

std::unique_ptr<OHOS::HiviewDFX::EventThreadPool> EventPoolTest::eventThreadPool;
int const EventPoolTest::maxCount;
void EventPoolTest::SetUpTestCase()
{
    printf("SetUpTestCase. Before the first testcase\n");
    eventThreadPool = std::make_unique<OHOS::HiviewDFX::EventThreadPool>(maxCount, "EPTest");
    eventThreadPool->Start();
}

void EventPoolTest::TearDownTestCase()
{
    printf("TearDownTestCase. After the last testcase\n");
    eventThreadPool->Stop();
}

void EventPoolTest::SetUp()
{
    printf("SetUp. Before each testcase\n");
}

void EventPoolTest::TearDown()
{
    printf("TearDown. After each testcase\n");
}

/**
 * @tc.name: EventPoolTest001
 * @tc.desc: 在线程池队列满的时候再添加线程
 * @tc.type: FUNC
 */
HWTEST_F(EventPoolTest, EventPoolTest001, TestSize.Level3)
{
    std::atomic<int> startCount = 0;
    std::atomic<int> endCount = 0;

    auto startTime = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("start time is %s\n", std::to_string(startTime).c_str());
    auto task = [&](int name) {
        printf("start name is %d\n", name);
        ++startCount;
        sleep(5);
        ++endCount;
        printf("end name is %d\n", name);
    };

    for (int i = 0; i < maxCount; ++i) {
        eventThreadPool->AddTask(std::bind(task, i));
    }

    int count = 0;
    while (startCount < maxCount) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // sleep 10ms
        ++count;
        ASSERT_LT(count, 1000); // 1000:大于10s报错退出
    }
    auto time = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("threadPool full time is %s\n", std::to_string(time).c_str());
    for (int i = 0; i < maxCount; ++i) {
        eventThreadPool->AddTask(std::bind(task, i + 10)); // 10 is name
    }
    time = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("AddTask second time is %s\n", std::to_string(time).c_str());

    count = 0;
    while (endCount < maxCount) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // sleep 10ms
        ++count;
        ASSERT_LT(count, 1000); // 1000:大于10s报错退出
    }
    time = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("first end time is %s\n", std::to_string(time).c_str());

    count = 0;
    while (endCount < (maxCount * 2)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // sleep 10ms
        ++count;
        ASSERT_LT(count, 1000); // 1000:大于10s报错退出
    }
    auto end = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("second end time is %s\n", std::to_string(end).c_str());
    printf("all time is %s ms\n", std::to_string(end - startTime).c_str());
    ASSERT_EQ(maxCount * 2, endCount);
    ASSERT_LE(end - startTime, 10200);
}

/**
 * @tc.name: EventPoolTest002
 * @tc.desc: 在线程池队列空的时候再添加线程
 * @tc.type: FUNC
 */
HWTEST_F(EventPoolTest, EventPoolTest002, TestSize.Level3)
{
    std::atomic<int> startCount = 0;
    std::atomic<int> endCount = 0;

    auto startTime = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("start time is %s\n", std::to_string(startTime).c_str());
    auto task = [&](int name) {
        printf("start name is %d\n", name);
        ++startCount;
        sleep(5);
        ++endCount;
        printf("end name is %d\n", name);
    };

    for (int i = 0; i < maxCount; ++i) {
        eventThreadPool->AddTask(std::bind(task, i));
    }

    int count = 0;
    while (startCount < maxCount) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // sleep 10ms
        ++count;
        ASSERT_LT(count, 1000); // 1000:大于10s报错退出
    }

    count = 0;
    while (endCount < maxCount) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // sleep 10ms
        ++count;
        ASSERT_LT(count, 1000); // 1000:大于10s报错退出
    }
    auto time = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("first end time is %s\n", std::to_string(time).c_str());

    sleep(1);
    for (int i = 0; i < maxCount; ++i) {
        eventThreadPool->AddTask(std::bind(task, i + 10)); // 10 is name
    }
    time = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("AddTask second time is %s\n", std::to_string(time).c_str());

    count = 0;
    while (endCount < (maxCount * 2)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // sleep 10ms
        ++count;
        ASSERT_LT(count, 1000); // 1000:大于10s报错退出
    }
    auto end = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("second end time is %s\n", std::to_string(end).c_str());
    printf("all time is %s ms\n", std::to_string(end - startTime).c_str());
    ASSERT_EQ(maxCount * 2, endCount);
    ASSERT_LE(end - startTime, 11200);
}

/**
 * @tc.name: EventPoolTest003
 * @tc.desc: 在线程池队列满的时候再添加少量线程
 * @tc.type: FUNC
 */
HWTEST_F(EventPoolTest, EventPoolTest003, TestSize.Level3)
{
    std::atomic<int> startCount = 0;
    std::atomic<int> endCount = 0;

    auto startTime = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("start time is %s\n", std::to_string(startTime).c_str());
    auto task = [&](int name) {
        printf("start name is %d\n", name);
        ++startCount;
        sleep(5);
        ++endCount;
        printf("end name is %d\n", name);
    };

    for (int i = 0; i < maxCount; ++i) {
        eventThreadPool->AddTask(std::bind(task, i));
    }

    int count = 0;
    while (startCount < maxCount) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // sleep 10ms
        ++count;
        ASSERT_LT(count, 1000); // 1000:大于10s报错退出
    }
    auto time = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("threadPool full time is %s\n", std::to_string(time).c_str());
    int count2 = maxCount - 2;
    for (int i = 0; i < count2; ++i) {
        eventThreadPool->AddTask(std::bind(task, i + 10)); // 10 is name
    }
    time = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("AddTask second time is %s\n", std::to_string(time).c_str());

    count = 0;
    while (endCount < maxCount) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // sleep 10ms
        ++count;
        ASSERT_LT(count, 1000); // 1000:大于10s报错退出
    }
    time = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("first end time is %s\n", std::to_string(time).c_str());

    count = 0;
    while (endCount < (maxCount + count2)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // sleep 10ms
        ++count;
        ASSERT_LT(count, 1000); // 1000:大于10s报错退出
    }
    auto end = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("second end time is %s\n", std::to_string(end).c_str());
    printf("all time is %s ms\n", std::to_string(end - startTime).c_str());
    ASSERT_EQ(maxCount + count2, endCount);
    ASSERT_LE(end - startTime, 10200);
}

/**
 * @tc.name: EventPoolTest004
 * @tc.desc: 随机添加10次事件，每次随机1~10个事件，事件执行时间随机0~9s
 * @tc.type: FUNC
 */
HWTEST_F(EventPoolTest, EventPoolTest004, TestSize.Level3)
{
    std::atomic<uint16_t> startCount = 0;
    std::atomic<uint16_t> endCount = 0;

    std::default_random_engine randEngine;
    std::uniform_int_distribution<uint8_t> randU(0, 9);

    auto startTime = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("start time is %s\n", std::to_string(startTime).c_str());

    auto task = [&](int name) {
        printf("start name is %d\n", name);
        ++startCount;
        sleep(randU(randEngine)); // [0, 9] seconds
        ++endCount;
        printf("end name is %d\n", name);
    };

    uint16_t sumCount = 0;

    for (int i = 0; i < 10; ++i) {
        uint8_t count = randU(randEngine) + 1; // [1, 10] count AddTask
        sumCount += count;
        printf("AddTask: %d count is %u\n", i, count);
        for (int j = 0; j < count; ++j) {
            eventThreadPool->AddTask(std::bind(task, i * 10 + j));
        }
        sleep(randU(randEngine)); // [0, 9] seconds
    }
    auto time0 = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("AddTask finsh time is %s\n", std::to_string(time0).c_str());

    int count = 0;
    while (startCount < sumCount) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // sleep 10ms
        ++count;
        ASSERT_LT(count, sumCount * 100); // 1000:大于10s报错退出
    }
    auto time1 = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("start finsh time is %s\n", std::to_string(time1).c_str());

    count = 0;
    while (endCount < sumCount) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // sleep 10ms
        ++count;
        ASSERT_LT(count, sumCount * 100); // 1000:大于10s报错退出
    }

    auto end = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();
    printf("end time is %s\n", std::to_string(end).c_str());
    printf("all time is %s ms\n", std::to_string(end - startTime).c_str());
    ASSERT_EQ(sumCount, endCount);
}