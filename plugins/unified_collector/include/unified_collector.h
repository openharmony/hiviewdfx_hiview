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
#ifndef HIVIEW_PLUGINS_UNIFIED_COLLECTOR_INCLUDE_UNIFIED_COLLECTOR_H
#define HIVIEW_PLUGINS_UNIFIED_COLLECTOR_INCLUDE_UNIFIED_COLLECTOR_H

#include <memory>

#include "plugin.h"
#include "cpu_collection_task.h"

namespace OHOS {
namespace HiviewDFX {
class UnifiedCollector : public Plugin {
public:
    void OnLoad() override;
    void OnUnload() override;
    void OnEventListeningCallback(const Event& event) override;

private:
    void Init();
    void InitWorkLoop();
    void InitWorkPath();
    void RunCpuCollectionTask();
    void RunIoCollectionTask();
    void IoCollectionTask();

private:
    std::string workPath_;
    std::shared_ptr<CpuCollectionTask> cpuCollectionTask_;
}; // UnifiedCollector
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_UNIFIED_COLLECTOR_INCLUDE_UNIFIED_COLLECTOR_H
