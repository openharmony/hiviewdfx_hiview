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

#include "network_collector_impl.h"

#include "logger.h"
#include "network_decorator.h"
#include "wifi_device.h"

using namespace OHOS::HiviewDFX::UCollect;

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
DEFINE_LOG_TAG("UCollectUtil");

std::shared_ptr<NetworkCollector> NetworkCollector::Create()
{
    return std::make_shared<NetworkDecorator>(std::make_shared<NetworkCollectorImpl>());
}

inline bool GetNetworkInfo(Wifi::WifiLinkedInfo& linkInfo)
{
    std::shared_ptr<Wifi::WifiDevice> wifiDevicePtr = Wifi::WifiDevice::GetInstance(OHOS::WIFI_DEVICE_SYS_ABILITY_ID);
    if (wifiDevicePtr == nullptr) {
        return false;
    }
    bool isActive = false;
    wifiDevicePtr->IsWifiActive(isActive);
    if (!isActive) {
        return false;
    }
    int ret = wifiDevicePtr->GetLinkedInfo(linkInfo);
    if (ret != Wifi::WIFI_OPT_SUCCESS) {
        HIVIEW_LOGE("GetLinkedInfo failed");
        return false;
    } else {
        return true;
    }
}

CollectResult<NetworkRate> NetworkCollectorImpl::CollectRate()
{
    CollectResult<NetworkRate> result;
    Wifi::WifiLinkedInfo linkInfo;
    if (GetNetworkInfo(linkInfo)) {
        NetworkRate& networkRate = result.data;
        networkRate.rssi = linkInfo.rssi;
        HIVIEW_LOGD("rssi = %d", networkRate.rssi);
        networkRate.txBitRate = linkInfo.txLinkSpeed;
        HIVIEW_LOGD("txBitRate = %d", networkRate.txBitRate);
        networkRate.rxBitRate = linkInfo.rxLinkSpeed;
        HIVIEW_LOGD("rxBitRate = %d", networkRate.rxBitRate);
        result.retCode = UcError::SUCCESS;
    } else {
        HIVIEW_LOGE("IsWifiActive failed");
        result.retCode = UcError::UNSUPPORT;
    }
    return result;
}

CollectResult<NetworkPackets> NetworkCollectorImpl::CollectSysPackets()
{
    CollectResult<NetworkPackets> result;
    Wifi::WifiLinkedInfo linkInfo;
    if (GetNetworkInfo(linkInfo)) {
        NetworkPackets& networkPackets = result.data;
        networkPackets.currentSpeed = linkInfo.linkSpeed;
        HIVIEW_LOGD("currentSpeed = %d", networkPackets.currentSpeed);
        networkPackets.currentTxBytes = linkInfo.lastTxPackets;
        HIVIEW_LOGD("currentTxBytes = %d", networkPackets.currentTxBytes);
        networkPackets.currentRxBytes = linkInfo.lastRxPackets;
        HIVIEW_LOGD("currentRxBytes = %d", networkPackets.currentRxBytes);
        result.retCode = UcError::SUCCESS;
    } else {
        HIVIEW_LOGE("IsWifiActive failed");
        result.retCode = UcError::UNSUPPORT;
    }
    return result;
}
} // UCollectUtil
} // HiViewDFX
} // OHOS
