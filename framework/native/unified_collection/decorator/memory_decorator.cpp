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

#include "memory_collector_impl.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
const std::string MEM_COLLECTOR_NAME = "MemoryCollector";
StatInfoWrapper MemoryDecorator::statInfoWrapper_;

std::shared_ptr<MemoryCollector> MemoryCollector::Create()
{
    std::shared_ptr<MemoryDecorator> instance = std::make_shared<MemoryDecorator>();
    return instance;
}

MemoryDecorator::MemoryDecorator()
{
    memoryCollector_ = std::make_shared<MemoryCollectorImpl>();
}

CollectResult<ProcessMemory> MemoryDecorator::CollectProcessMemory(int32_t pid)
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<ProcessMemory> result = memoryCollector_->CollectProcessMemory(pid);
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<SysMemory> MemoryDecorator::CollectSysMemory()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<SysMemory> result = memoryCollector_->CollectSysMemory();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::string> MemoryDecorator::CollectRawMemInfo()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::string> result = memoryCollector_->CollectRawMemInfo();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::vector<ProcessMemory>> MemoryDecorator::CollectAllProcessMemory()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::vector<ProcessMemory>> result = memoryCollector_->CollectAllProcessMemory();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}
CollectResult<std::string> MemoryDecorator::ExportAllProcessMemory()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::string> result = memoryCollector_->ExportAllProcessMemory();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::string> MemoryDecorator::CollectRawSlabInfo()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::string> result = memoryCollector_->CollectRawSlabInfo();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::string> MemoryDecorator::CollectRawPageTypeInfo()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::string> result = memoryCollector_->CollectRawPageTypeInfo();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::string> MemoryDecorator::CollectRawDMA()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::string> result = memoryCollector_->CollectRawDMA();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::vector<AIProcessMem>> MemoryDecorator::CollectAllAIProcess()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::vector<AIProcessMem>> result = memoryCollector_->CollectAllAIProcess();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::string> MemoryDecorator::ExportAllAIProcess()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::string> result = memoryCollector_->ExportAllAIProcess();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::string> MemoryDecorator::CollectRawSmaps(int32_t pid)
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::string> result = memoryCollector_->CollectRawSmaps(pid);
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::string> MemoryDecorator::CollectHprof(int32_t pid)
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::string> result = memoryCollector_->CollectHprof(pid);
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<uint64_t> MemoryDecorator::CollectProcessVss(int32_t pid)
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<uint64_t> result = memoryCollector_->CollectProcessVss(pid);
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = MEM_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
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
