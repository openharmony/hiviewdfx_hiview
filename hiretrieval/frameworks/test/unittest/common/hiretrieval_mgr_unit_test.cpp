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

#include "hiretrieval_mgr_unit_test.h"

#include <gmock/gmock.h>

#include "hiretrieval_mgr.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
bool IsUnitializedCfg(const HiRetrievalMgr::Config& cfg)
{
    return std::string(cfg.userType).empty() &&
        std::string(cfg.deviceType).empty() &&
        std::string(cfg.deviceModel).empty();
}

bool IsSameCfg(const HiRetrievalMgr::Config& src, const HiRetrievalMgr::Config& dest)
{
    return std::string(src.userType) == std::string(dest.userType) &&
        std::string(src.deviceType) == std::string(dest.deviceType) &&
        std::string(src.deviceModel) == std::string(dest.deviceModel);
}
}
void HiRetrievalMgrUnitTest::SetUpTestCase()
{
}

void HiRetrievalMgrUnitTest::TearDownTestCase()
{
}

void HiRetrievalMgrUnitTest::SetUp()
{
}

void HiRetrievalMgrUnitTest::TearDown()
{
}

/**
 * @tc.name: HiRetrievalMgrUnitTest001
 * @tc.desc: test the Participate/Quit/Run of class HiRetrievalMgr
 * @tc.type: FUNC
 * @tc.require: issue3276
 */
HWTEST_F(HiRetrievalMgrUnitTest, HiRetrievalMgrUnitTest001, testing::ext::TestSize.Level3)
{
    HiRetrievalMgr::Config cfg {
        "user_type",
        "device_type",
        "device_model",
    };
    auto& instance = HiRetrievalMgr::GetInstance();
    ASSERT_EQ(instance.Participate(cfg), HiRetrieval::NativeErrorCode::NOT_INIT);
    ASSERT_EQ(instance.Quit(), HiRetrieval::NativeErrorCode::NOT_INIT);
    ASSERT_EQ(instance.Run(), HiRetrieval::NativeErrorCode::NOT_INIT);
    ASSERT_EQ(instance.Init(), HiRetrieval::NativeErrorCode::SUCC);
    ASSERT_EQ(instance.Participate(cfg), HiRetrieval::NativeErrorCode::SUCC);
    ASSERT_EQ(instance.Run(), HiRetrieval::NativeErrorCode::SUCC);
    ASSERT_EQ(instance.Quit(), HiRetrieval::NativeErrorCode::SUCC);
}

/**
 * @tc.name: HiRetrievalMgrUnitTest002
 * @tc.desc: test the IsParticipant of class HiRetrievalMgr
 * @tc.type: FUNC
 * @tc.require: issue3276
 */
HWTEST_F(HiRetrievalMgrUnitTest, HiRetrievalMgrUnitTest002, testing::ext::TestSize.Level3)
{
    HiRetrievalMgr::Config cfg {
        "user_type",
        "device_type",
        "device_model",
    };
    auto& instance = HiRetrievalMgr::GetInstance();
    ASSERT_EQ(instance.Participate(cfg), HiRetrieval::NativeErrorCode::NOT_INIT);
    ASSERT_FALSE(instance.IsParticipant());
    ASSERT_EQ(instance.Init(), HiRetrieval::NativeErrorCode::SUCC);
    ASSERT_FALSE(instance.IsParticipant());
    ASSERT_EQ(instance.Participate(cfg), HiRetrieval::NativeErrorCode::SUCC);
    ASSERT_TRUE(instance.IsParticipant());
    ASSERT_EQ(instance.Quit(), HiRetrieval::NativeErrorCode::SUCC);
    ASSERT_FALSE(instance.IsParticipant());
}

/**
 * @tc.name: HiRetrievalMgrUnitTest003
 * @tc.desc: test the GetLastParticipationTs of class HiRetrievalMgr
 * @tc.type: FUNC
 * @tc.require: issue3276
 */
HWTEST_F(HiRetrievalMgrUnitTest, HiRetrievalMgrUnitTest003, testing::ext::TestSize.Level3)
{
    HiRetrievalMgr::Config cfg {
        "user_type",
        "device_type",
        "device_model",
    };
    auto& instance = HiRetrievalMgr::GetInstance();
    ASSERT_EQ(instance.GetLastParticipationTs(), 0);
    ASSERT_EQ(instance.Init(), HiRetrieval::NativeErrorCode::SUCC);
    ASSERT_EQ(instance.Participate(cfg), HiRetrieval::NativeErrorCode::SUCC);
    ASSERT_GT(instance.GetLastParticipationTs(), 0);
    ASSERT_EQ(instance.Quit(), HiRetrieval::NativeErrorCode::SUCC);
    ASSERT_GT(instance.GetLastParticipationTs(), 0);
}

/**
 * @tc.name: HiRetrievalMgrUnitTest004
 * @tc.desc: test the GetCurrentConfig of class HiRetrievalMgr
 * @tc.type: FUNC
 * @tc.require: issue3276
 */
HWTEST_F(HiRetrievalMgrUnitTest, HiRetrievalMgrUnitTest004, testing::ext::TestSize.Level3)
{
    HiRetrievalMgr::Config cfg {
        "user_type",
        "device_type",
        "device_model",
    };
    auto& instance = HiRetrievalMgr::GetInstance();
    auto cfgGot = instance.GetCurrentConfig();
    ASSERT_TRUE(IsUnitializedCfg(cfgGot));
    ASSERT_EQ(instance.Init(), HiRetrieval::NativeErrorCode::SUCC);
    ASSERT_EQ(instance.Participate(cfg), HiRetrieval::NativeErrorCode::SUCC);
    cfgGot = instance.GetCurrentConfig();
    ASSERT_TRUE(IsSameCfg(cfg, cfgGot));
    ASSERT_EQ(instance.Quit(), HiRetrieval::NativeErrorCode::SUCC);
    ASSERT_TRUE(IsUnitializedCfg(cfgGot));
}
} // namespace HiviewDFX
} // namespace OHOS