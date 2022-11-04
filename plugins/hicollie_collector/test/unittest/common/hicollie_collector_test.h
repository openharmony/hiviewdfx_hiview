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
#ifndef HIVIEW_HICOLLIE_COLLECTOR_TEST_H
#define HIVIEW_HICOLLIE_COLLECTOR_TEST_H
#include <memory>
#include <string>
#include <sys/types.h>

#include <gtest/gtest.h>
namespace OHOS {
namespace HiviewDFX {
class HicollieCollectorTest : public testing::Test {
public:
    void SetUp();

    void TearDown();

    static void SetUpTestCase();

    static void TearDownTestCase();

    bool SendEvent(uint64_t time, const std::string& name);

    bool GetHicollieCollectorTest001File(uint64_t time1, uint64_t time2);

    bool GetHicollieCollectorTest002File(uint64_t time);
};
}
}
#endif // HIVIEW_HICOLLIE_COLLECTOR_TEST_H