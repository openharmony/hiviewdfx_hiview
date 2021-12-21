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

#include "query_sys_event_callback_proxy.h"

#include "errors.h"
#include "logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-QuerySysEventCallbackProxy");
void QuerySysEventCallbackProxy::OnQuery(const std::vector<std::u16string>& sysEvent, const std::vector<int64_t>& seq)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HIVIEW_LOGE("SysEventService Remote is NULL.");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(QuerySysEventCallbackProxy::GetDescriptor())) {
        HIVIEW_LOGE("write descriptor failed.");
        return;
    }

    bool ret = data.WriteString16Vector(sysEvent);
    if (!ret) {
        HIVIEW_LOGE("write sys event failed.");
        return;
    }
    ret = data.WriteInt64Vector(seq);
    if (!ret) {
        HIVIEW_LOGE("write sys seq failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(ON_QUERY, data, reply, option);
    if (res != ERR_OK) {
        HIVIEW_LOGE("send request failed, error is %{public}d.", res);
    }
}

void QuerySysEventCallbackProxy::OnComplete(int32_t reason, int32_t total)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HIVIEW_LOGE("SysEventService Remote is NULL.");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(QuerySysEventCallbackProxy::GetDescriptor())) {
        HIVIEW_LOGE("write descriptor failed.");
        return;
    }

    bool ret = data.WriteInt32(reason);
    if (!ret) {
        HIVIEW_LOGE("write reason failed.");
        return;
    }
    ret = data.WriteInt32(total);
    if (!ret) {
        HIVIEW_LOGE("write total failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(ON_COMPLETE, data, reply, option);
    if (res != ERR_OK) {
        HIVIEW_LOGE("send request failed, error is %{public}d.", res);
    }
}
} // namespace HiviewDFX
} // namespace OHOS