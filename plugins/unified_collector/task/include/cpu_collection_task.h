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

#ifndef HIVIEW_PLUGINS_UNIFIED_COLLECTOR_STORAGE_INCLUDE_CPU_COLLECTION_TASK_H
#define HIVIEW_PLUGINS_UNIFIED_COLLECTOR_STORAGE_INCLUDE_CPU_COLLECTION_TASK_H

#include <memory>

#include "cpu_collector.h"
#include "cpu_storage.h"

namespace OHOS {
namespace HiviewDFX {
class CpuCollectionTask {
public:
    CpuCollectionTask(const std::string& workPath);
    ~CpuCollectionTask() = default;
    void Collect();

private:
    void InitCpuCollector();
    void InitCpuStorage();
    void ReportCpuCollectionEvent();
    void CollectCpuData();

private:
    std::string workPath_;
    std::shared_ptr<UCollectUtil::CpuCollector> cpuCollector_;
    std::shared_ptr<CpuStorage> cpuStorage_;
}; // CpuCollectionTask
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_UNIFIED_COLLECTOR_STORAGE_INCLUDE_CPU_COLLECTION_TASK_H
