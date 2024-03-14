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

#include "hiview_service_ability_stub.h"

#include <unordered_map>
#include <vector>

#include "accesstoken_kit.h"
#include "ash_memory_utils.h"
#include "client/trace_collector.h"
#include "errors.h"
#include "hiview_err_code.h"
#include "ipc_skeleton.h"
#include "logger.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
DEFINE_LOG_TAG("HiViewSA-HiViewServiceAbilityStub");
const std::string ASH_MEM_NAME = "HiviewLogLibrary SharedMemory";
constexpr uint32_t ASH_MEM_SIZE = 107 * 5000; // 535k

const std::unordered_map<uint32_t, std::string> ALL_PERMISSION_MAP = {
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_LIST),
        "ohos.permission.READ_HIVIEW_SYSTEM"},
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_COPY),
        "ohos.permission.READ_HIVIEW_SYSTEM"},
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_MOVE),
        "ohos.permission.WRITE_HIVIEW_SYSTEM"},
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_REMOVE),
        "ohos.permission.WRITE_HIVIEW_SYSTEM"},
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_OPEN_SNAPSHOT_TRACE),
        "ohos.permission.WRITE_HIVIEW_SYSTEM"},
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_DUMP_SNAPSHOT_TRACE),
        "ohos.permission.READ_HIVIEW_SYSTEM"},
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_OPEN_RECORDING_TRACE),
        "ohos.permission.WRITE_HIVIEW_SYSTEM"},
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_RECORDING_TRACE_ON),
        "ohos.permission.READ_HIVIEW_SYSTEM"},
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_RECORDING_TRACE_OFF),
        "ohos.permission.READ_HIVIEW_SYSTEM"},
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_CLOSE_TRACE),
        "ohos.permission.WRITE_HIVIEW_SYSTEM"},
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_RECOVER_TRACE),
        "ohos.permission.WRITE_HIVIEW_SYSTEM"}
};

const std::unordered_map<uint32_t, std::string> TRACE_PERMISSION_MAP = {
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_OPEN_SNAPSHOT_TRACE),
        "ohos.permission.DUMP"},
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_DUMP_SNAPSHOT_TRACE),
        "ohos.permission.DUMP"},
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_OPEN_RECORDING_TRACE),
        "ohos.permission.DUMP"},
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_RECORDING_TRACE_ON),
        "ohos.permission.DUMP"},
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_RECORDING_TRACE_OFF),
        "ohos.permission.DUMP"},
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_CLOSE_TRACE),
        "ohos.permission.DUMP"},
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_RECOVER_TRACE),
        "ohos.permission.DUMP"}
};

const std::unordered_map<uint32_t, std::string> CPU_PERMISSION_MAP = {
    {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_GET_SYSTEM_CPU_USAGE), ""}
};

bool HasAccessPermission(uint32_t code, const std::unordered_map<uint32_t, std::string>& permissions)
{
    using namespace Security::AccessToken;
    auto iter = permissions.find(code);
    if (iter == permissions.end()) {
        return false;
    }
    if (iter->second.empty()) {
        return true;
    }
    AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    int verifyResult = AccessTokenKit::VerifyAccessToken(callerToken, iter->second);
    if (verifyResult == PERMISSION_GRANTED) {
        return true;
    }
    HIVIEW_LOGW("%{public}s not granted, code: %{public}u", iter->second.c_str(), code);
    return false;
}

int32_t WritePracelableToMessage(MessageParcel& dest, Parcelable& data)
{
    if (!dest.WriteParcelable(&data)) {
        HIVIEW_LOGW("failed to write TraceErrorCodeWrapper to parcel");
        return TraceErrCode::ERR_WRITE_MSG_PARCEL;
    }
    return TraceErrCode::ERR_OK;
}
}

int32_t HiviewServiceAbilityStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    HIVIEW_LOGI("cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
    std::u16string descripter = HiviewServiceAbilityStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        return -ERR_INVALID_VALUE;
    }
    if (!IsPermissionGranted(code)) {
        return HiviewNapiErrCode::ERR_PERMISSION_CHECK;
    }
    auto requestHandler = GetRequestHandler(code);
    if (requestHandler == nullptr) {
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return requestHandler(data, reply, option);
}

