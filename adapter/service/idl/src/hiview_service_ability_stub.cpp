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

#include "accesstoken_kit.h"
#include "errors.h"
#include "hiview_napi_err_code.h"
#include "ipc_skeleton.h"
#include "logger.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
DEFINE_LOG_TAG("HiViewSA-HiViewServiceAbilityStub");
const std::unordered_map<uint32_t, std::string> PERMISSION_MAP = {
    {IHiviewServiceAbility::HIVIEW_SERVICE_ID_LIST, "ohos.permission.READ_HIVIEW_SYSTEM"},
    {IHiviewServiceAbility::HIVIEW_SERVICE_ID_COPY, "ohos.permission.READ_HIVIEW_SYSTEM"},
    {IHiviewServiceAbility::HIVIEW_SERVICE_ID_MOVE, "ohos.permission.WRITE_HIVIEW_SYSTEM"},
    {IHiviewServiceAbility::HIVIEW_SERVICE_ID_REMOVE, "ohos.permission.WRITE_HIVIEW_SYSTEM"}
};
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
    switch (code) {
        case HIVIEW_SERVICE_ID_LIST:
            return HandleListRequest(data, reply, option);
        case HIVIEW_SERVICE_ID_COPY:
            return HandleCopyRequest(data, reply, option);
        case HIVIEW_SERVICE_ID_MOVE:
            return HandleMoveRequest(data, reply, option);
        case HIVIEW_SERVICE_ID_REMOVE:
            return HandleRemoveRequest(data, reply, option);
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

bool HiviewServiceAbilityStub::IsPermissionGranted(uint32_t code)
{
    auto iter = PERMISSION_MAP.find(code);
    if (iter == PERMISSION_MAP.end()) {
        return true;
    }
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    int verifyResult = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, iter->second);
    if (verifyResult == Security::AccessToken::PERMISSION_GRANTED) {
        return true;
    }
    HIVIEW_LOGW("%{public}s not granted, code: %{public}u", iter->second.c_str(), code);
    return false;
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
    auto fileNum = fileInfos.size();
    if (!reply.WriteInt32(fileNum)) {
        HIVIEW_LOGE("write result failed, ret: %{public}d", ret);
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    for (auto& fileInfo : fileInfos) {
        if (!reply.WriteParcelable(&fileInfo)) {
            HIVIEW_LOGE("write file info failed.");
            return HiviewNapiErrCode::ERR_DEFAULT;
        }
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
} // namespace HiviewDFX
} // namespace OHOS