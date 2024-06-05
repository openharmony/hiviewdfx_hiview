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
#include "faultlog_info_ohos.h"

#include "hiview_logger.h"
#include "parcel.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_LABEL(0xD002D11, "FaultLogInfoOhos");
bool FaultLogInfoOhos::Marshalling(Parcel& parcel) const
{
    if (!parcel.WriteInt64(time) || !parcel.WriteInt32(uid) ||
        !parcel.WriteInt32(pid) || !parcel.WriteInt32(faultLogType)) {
        HIVIEW_LOGE("Parcel failed to write int number.");
        return false;
    }
    if (!parcel.WriteString(module)) {
        HIVIEW_LOGE("Parcel failed to write module name(%{public}s).", module.c_str());
        return false;
    }
    if (!parcel.WriteString(reason)) {
        HIVIEW_LOGE("Parcel failed to write reason(%{public}s).", reason.c_str());
        return false;
    }
    if (!parcel.WriteString(summary)) {
        HIVIEW_LOGE("Parcel failed to write summary(%{public}s).", summary.c_str());
        return false;
    }
    if (!parcel.WriteString(logPath)) {
        HIVIEW_LOGE("Parcel failed to write log path(%{public}s).", logPath.c_str());
        return false;
    }
    if (!parcel.WriteString(registers)) {
        HIVIEW_LOGE("Parcel failed to write registers(%{public}s).", registers.c_str());
        return false;
    }
    uint32_t size = sectionMaps.size();
    if (!parcel.WriteUint32(size)) {
        return false;
    }
    for (const auto& [key, value] : sectionMaps) {
        if (!parcel.WriteString(key)) {
            HIVIEW_LOGE("Parcel failed to write key of sectionMaps(%{public}s).", key.c_str());
            return false;
        }
        if (!parcel.WriteString(value)) {
            HIVIEW_LOGE("Parcel failed to write value of sectionMaps(%{public}s).", value.c_str());
            return false;
        }
    }
    return true;
}

FaultLogInfoOhos* FaultLogInfoOhos::Unmarshalling(Parcel& parcel)
{
    const uint32_t maxSize = 128;
    uint32_t size;
    uint32_t i;
    FaultLogInfoOhos* ret = new FaultLogInfoOhos();
    if (!parcel.ReadInt64(ret->time)) {
        goto error;
    }

    if (!parcel.ReadInt32(ret->uid)) {
        goto error;
    }

    if (!parcel.ReadInt32(ret->pid)) {
        goto error;
    }

    if (!parcel.ReadInt32(ret->faultLogType)) {
        goto error;
    }

    if (!parcel.ReadString(ret->module)) {
        HIVIEW_LOGE("Parcel failed to read module(%{public}s).", ret->module.c_str());
        goto error;
    }

    if (!parcel.ReadString(ret->reason)) {
        HIVIEW_LOGE("Parcel failed to read reason(%{public}s).", ret->reason.c_str());
        goto error;
    }

    if (!parcel.ReadString(ret->summary)) {
        HIVIEW_LOGE("Parcel failed to read summary(%{public}s).", ret->summary.c_str());
        goto error;
    }

    if (!parcel.ReadString(ret->logPath)) {
        HIVIEW_LOGE("Parcel failed to read log path(%{public}s).", ret->logPath.c_str());
        goto error;
    }

    if (!parcel.ReadString(ret->registers)) {
        HIVIEW_LOGE("Parcel failed to read registers(%{public}s).", ret->registers.c_str());
        goto error;
    }

    if (!parcel.ReadUint32(size)) {
        goto error;
    }

    if (size > maxSize) {
        goto error;
    }

    for (i = 0; i < size; i++) {
        std::string key;
        std::string value;
        if (!parcel.ReadString(key)) {
            HIVIEW_LOGE("Parcel failed to read key of sectionMaps(%{public}s).", key.c_str());
            goto error;
        }

        if (!parcel.ReadString(value)) {
            HIVIEW_LOGE("Parcel failed to read value of sectionMaps(%{public}s).", value.c_str());
            goto error;
        }

        ret->sectionMaps[key] = value;
    }
    return ret;

error:
    delete ret;
    ret = nullptr;
    return nullptr;
}
}  // namespace hiview
}  // namespace OHOS
