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
#include <iostream>

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "network_collector.h"
#include "token_setproc.h"
#include "network_collector.h"
#include "wifi_device.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace OHOS::HiviewDFX::UCollect;

namespace {
void NativeTokenGet(const char* perms[], int size)
{
    uint64_t tokenId;
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = size,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .aplStr = "system_basic",
    };

    infoInstance.processName = "UCollectionUtilityUnitTest";
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void EnablePermissionAccess()
{
    const char* perms[] = {
        "ohos.permission.GET_WIFI_INFO",
    };
    NativeTokenGet(perms, 1); // 1 is the size of the array which consists of required permissions.
}

void DisablePermissionAccess()
{
    NativeTokenGet(nullptr, 0); // empty permission array.
}

bool IsWifiEnabled()
{
    std::shared_ptr<OHOS::Wifi::WifiDevice> wifiDevicePtr =
        OHOS::Wifi::WifiDevice::GetInstance(OHOS::WIFI_DEVICE_SYS_ABILITY_ID);
    if (wifiDevicePtr == nullptr) {
        return false;
    }
    bool isActive = false;
    wifiDevicePtr->IsWifiActive(isActive);
    if (!isActive) {
        return false;
    }
    OHOS::Wifi::WifiLinkedInfo linkInfo;
    int ret = wifiDevicePtr->GetLinkedInfo(linkInfo);
    return ret == OHOS::Wifi::WIFI_OPT_SUCCESS;
}
}

class NetworkCollectorTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
};

/**
 * @tc.name: NetworkCollectorTest001
 * @tc.desc: used to test NetworkCollector.CollectRate
 * @tc.type: FUNC
*/
HWTEST_F(NetworkCollectorTest, NetworkCollectorTest001, TestSize.Level1)
{
    EnablePermissionAccess();
    std::shared_ptr<NetworkCollector> collector = NetworkCollector::Create();
    CollectResult<NetworkRate> data = collector->CollectRate();
    std::cout << "collect network rate result" << data.retCode << std::endl;
    if (IsWifiEnabled()) {
        ASSERT_TRUE(data.retCode == UcError::SUCCESS);
    }
    DisablePermissionAccess();
}

/**
 * @tc.name: NetworkCollectorTest002
 * @tc.desc: used to test NetworkCollector.CollectSysPackets
 * @tc.type: FUNC
*/
HWTEST_F(NetworkCollectorTest, NetworkCollectorTest002, TestSize.Level1)
{
    EnablePermissionAccess();
    std::shared_ptr<NetworkCollector> collector = NetworkCollector::Create();
    CollectResult<NetworkPackets> data = collector->CollectSysPackets();
    std::cout << "collect network packets result" << data.retCode << std::endl;
    if (IsWifiEnabled()) {
        ASSERT_TRUE(data.retCode == UcError::SUCCESS);
    }
    DisablePermissionAccess();
}