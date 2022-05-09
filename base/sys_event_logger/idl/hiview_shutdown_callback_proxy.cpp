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
#include "hiview_shutdown_callback_proxy.h"

#include <ipc_types.h>
#include <message_parcel.h>

#include "hilog/log.h"
#include "sys_event_common.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, LABEL_DOMAIN, "HiViewShutdownCallbackProxy" };
}

void HiViewShutdownCallbackProxy::ShutdownCallback()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HiLog::Error(LABEL, "failed to create remote object");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(HiViewShutdownCallbackProxy::GetDescriptor())) {
        HiLog::Error(LABEL, "failed to write descriptor");
        return;
    }

    int ret = remote->SendRequest(static_cast<int>(PowerMgr::IShutdownCallback::POWER_SHUTDOWN_CHANGED),
        data, reply, option);
    if (ret != ERR_OK) {
        HiLog::Error(LABEL, "failed to SendRequest, ret: %{public}d", ret);
    }
}
} // namespace HiviewDFX
} // namespace OHOS
