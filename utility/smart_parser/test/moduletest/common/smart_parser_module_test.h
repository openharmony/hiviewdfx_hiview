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
#ifndef SMART_PARSER_MODULE_TEST_H
#define SMART_PARSER_MODULE_TEST_H

#include <gtest/gtest.h>
#include <string>

namespace OHOS {
namespace HiviewDFX {
static const std::string TEST_CONFIG = "/data/test/test_data/SmartParser/common/";

class SmartParserModuleTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::string GetTraceParam(std::map<std::string, std::string>& eventInfo) const;
};
}  // namespace HiviewDFX
}  // namespace OHOS

#endif // SMART_PARSER_MODULE_TEST_H