bool HiviewServiceAbilityStub::IsPermissionGranted(uint32_t code)
{
    return HasAccessPermission(code, ALL_PERMISSION_MAP) || HasAccessPermission(code, TRACE_PERMISSION_MAP) ||
        HasAccessPermission(code, CPU_PERMISSION_MAP);
}

std::unordered_map<uint32_t, RequestHandler> HiviewServiceAbilityStub::GetRequestHandlers()
{
    static std::unordered_map<uint32_t, RequestHandler> requestHandlers = {
        {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_LIST),
            std::bind(&HiviewServiceAbilityStub::HandleListRequest, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_COPY),
            std::bind(&HiviewServiceAbilityStub::HandleCopyRequest, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_MOVE),
            std::bind(&HiviewServiceAbilityStub::HandleMoveRequest, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_REMOVE),
            std::bind(&HiviewServiceAbilityStub::HandleRemoveRequest, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)}
    };
    return requestHandlers;
}

std::unordered_map<uint32_t, RequestHandler> HiviewServiceAbilityStub::GetTraceRequestHandlers()
{
    static std::unordered_map<uint32_t, RequestHandler> requestHandlers = {
        {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_OPEN_SNAPSHOT_TRACE),
            std::bind(&HiviewServiceAbilityStub::HandleOpenSnapshotTraceRequest, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_DUMP_SNAPSHOT_TRACE),
            std::bind(&HiviewServiceAbilityStub::HandleDumpSnapshotTraceRequest, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_OPEN_RECORDING_TRACE),
            std::bind(&HiviewServiceAbilityStub::HandleOpenRecordingTraceRequest, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_RECORDING_TRACE_ON),
            std::bind(&HiviewServiceAbilityStub::HandleRecordingTraceOnRequest, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_RECORDING_TRACE_OFF),
            std::bind(&HiviewServiceAbilityStub::HandleRecordingTraceOffRequest, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_CLOSE_TRACE),
            std::bind(&HiviewServiceAbilityStub::HandleCloseTraceRequest, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
        {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_RECOVER_TRACE),
            std::bind(&HiviewServiceAbilityStub::HandleRecoverTraceRequest, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)}
    };
    return requestHandlers;
}

std::unordered_map<uint32_t, RequestHandler> HiviewServiceAbilityStub::GetCpuRequestHandlers()
{
    static std::unordered_map<uint32_t, RequestHandler> cpuRequestHandlers = {
        {static_cast<uint32_t>(HiviewServiceInterfaceCode::HIVIEW_SERVICE_ID_GET_SYSTEM_CPU_USAGE),
         std::bind(&HiviewServiceAbilityStub::HandleGetSysCpuUsageRequest, this,
                   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)}
    };
    return cpuRequestHandlers;
}

RequestHandler HiviewServiceAbilityStub::GetRequestHandler(uint32_t code)
{
    std::vector<std::unordered_map<uint32_t, RequestHandler>> allHandlerMaps = {
        GetRequestHandlers(),
        GetTraceRequestHandlers(),
        GetCpuRequestHandlers()
    };
    for (const auto &handlerMap : allHandlerMaps) {
        auto iter = handlerMap.find(code);
        if (iter == handlerMap.end()) {
            continue;
        }
        return iter->second;
    }
    HIVIEW_LOGE("function for handling request isn't found");
    return nullptr;
}

