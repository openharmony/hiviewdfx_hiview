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

#include "sys_event_service_proxy.h"

#include "errors.h"
#include "logger.h"
#include "parcelable_vector_rw.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-SysEventServiceProxy");
int SysEventServiceProxy::AddListener(const std::vector<SysEventRule>& rules, const sptr<ISysEventCallback>& callback)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HIVIEW_LOGE("SysEventService Remote is NULL.");
        return ERR_FLATTEN_OBJECT;
    }
    MessageParcel data;
    int32_t result = 0;
    if (!data.WriteInterfaceToken(SysEventServiceProxy::GetDescriptor())) {
        HIVIEW_LOGE("write descriptor failed.");
        return result;
    }

    bool ret = WriteVectorToParcel(data, rules);
    if (!ret) {
        HIVIEW_LOGE("parcel write rules failed.");
        return ERR_FLATTEN_OBJECT;
    }
    ret = data.WriteRemoteObject(callback->AsObject());
    if (!ret) {
        HIVIEW_LOGE("parcel write callback failed.");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(ADD_SYS_EVENT_LISTENER, data, reply, option);
    if (res != ERR_OK) {
        HIVIEW_LOGE("send request failed, error is %{public}d.", res);
        return false;
    }

    ret = reply.ReadInt32(result);
    if (!ret) {
        HIVIEW_LOGE("parcel read result failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return result;
}

void SysEventServiceProxy::RemoveListener(const sptr<ISysEventCallback> &callback)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HIVIEW_LOGE("SysEventService Remote is null.");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SysEventServiceProxy::GetDescriptor())) {
        HIVIEW_LOGE("write descriptor failed.");
        return;
    }

    bool ret = data.WriteRemoteObject(callback->AsObject());
    if (!ret) {
        HIVIEW_LOGE("parcel write object in callback failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(REMOVE_SYS_EVENT_LISTENER, data, reply, option);
    if (res != ERR_OK) {
        HIVIEW_LOGE("send request failed, error is %{public}d.", res);
    }
}

bool SysEventServiceProxy::QuerySysEvent(int64_t beginTime, int64_t endTime, int32_t maxEvents,
    const std::vector<SysEventQueryRule>& rules, const sptr<IQuerySysEventCallback>& callback)
{
    bool result = false;
    auto remote = Remote();
    if (remote == nullptr) {
        HIVIEW_LOGE("SysEventService Remote is null.");
        return result;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SysEventServiceProxy::GetDescriptor())) {
        HIVIEW_LOGE("write descriptor failed.");
        return result;
    }

    bool ret = data.WriteInt64(beginTime);
    if (!ret) {
        HIVIEW_LOGE("parcel write begin time failed.");
        return result;
    }
    ret = data.WriteInt64(endTime);
    if (!ret) {
        HIVIEW_LOGE("parcel write end time failed.");
        return result;
    }
    ret = data.WriteInt32(maxEvents);
    if (!ret) {
        HIVIEW_LOGE("parcel write max events failed.");
        return result;
    }
    ret = WriteVectorToParcel(data, rules);
    if (!ret) {
        HIVIEW_LOGE("parcel write query rules failed.");
        return result;
    }
    ret = data.WriteRemoteObject(callback->AsObject());
    if (!ret) {
        HIVIEW_LOGE("parcel write callback failed.");
        return result;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(QUERY_SYS_EVENT, data, reply, option);
    if (res != ERR_OK) {
        HIVIEW_LOGE("send request failed, error is %{public}d.", res);
        return result;
    }
    ret = reply.ReadBool(result);
    if (!ret) {
        HIVIEW_LOGE("parcel read result failed.");
        return result;
    }
    return result;
}

bool SysEventServiceProxy::SetDebugMode(const sptr<ISysEventCallback>& callback, bool mode)
{
    bool result = false;
    auto remote = Remote();
    if (remote == nullptr) {
        HIVIEW_LOGE("SysEventService Remote is null.");
        return result;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SysEventServiceProxy::GetDescriptor())) {
        HIVIEW_LOGE("write descriptor failed.");
        return result;
    }

    bool ret = data.WriteRemoteObject(callback->AsObject());
    if (!ret) {
        HIVIEW_LOGE("parcel write callback failed.");
        return result;
    }
    ret = data.WriteBool(mode);
    if (!ret) {
        HIVIEW_LOGE("parcel write mode failed.");
        return result;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t res = remote->SendRequest(SET_DEBUG_MODE, data, reply, option);
    if (res != ERR_OK) {
        HIVIEW_LOGE("send request failed, error is %{public}d.", res);
        return result;
    }
    ret = reply.ReadBool(result);
    if (!ret) {
        HIVIEW_LOGE("parcel read result failed.");
        return result;
    }
    return result;
}
} // namespace HiviewDFX
} // namespace OHOS
