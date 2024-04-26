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

#include "export_base_task.h"

#include "hiview_logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-ExportBaseTask");
namespace {
constexpr int64_t DEFAULT_TASK_CYCLE = 1; // unit: hour
}

void ExportBaseTask::Run()
{
    OnTaskRun();
}

TaskCycle ExportBaseTask::GetExecutingCycle()
{
    if (config_ == nullptr) {
        HIVIEW_LOGE("export config of this task is invalid");
        return TaskCycle(DEFAULT_TASK_CYCLE);
    }
    return TaskCycle(config_->taskCycle);
}
} // HiviewDFX
} // OHOS