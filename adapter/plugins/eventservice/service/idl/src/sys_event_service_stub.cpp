/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "sys_event_service_stub.h"

#include "errors.h"
#include "hiview_logger.h"
#include "memory_util.h"
#include "parcelable_vector_rw.h"
#include "query_argument.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-SysEventServiceStub");
int32_t SysEventServiceStub::HandleAddSysEventListener(MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    std::vector<SysEventRule> rules;
    bool ret = ReadVectorFromParcel(data, rules);
    if (!ret) {
        HIVIEW_LOGE("parcel read rules failed.");
        return ERR_FLATTEN_OBJECT;
    }
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        return ERR_FLATTEN_OBJECT;
    }
    sptr<ISysEventCallback> callback = iface_cast<ISysEventCallback>(remoteObject);
    ret = reply.WriteInt32(AddListener(rules, callback));
    if (!ret) {
        HIVIEW_LOGE("parcel write return-value of AddListener failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SysEventServiceStub::HandleRemoveSysEventListener(MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        return ERR_FLATTEN_OBJECT;
    }
    sptr<ISysEventCallback> callback = iface_cast<ISysEventCallback>(remoteObject);
    if (callback == nullptr) {
        HIVIEW_LOGE("parcel read callback failed.");
        return ERR_FLATTEN_OBJECT;
    }
    bool ret = reply.WriteInt32(RemoveListener(callback));
    if (!ret) {
        HIVIEW_LOGE("parcel write return-value of RemoveListener failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SysEventServiceStub::HandleQueryEvent(MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    std::unique_ptr<QueryArgument> queryArgument(data.ReadParcelable<QueryArgument>());
    if (queryArgument == nullptr) {
        HIVIEW_LOGE("parcel read query arguments failed.");
        return ERR_FLATTEN_OBJECT;
    }
    std::vector<SysEventQueryRule> queryRules;
    auto ret = ReadVectorFromParcel(data, queryRules);
    if (!ret) {
        HIVIEW_LOGE("parcel read query rules failed.");
        return ERR_FLATTEN_OBJECT;
    }
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        HIVIEW_LOGE("parcel read query callback failed.");
        return ERR_FLATTEN_OBJECT;
    }
    sptr<IQuerySysEventCallback> callback = iface_cast<IQuerySysEventCallback>(remoteObject);
    ret = reply.WriteInt32(Query(*queryArgument, queryRules, callback));
    if (!ret) {
        HIVIEW_LOGE("parcel write return-value of QuerySysEvent failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SysEventServiceStub::HandleSetDebugMode(MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        return ERR_FLATTEN_OBJECT;
    }
    sptr<ISysEventCallback> callback = iface_cast<ISysEventCallback>(remoteObject);
    bool mode = false;
    bool ret = data.ReadBool(mode);
    if (!ret) {
        HIVIEW_LOGE("parcel read mode failed.");
        return ERR_FLATTEN_OBJECT;
    }
    ret = reply.WriteInt32(SetDebugMode(callback, mode));
    if (!ret) {
        HIVIEW_LOGE("parcel write return-value of SetDebugMode failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SysEventServiceStub::HandleAddSubscriber(MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    std::vector<SysEventQueryRule> queryRules;
    auto ret = ReadVectorFromParcel(data, queryRules);
    if (!ret) {
        HIVIEW_LOGE("parcel read export rules failed.");
        return ERR_FLATTEN_OBJECT;
    }
    ret = reply.WriteInt64(AddSubscriber(queryRules));
    if (!ret) {
        HIVIEW_LOGE("write return-value of AddSubscriber failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SysEventServiceStub::HandleRemoveSubscriber(
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    auto ret = reply.WriteInt32(RemoveSubscriber());
    if (!ret) {
        HIVIEW_LOGE("write return-value of RemoveSubscriber failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SysEventServiceStub::HandleExportEvent(MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    std::unique_ptr<QueryArgument> queryArgument(data.ReadParcelable<QueryArgument>());
    if (queryArgument == nullptr) {
        HIVIEW_LOGE("parcel read export arguments failed.");
        return ERR_FLATTEN_OBJECT;
    }
    std::vector<SysEventQueryRule> queryRules;
    auto ret = ReadVectorFromParcel(data, queryRules);
    if (!ret) {
        HIVIEW_LOGE("parcel read export rules failed.");
        return ERR_FLATTEN_OBJECT;
    }
    ret = reply.WriteInt64(Export(*queryArgument, queryRules));
    if (!ret) {
        HIVIEW_LOGE("parcel write return-value of ExportSysEvent failed.");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

int32_t SysEventServiceStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    if (MemoryUtil::DisableThreadCache() != 0 || MemoryUtil::DisableDelayFree() != 0) {
        HIVIEW_LOGW("Failed to optimize memory for current thread");
    }
    std::u16string descripter = SysEventServiceStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        HIVIEW_LOGE("read descriptor failed.");
        return ERR_INVALID_VALUE;
    }
    switch (code) {
        case static_cast<uint32_t>(SysEventServiceInterfaceCode::ADD_SYS_EVENT_LISTENER): {
            return HandleAddSysEventListener(data, reply, option);
        }
        case static_cast<uint32_t>(SysEventServiceInterfaceCode::REMOVE_SYS_EVENT_LISTENER): {
            return HandleRemoveSysEventListener(data, reply, option);
        }
        case static_cast<uint32_t>(SysEventServiceInterfaceCode::QUERY_SYS_EVENT): {
            return HandleQueryEvent(data, reply, option);
        }
        case static_cast<uint32_t>(SysEventServiceInterfaceCode::SET_DEBUG_MODE): {
            return HandleSetDebugMode(data, reply, option);
        }
        case static_cast<uint32_t>(SysEventServiceInterfaceCode::ADD_SYS_EVENT_SUBSCRIBER): {
            return HandleAddSubscriber(data, reply, option);
        }
        case static_cast<uint32_t>(SysEventServiceInterfaceCode::REMOVE_SYS_EVENT_SUBSCRIBER): {
            return HandleRemoveSubscriber(data, reply, option);
        }
        case static_cast<uint32_t>(SysEventServiceInterfaceCode::EXPORT_SYS_EVENT): {
            return HandleExportEvent(data, reply, option);
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}
} // namespace HiviewDFX
} // namespace OHOS
