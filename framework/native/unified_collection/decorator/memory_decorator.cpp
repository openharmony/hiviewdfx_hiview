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

#include "memory_decorator.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
const std::string MEM_COLLECTOR_NAME = "MemoryCollector";
StatInfoWrapper MemoryDecorator::statInfoWrapper_;

CollectResult<ProcessMemory> MemoryDecorator::CollectProcessMemory(int32_t pid)
{
    auto task = std::bind(&MemoryCollector::CollectProcessMemory, memoryCollector_.get(), pid);
    return Invoke(task, statInfoWrapper_, MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<SysMemory> MemoryDecorator::CollectSysMemory()
{
    auto task = std::bind(&MemoryCollector::CollectSysMemory, memoryCollector_.get());
    return Invoke(task, statInfoWrapper_, MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::string> MemoryDecorator::CollectRawMemInfo()
{
    auto task = std::bind(&MemoryCollector::CollectRawMemInfo, memoryCollector_.get());
    return Invoke(task, statInfoWrapper_, MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::string> MemoryDecorator::ExportMemView()
{
    auto task = std::bind(&MemoryCollector::ExportMemView, memoryCollector_.get());
    return Invoke(task, statInfoWrapper_, MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::vector<ProcessMemory>> MemoryDecorator::CollectAllProcessMemory()
{
    auto task = std::bind(&MemoryCollector::CollectAllProcessMemory, memoryCollector_.get());
    return Invoke(task, statInfoWrapper_, MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}
CollectResult<std::string> MemoryDecorator::ExportAllProcessMemory()
{
    auto task = std::bind(&MemoryCollector::ExportAllProcessMemory, memoryCollector_.get());
    return Invoke(task, statInfoWrapper_, MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::string> MemoryDecorator::CollectRawSlabInfo()
{
    auto task = std::bind(&MemoryCollector::CollectRawSlabInfo, memoryCollector_.get());
    return Invoke(task, statInfoWrapper_, MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::string> MemoryDecorator::CollectRawPageTypeInfo()
{
    auto task = std::bind(&MemoryCollector::CollectRawPageTypeInfo, memoryCollector_.get());
    return Invoke(task, statInfoWrapper_, MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::string> MemoryDecorator::CollectRawDMA()
{
    auto task = std::bind(&MemoryCollector::CollectRawDMA, memoryCollector_.get());
    return Invoke(task, statInfoWrapper_, MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::vector<AIProcessMem>> MemoryDecorator::CollectAllAIProcess()
{
    auto task = std::bind(&MemoryCollector::CollectAllAIProcess, memoryCollector_.get());
    return Invoke(task, statInfoWrapper_, MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::string> MemoryDecorator::ExportAllAIProcess()
{
    auto task = std::bind(&MemoryCollector::ExportAllAIProcess, memoryCollector_.get());
    return Invoke(task, statInfoWrapper_, MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::string> MemoryDecorator::CollectRawSmaps(int32_t pid)
{
    auto task = std::bind(&MemoryCollector::CollectRawSmaps, memoryCollector_.get(), pid);
    return Invoke(task, statInfoWrapper_, MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::string> MemoryDecorator::CollectHprof(int32_t pid)
{
    auto task = std::bind(&MemoryCollector::CollectHprof, memoryCollector_.get(), pid);
    return Invoke(task, statInfoWrapper_, MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<uint64_t> MemoryDecorator::CollectProcessVss(int32_t pid)
{
    auto task = std::bind(&MemoryCollector::CollectProcessVss, memoryCollector_.get(), pid);
    return Invoke(task, statInfoWrapper_, MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<MemoryLimit> MemoryDecorator::CollectMemoryLimit()
{
    auto task = std::bind(&MemoryCollector::CollectMemoryLimit, memoryCollector_.get());
    return Invoke(task, statInfoWrapper_, MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

void MemoryDecorator::SaveStatCommonInfo()
{
    std::map<std::string, StatInfo> statInfo = statInfoWrapper_.GetStatInfo();
    std::vector<std::string> formattedStatInfo;
    for (const auto& record : statInfo) {
        formattedStatInfo.push_back(record.second.ToString());
    }
    WriteLinesToFile(formattedStatInfo, false);
}

void MemoryDecorator::ResetStatInfo()
{
    statInfoWrapper_.ResetStatInfo();
}
} // namespace UCollectUtil
} // namespace HiviewDFX
} // namespace OHOS
