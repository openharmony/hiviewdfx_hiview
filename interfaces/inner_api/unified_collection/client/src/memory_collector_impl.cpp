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
#include "memory_collector.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectClient {
class MemoryCollectorImpl: public MemoryCollector {
public:
    MemoryCollectorImpl() = default;
    virtual ~MemoryCollectorImpl() = default;

public:
    virtual CollectResult<ProcessMemory> CollectProcessMemory(int32_t pid) override;
    virtual CollectResult<SysMemory> CollectSysMemory() override;
};

std::shared_ptr<MemoryCollector> MemoryCollector::Create()
{
    return std::make_shared<MemoryCollectorImpl>();
}

CollectResult<ProcessMemory> MemoryCollectorImpl::CollectProcessMemory(int32_t pid)
{
    CollectResult<ProcessMemory> result;
    return result;
}

CollectResult<SysMemory> MemoryCollectorImpl::CollectSysMemory()
{
    CollectResult<SysMemory> result;
    return result;
}
} // UCollectClient
} // HiViewDFX
} // OHOS