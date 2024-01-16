/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef ASH_MEMORY_UTILS_H
#define ASH_MEMORY_UTILS_H

#include "ashmem.h"
#include "refbase.h"

namespace OHOS {
namespace HiviewDFX {
class AshMemoryUtils {
public:
    template <class T>
    static bool WriteBulkData(const std::vector<T>& src, sptr<Ashmem> ashmem, size_t ashSize,
        std::vector<uint32_t>& allSize)
    {
        uint32_t offset = 0;
        for (auto& dataInfo : src) {
            uint32_t dataSize = 0;
            void* data = dataInfo.GetData(dataSize);
            if (data == nullptr) {
                continue;
            }
            if (offset + dataSize > ashSize) {
                free(data);
                break;
            }
            allSize.emplace_back(dataSize);
            if (!ashmem->WriteToAshmem(data, dataSize, offset)) {
                free(data);
                return false;
            }
            free(data);
            offset += dataSize;
        }
        return true;
    }

    template <class T>
    static bool ReadBulkData(sptr<Ashmem> ashmem, const std::vector<uint32_t>& allSize, std::vector<T>& dest)
    {
        uint32_t offset = 0;
        for (uint32_t i = 0; i < allSize.size(); i++) {
            auto origin = ashmem->ReadFromAshmem(allSize[i], offset);
            if (origin == nullptr) {
                return false;
            }
            auto dataInfo = T::ParseData(reinterpret_cast<const char*>(origin), allSize[i]);
            offset += allSize[i];
            dest.emplace_back(dataInfo);
        }
        return true;
    }

    static sptr<Ashmem> GetAshmem(const std::string& ashName, const uint32_t ashSize);
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // ASH_MEMORY_UTILS_H