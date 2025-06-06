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
#ifndef EVENT_LOGGER_CATCHER_TEST_H
#define EVENT_LOGGER_CATCHER_TEST_H
#include <string>
#include <sys/types.h>

#include <gtest/gtest.h>
namespace OHOS {
namespace HiviewDFX {
class EventloggerCatcherTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
private:
    std::string path_ = "/data/test/log";
    std::string logFile_ = "";
    const int RETURN_OPEN_FAIL = -1;
    const int RETURN_TASK_NO_SUCCESS = -2;
    const int RETURN_TASK_LESS_THAN_SIZE = -3;
    const int RETURN_OPEN_FAIL2 = -4;
    const int RETURN_LESS_THAN_SIZE = -5;

    bool isSelinuxEnabled_ = false;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // EVENT_LOGGER_CATCHER_TEST_H