int32_t HiviewServiceAbilityStub::HandleListRequest(MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    std::string logType;
    if (!data.ReadString(logType)) {
        HIVIEW_LOGE("cannot get log type");
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    std::vector<HiviewFileInfo> fileInfos;
    int32_t ret = List(logType, fileInfos);
    if (ret != ERR_OK) {
        return ret;
    }
    HIVIEW_LOGW("file list num:%{public}d", fileInfos.size());
    sptr<Ashmem> ashmem = AshMemoryUtils::GetAshmem(ASH_MEM_NAME, ASH_MEM_SIZE);
    if (ashmem == nullptr) {
        HIVIEW_LOGE("ge ashmem failed.");
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    std::vector<uint32_t> allSize;
    if (!AshMemoryUtils::WriteBulkData<HiviewFileInfo>(fileInfos, ashmem, ASH_MEM_SIZE, allSize)) {
        HIVIEW_LOGE("WriteBulkData failed.");
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    if (!reply.WriteUInt32Vector(allSize)) {
        HIVIEW_LOGE("write size failed.");
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    if (!reply.WriteAshmem(ashmem)) {
        HIVIEW_LOGE("write ashmem failed.");
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    return ERR_OK;
}

int32_t HiviewServiceAbilityStub::HandleCopyRequest(MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    return HandleCopyOrMoveRequest(data, reply, option, false);
}

int32_t HiviewServiceAbilityStub::HandleMoveRequest(MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    return HandleCopyOrMoveRequest(data, reply, option, true);
}

int32_t HiviewServiceAbilityStub::HandleCopyOrMoveRequest(
    MessageParcel& data, MessageParcel& reply, MessageOption& option, bool isMove)
{
    std::string logType;
    if (!data.ReadString(logType)) {
        HIVIEW_LOGW("cannot get logtype");
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    std::string logName;
    if (!data.ReadString(logName)) {
        HIVIEW_LOGW("cannot get log type");
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    std::string dest;
    if (!data.ReadString(dest)) {
        HIVIEW_LOGW("cannot get dest dir");
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    if (dest.find("..") != std::string::npos) {
        HIVIEW_LOGW("invalid dest: %{public}s", dest.c_str());
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    int32_t ret = isMove ? Move(logType, logName, dest) : Copy(logType, logName, dest);
    if (!reply.WriteInt32(ret)) {
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    return ERR_OK;
}

int32_t HiviewServiceAbilityStub::HandleRemoveRequest(MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    std::string logType;
    if (!data.ReadString(logType)) {
        HIVIEW_LOGW("cannot get log type");
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    std::string logName;
    if (!data.ReadString(logName)) {
        HIVIEW_LOGW("cannot get log name");
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    int32_t ret = Remove(logType, logName);
    if (!reply.WriteInt32(ret)) {
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    return ERR_OK;
}

int32_t HiviewServiceAbilityStub::HandleOpenSnapshotTraceRequest(MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    std::vector<std::string> tagGroups;
    if (!data.ReadStringVector(&tagGroups)) {
        HIVIEW_LOGW("failed to read tag groups from parcel");
        return TraceErrCode::ERR_READ_MSG_PARCEL;
    }
    auto ret = OpenSnapshotTrace(tagGroups);
    return WritePracelableToMessage(reply, ret);
}

int32_t HiviewServiceAbilityStub::HandleDumpSnapshotTraceRequest(MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    int32_t caller = UCollectClient::TraceCollector::Caller::OTHER;
    if (!data.ReadInt32(caller)) {
        HIVIEW_LOGW("failed to read trace caller from parcel");
        return TraceErrCode::ERR_READ_MSG_PARCEL;
    }
    auto ret = DumpSnapshotTrace(caller);
    return WritePracelableToMessage(reply, ret);
}

int32_t HiviewServiceAbilityStub::HandleOpenRecordingTraceRequest(MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    std::string tags;
    if (!data.ReadString(tags)) {
        HIVIEW_LOGW("failed to read tags from parcel");
        return TraceErrCode::ERR_READ_MSG_PARCEL;
    }
    auto ret = OpenRecordingTrace(tags);
    return WritePracelableToMessage(reply, ret);
}

int32_t HiviewServiceAbilityStub::HandleRecordingTraceOnRequest(MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    auto ret = RecordingTraceOn();
    return WritePracelableToMessage(reply, ret);
}

int32_t HiviewServiceAbilityStub::HandleRecordingTraceOffRequest(MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    auto ret = RecordingTraceOff();
    return WritePracelableToMessage(reply, ret);
}

int32_t HiviewServiceAbilityStub::HandleCloseTraceRequest(MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    auto ret = CloseTrace();
    return WritePracelableToMessage(reply, ret);
}

int32_t HiviewServiceAbilityStub::HandleRecoverTraceRequest(MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    auto ret = RecoverTrace();
    return WritePracelableToMessage(reply, ret);
}

int32_t HiviewServiceAbilityStub::HandleGetSysCpuUsageRequest(MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    auto ret = GetSysCpuUsage();
    return WritePracelableToMessage(reply, ret);
}
} // namespace HiviewDFX
} // namespace OHOS