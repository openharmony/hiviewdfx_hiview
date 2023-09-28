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

#ifndef HIVIEW_SERVICE_ABILITY_PROXY_H
#define HIVIEW_SERVICE_ABILITY_PROXY_H

#include <string>

#include "hiview_err_code.h"
#include "hiview_file_info.h"
#include "ihiview_service_ability.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace HiviewDFX {
class HiviewServiceAbilityProxy : public IRemoteProxy<IHiviewServiceAbility> {
public:
    explicit HiviewServiceAbilityProxy(const sptr<IRemoteObject> &remote)
        : IRemoteProxy<IHiviewServiceAbility>(remote) {}
    virtual ~HiviewServiceAbilityProxy() = default;

    int32_t List(const std::string& logType, std::vector<HiviewFileInfo>& fileInfos) override;
    int32_t Copy(const std::string& logType, const std::string& logName, const std::string& dest) override;
    int32_t Move(const std::string& logType, const std::string& logName, const std::string& dest) override;
    int32_t Remove(const std::string& logType, const std::string& logName) override;

    CollectResultParcelable<int32_t> OpenSnapshotTrace(const std::vector<std::string>& tagGroups) override;
    CollectResultParcelable<std::vector<std::string>> DumpSnapshotTrace() override;
    CollectResultParcelable<int32_t> OpenRecordingTrace(const std::string& tags) override;
    CollectResultParcelable<int32_t> RecordingTraceOn() override;
    CollectResultParcelable<std::vector<std::string>> RecordingTraceOff() override;
    CollectResultParcelable<int32_t> CloseTrace() override;
    CollectResultParcelable<int32_t> RecoverTrace() override;

private:
    int32_t CopyOrMoveFile(
        const std::string& logType, const std::string& logName, const std::string& dest, bool isMove);

    template<typename T>
    CollectResultParcelable<T> SendTraceRequest(
        HiviewServiceInterfaceCode requestCode, std::function<bool(MessageParcel&)> parcelHandler)
    {
        auto traceRet = CollectResultParcelable<T>::Init();
        MessageParcel data;
        if (!data.WriteInterfaceToken(HiviewServiceAbilityProxy::GetDescriptor()) ||
            (parcelHandler == nullptr) || !parcelHandler(data)) {
            return traceRet;
        }
        MessageParcel reply;
        MessageOption option;
        int32_t ret = Remote()->SendRequest(static_cast<uint32_t>(requestCode), data, reply, option);
        if (ret != TraceErrCode::ERR_OK) {
            return traceRet;
        }
        auto readParcel = reply.ReadParcelable<CollectResultParcelable<T>>();
        if (readParcel == nullptr) {
            return traceRet;
        }
        traceRet = *readParcel;
        return traceRet;
    }
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_SERVICE_ABILITY_PROXY_H
