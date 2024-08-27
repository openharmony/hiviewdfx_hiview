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

#ifndef MOCK_HISYSEVENT_UTIL_MOCK_H_
#define MOCK_HISYSEVENT_UTIL_MOCK_H_

#include <gmock/gmock.h>

#include "hisysevent_util.h"
namespace OHOS {
namespace HiviewDFX {
class MockHisyseventUtil {
public:
    static MockHisyseventUtil& GetInstance();
    MOCK_METHOD0(IsEventProcessed, bool());
private:
    MockHisyseventUtil() = default;
    ~MockHisyseventUtil() = default;
    MockHisyseventUtil& operator=(const MockHisyseventUtil& mockHisyseventUtil) = delete;
    MockHisyseventUtil& operator=(MockHisyseventUtil&& mockHisyseventUtil) = delete;
    MockHisyseventUtil(const MockHisyseventUtil& mockHisyseventUtil) = delete;
    MockHisyseventUtil(MockHisyseventUtil&& mockHisyseventUtil) = delete;
};
}
}

#endif // MOCK_HISYSEVENT_UTIL_MOCK_H_
