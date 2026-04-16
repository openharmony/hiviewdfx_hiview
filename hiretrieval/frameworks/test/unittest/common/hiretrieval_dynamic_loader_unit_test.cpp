/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "hiretrieval_dynamic_loader_unit_test.h"

#include <gmock/gmock.h>

#include "hiretrieval_dynamic_loader.h"

namespace OHOS {
namespace HiviewDFX {
void HiRetrievalDynamicLoaderUnitTest::SetUpTestCase()
{
}

void HiRetrievalDynamicLoaderUnitTest::TearDownTestCase()
{
}

void HiRetrievalDynamicLoaderUnitTest::SetUp()
{
}

void HiRetrievalDynamicLoaderUnitTest::TearDown()
{
}

/**
 * @tc.name: HiRetrievalDynamicLoaderUnitTest001
 * @tc.desc: test the apis of class DynamicLoader
 * @tc.type: FUNC
 * @tc.require: issueIBT9BB
 */
HWTEST_F(HiRetrievalDynamicLoaderUnitTest, HiRetrievalDynamicLoaderUnitTest001, testing::ext::TestSize.Level3)
{
    HiRetrievalConfig cfg {
        "user_type",
        "device_type",
        "device_model",
    };
    auto dllLoader = std::make_unique<HiRetrievalDynamicLoader>();
    ASSERT_NE(dllLoader, nullptr);
    ASSERT_EQ(dllLoader->Participate(cfg), HiRetrieval::NativeErrorCode::NOT_INIT);
    ASSERT_EQ(dllLoader->Quit(), HiRetrieval::NativeErrorCode::NOT_INIT);
    ASSERT_EQ(dllLoader->Run(), HiRetrieval::NativeErrorCode::NOT_INIT);
    ASSERT_EQ(dllLoader->Init(), HiRetrieval::NativeErrorCode::SUCC);
    ASSERT_EQ(dllLoader->Participate(cfg), HiRetrieval::NativeErrorCode::SUCC);
    ASSERT_EQ(dllLoader->Quit(), HiRetrieval::NativeErrorCode::SUCC);
    ASSERT_EQ(dllLoader->Run(), HiRetrieval::NativeErrorCode::SUCC);
}
} // namespace HiviewDFX
} // namespace OHOS