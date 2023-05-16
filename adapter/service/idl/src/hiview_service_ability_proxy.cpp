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

#include "hiview_service_ability_proxy.h"

#include "hiview_napi_err_code.h"
#include "logger.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
DEFINE_LOG_TAG("HiviewServiceAbilityProxy");
constexpr int32_t MAX_FILE_NUM = 10000;
}

int32_t HiviewServiceAbilityProxy::List(const std::string& logType, std::vector<HiviewFileInfo>& fileInfos)
{
    HIVIEW_LOGI("start list.");
    MessageParcel data;
    int32_t constructRequestRet = ConstructRequestParcel(data, logType);
    if (constructRequestRet != ERR_OK) {
        return constructRequestRet;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t res = Remote()->SendRequest(HIVIEW_SERVICE_ID_LIST, data, reply, option);
    if (res != ERR_OK) {
        HIVIEW_LOGE("send request failed, error is %{public}d.", res);
        return res;
    }
    int32_t fileCount = 0;
    if (!reply.ReadInt32(fileCount) || fileCount > MAX_FILE_NUM) {
        HIVIEW_LOGE("read file count failed, count: %{public}d", fileCount);
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    for (int32_t i = 0; i < fileCount; ++i) {
        std::unique_ptr<HiviewFileInfo> fileInfoPtr(reply.ReadParcelable<HiviewFileInfo>());
        if (!fileInfoPtr) {
            HIVIEW_LOGE("read file info failed.");
            fileInfos.clear();
            return HiviewNapiErrCode::ERR_DEFAULT;
        }
        fileInfos.push_back(*fileInfoPtr);
    }
    return ERR_OK;
}

int32_t HiviewServiceAbilityProxy::Copy(const std::string& logType, const std::string& logName, const std::string& dest)
{
    HIVIEW_LOGI("start copy.");
    return CopyOrMoveFile(logType, logName, dest, false);
}

int32_t HiviewServiceAbilityProxy::Move(const std::string& logType, const std::string& logName, const std::string& dest)
{
    HIVIEW_LOGI("start move.");
    return CopyOrMoveFile(logType, logName, dest, true);
}

int32_t HiviewServiceAbilityProxy::CopyOrMoveFile(
    const std::string& logType, const std::string& logName, const std::string& dest, bool isMove)
{
    MessageParcel data;
    int32_t constructRequestRet = ConstructRequestParcel(data, logType, logName, dest);
    if (constructRequestRet != ERR_OK) {
        return constructRequestRet;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t res = Remote()->SendRequest(isMove ? HIVIEW_SERVICE_ID_MOVE : HIVIEW_SERVICE_ID_COPY, data, reply, option);
    if (res != ERR_OK) {
        HIVIEW_LOGE("send request failed, error is %{public}d.", res);
        return res;
    }
    int32_t result = 0;
    if (!reply.ReadInt32(result)) {
        HIVIEW_LOGE("parcel read result failed.");
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    return result;
}

int32_t HiviewServiceAbilityProxy::Remove(const std::string& logType, const std::string& logName)
{
    MessageParcel data;
    int32_t constructRequestRet = ConstructRequestParcel(data, logType, logName);
    if (constructRequestRet != ERR_OK) {
        return constructRequestRet;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t res = Remote()->SendRequest(HIVIEW_SERVICE_ID_REMOVE, data, reply, option);
    if (res != ERR_OK) {
        HIVIEW_LOGE("send request failed, error is %{public}d.", res);
        return res;
    }
    int32_t result = 0;
    if (!reply.ReadInt32(result)) {
        HIVIEW_LOGE("parcel read result failed.");
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    return result;
}

int32_t HiviewServiceAbilityProxy::ConstructRequestParcel(
    MessageParcel& data, const std::string& logType, const std::string& logName, const std::string& dest)
{
    if (!data.WriteInterfaceToken(HiviewServiceAbilityProxy::GetDescriptor())) {
        HIVIEW_LOGE("write descriptor failed.");
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    if (!logType.empty() && !data.WriteString(logType)) {
        HIVIEW_LOGE("write log type failed.");
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    if (!logName.empty() && !data.WriteString(logName)) {
        HIVIEW_LOGE("write log name failed.");
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    if (!dest.empty() && !data.WriteString(dest)) {
        HIVIEW_LOGE("write dest failed.");
        return HiviewNapiErrCode::ERR_DEFAULT;
    }
    return ERR_OK;
}
} // namespace HiviewDFX
} // namespace OHOS
