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

#ifndef OHOS_HIVIEWDFX_ADAPTER_SERVICE_IDL_INCLUDE_COLLECT_RESULT_PARCELABLE_H
#define OHOS_HIVIEWDFX_ADAPTER_SERVICE_IDL_INCLUDE_COLLECT_RESULT_PARCELABLE_H

#include "parcel.h"

#include <vector>

#include "collect_result.h"

namespace OHOS {
namespace HiviewDFX {
template<typename T>
struct CollectResultParcelable : public Parcelable {
    CollectResultParcelable(CollectResult<T>& result)
    {
        result_.retCode = result.retCode;
        if constexpr (std::is_same_v<std::decay_t<T>, int32_t>) {
            result_.data = result.data;
        }
        if constexpr (std::is_same_v<std::decay_t<T>, std::vector<std::string>>) {
            result_.data.insert(result_.data.begin(), result.data.begin(), result.data.end());
        }
    }

    bool Marshalling(Parcel& outParcel) const override
    {
        if (!outParcel.WriteInt32(result_.retCode)) {
            return false;
        }
        if constexpr (std::is_same_v<std::decay_t<T>, int32_t>) {
            return outParcel.WriteInt32(result_.data);
        }
        if constexpr (std::is_same_v<std::decay_t<T>, std::vector<std::string>>) {
            return outParcel.WriteStringVector(result_.data);
        }
        return true;
    }

    static CollectResultParcelable<T>* Unmarshalling(Parcel& inParcel)
    {
        int32_t retCode;
        if (!inParcel.ReadInt32(retCode)) {
            return nullptr;
        }
        T data;
        if constexpr (std::is_same_v<std::decay_t<T>, int32_t>) {
            if (!inParcel.ReadInt32(data)) {
                return nullptr;
            }
        }
        if constexpr (std::is_same_v<std::decay_t<T>, std::vector<std::string>>) {
            if (!inParcel.ReadStringVector(&data)) {
                return nullptr;
            }
        }
        CollectResult<T> result;
        result.retCode = UCollect::UcError(retCode);
        result.data = data;
        return new CollectResultParcelable(result);
    }

    static CollectResultParcelable<T> Init()
    {
        CollectResult<T>  ret;
        ret.retCode = UCollect::UcError::UNSUPPORT;
        CollectResultParcelable traceRet(ret);
        return traceRet;
    }

    CollectResult<T> result_;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_ADAPTER_SERVICE_IDL_INCLUDE_COLLECT_RESULT_PARCELABLE_H
