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
#include "freeze_detector_unittest.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <unistd.h>

#include "event.h"
#include "file_util.h"
#include "time_util.h"

#include "sys_event.h"

#include "freeze_common.h"
#include "rule_cluster.h"
#include "resolver.h"
#include "watch_point.h"

using namespace testing::ext;
namespace OHOS {
namespace HiviewDFX {
void FreezeDetectorUnittest::SetUp()
{
    /**
     * @tc.setup: create work directories
     */
    printf("SetUp.\n");
}
void FreezeDetectorUnittest::SetUpTestCase()
{
    /**
     * @tc.setup: all first
     */
    printf("SetUpTestCase.\n");
}

void FreezeDetectorUnittest::TearDownTestCase()
{
    /**
     * @tc.setup: all end
     */
    printf("TearDownTestCase.\n");
}

void FreezeDetectorUnittest::TearDown()
{
    /**
     * @tc.teardown: destroy the event loop we have created
     */
    printf("TearDown.\n");
}

/**
 * @tc.name: FreezeResolver_001
 * @tc.desc: FreezeDetector
 */
HWTEST_F(FreezeDetectorUnittest, FreezeResolver_001, TestSize.Level3)
{
    FreezeResolver freezeResolver(nullptr);
    ASSERT_EQ(freezeResolver.Init(), false);
}

/**
 * @tc.name: FreezeResolver_002
 * @tc.desc: FreezeDetector
 */
HWTEST_F(FreezeDetectorUnittest, FreezeResolver_002, TestSize.Level3)
{
    auto freezeCommon = std::make_shared<FreezeCommon>();
    auto freezeResolver = std::make_unique<FreezeResolver>(freezeCommon);
    ASSERT_EQ(freezeResolver->Init(), false);
}

/**
 * @tc.name: FreezeResolver_003
 * @tc.desc: FreezeDetector
 */
HWTEST_F(FreezeDetectorUnittest, FreezeResolver_003, TestSize.Level3)
{
    auto freezeCommon = std::make_shared<FreezeCommon>();
    bool ret1 = freezeCommon->Init();
    ASSERT_EQ(ret1, true);
    auto freezeResolver = std::make_unique<FreezeResolver>(freezeCommon);
    ASSERT_EQ(freezeResolver->Init(), true);
}

/**
 * @tc.name: FreezeResolver_004
 * @tc.desc: FreezeDetector
 */
HWTEST_F(FreezeDetectorUnittest, FreezeResolver_004, TestSize.Level3)
{
    auto freezeCommon = std::make_shared<FreezeCommon>();
    bool ret1 = freezeCommon->Init();
    ASSERT_EQ(ret1, true);
    auto freezeResolver = std::make_unique<FreezeResolver>(freezeCommon);
    ASSERT_EQ(freezeResolver->Init(), true);
    auto time = freezeResolver->GetTimeZone();
    ASSERT_NE(time, "");
}

/**
 * @tc.name: FreezeVender_001
 * @tc.desc: FreezeDetector
 */
HWTEST_F(FreezeDetectorUnittest, FreezeVender_001, TestSize.Level3)
{
    auto vendor = std::make_unique<Vendor>(nullptr);
    ASSERT_EQ(vendor->Init(), false);
    std::list<WatchPoint> list;
    FreezeResult result;
    ASSERT_EQ(vendor->ReduceRelevanceEvents(list, result), false);
}

/**
 * @tc.name: FreezeVender_002
 * @tc.desc: FreezeDetector
 */
HWTEST_F(FreezeDetectorUnittest, FreezeVender_002, TestSize.Level3)
{
    auto freezeCommon = std::make_shared<FreezeCommon>();
    bool ret1 = freezeCommon->Init();
    ASSERT_EQ(ret1, true);
    auto vendor = std::make_unique<Vendor>(freezeCommon);
    ASSERT_EQ(vendor->Init(), true);
    std::list<WatchPoint> list;
    FreezeResult result;
    result.SetId(3);
    ASSERT_EQ(vendor->ReduceRelevanceEvents(list, result), false);
}

/**
 * @tc.name: FreezeVender_003
 * @tc.desc: FreezeDetector
 */
HWTEST_F(FreezeDetectorUnittest, FreezeVender_003, TestSize.Level3)
{
    auto freezeCommon = std::make_shared<FreezeCommon>();
    bool ret1 = freezeCommon->Init();
    ASSERT_EQ(ret1, true);
    auto vendor = std::make_unique<Vendor>(freezeCommon);
    ASSERT_EQ(vendor->Init(), true);
    std::list<WatchPoint> list;
    FreezeResult result;
    result.SetId(1);
    WatchPoint watchPoint1 = OHOS::HiviewDFX::WatchPoint::Builder()
        .InitDomain("KERNEL_VENDOR")
        .InitStringId("SCREEN_ON")
        .InitTimestamp(TimeUtil::GetMilliseconds())
        .Build();
    list.push_back(watchPoint1);
    WatchPoint watchPoint2 = OHOS::HiviewDFX::WatchPoint::Builder()
        .InitDomain("KERNEL_VENDOR3")
        .InitStringId("SCREEN_ON223")
        .InitTimestamp(TimeUtil::GetMilliseconds())
        .Build();
    list.push_back(watchPoint2);
    ASSERT_EQ(vendor->ReduceRelevanceEvents(list, result), true);
}

/**
 * @tc.name: FreezeVender_004
 * @tc.desc: FreezeDetector
 */
HWTEST_F(FreezeDetectorUnittest, FreezeVender_004, TestSize.Level3)
{
    auto freezeCommon = std::make_shared<FreezeCommon>();
    bool ret1 = freezeCommon->Init();
    ASSERT_EQ(ret1, true);
    auto vendor = std::make_unique<Vendor>(freezeCommon);
    ASSERT_EQ(vendor->Init(), true);
    std::list<WatchPoint> list;
    FreezeResult result;
    result.SetId(0);
    WatchPoint watchPoint1 = OHOS::HiviewDFX::WatchPoint::Builder()
        .InitDomain("KERNEL_VENDOR")
        .InitStringId("SCREEN_ON")
        .InitTimestamp(TimeUtil::GetMilliseconds())
        .Build();
    list.push_back(watchPoint1);
    WatchPoint watchPoint2 = OHOS::HiviewDFX::WatchPoint::Builder()
        .InitDomain("AAFWK")
        .InitStringId("LIFECYCLE_TIMEOUT")
        .InitTimestamp(TimeUtil::GetMilliseconds())
        .Build();
    list.push_back(watchPoint2);
    ASSERT_EQ(vendor->ReduceRelevanceEvents(list, result), true);
}
}
}
