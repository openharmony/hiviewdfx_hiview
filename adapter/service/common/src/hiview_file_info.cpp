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

#include "hiview_file_info.h"

#include "hiview_logger.h"
#include "securec.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("LOGLIBRARY_FILE_INFO");
void* HiviewFileInfo::GetData(uint32_t& dataSize) const
{
    Parcel outParcel;
    if (!outParcel.WriteString(name)) {
        return nullptr;
    }
    if (!outParcel.WriteInt64(mtime)) {
        return nullptr;
    }
    if (!outParcel.WriteUint64(size)) {
        return nullptr;
    }
    size_t size = outParcel.GetDataSize();
    const size_t maxSize = 1024;
    if (size == 0 || size > maxSize) {
        HIVIEW_LOGE("invalid size.");
        return nullptr;
    }
    void* data = malloc(size);
    if (data == nullptr) {
        HIVIEW_LOGE("invalid data.");
        return nullptr;
    }
    memset_s(data, size, 0, size);
    if (memcpy_s(data, size, reinterpret_cast<void*>(outParcel.GetData()), size) != 0) {
        HIVIEW_LOGE("copy failed.");
        free(data);
        return nullptr;
    }
    dataSize = size;
    return data;
}

HiviewFileInfo HiviewFileInfo::ParseData(const char* data, const uint32_t dataSize)
{
    if (data == nullptr) {
        HIVIEW_LOGE("param invalid.");
        return HiviewFileInfo("", 0, 0);
    }
    Parcel inParcel;
    inParcel.WriteBuffer(data, dataSize);
    std::string name;
    if (!inParcel.ReadString(name)) {
        return HiviewFileInfo("", 0, 0);
    }
    time_t mtime;
    if (!inParcel.ReadInt64(mtime)) {
        return HiviewFileInfo("", 0, 0);
    }
    uint64_t size;
    if (!inParcel.ReadUint64(size)) {
        return HiviewFileInfo("", 0, 0);
    }
    return HiviewFileInfo(name, mtime, size);
}
} // namespace HiviewDFX
} // namespace OHOS