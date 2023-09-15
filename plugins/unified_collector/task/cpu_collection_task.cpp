/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include "cpu_collection_task.h"

namespace OHOS {
namespace HiviewDFX {
CpuCollectionTask::CpuCollectionTask(const std::string& workPath) : workPath_(workPath)
{
    InitCpuCollector();
    InitCpuStorage();
}

void CpuCollectionTask::Collect()
{
    ReportCpuCollectionEvent();
    CollectCpuData();
}

void CpuCollectionTask::InitCpuCollector()
{
    cpuCollector_ = UCollectUtil::CpuCollector::Create();
}

void CpuCollectionTask::InitCpuStorage()
{
    cpuStorage_ = std::make_shared<CpuStorage>(workPath_);
}

void CpuCollectionTask::ReportCpuCollectionEvent()
{
    cpuStorage_->Report();
}

void CpuCollectionTask::CollectCpuData()
{
    auto cpuCollectionsResult = cpuCollector_->CollectProcessCpuStatInfos();
    if (cpuCollectionsResult.retCode == UCollect::UcError::SUCCESS) {
        cpuStorage_->Store(cpuCollectionsResult.data);
    }
}
}  // namespace HiviewDFX
}  // namespace OHOS
