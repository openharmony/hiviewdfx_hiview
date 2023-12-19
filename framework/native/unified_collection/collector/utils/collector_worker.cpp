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
#include "collector_worker.h"
#include "logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("UCollectUtil-TraceCollector");
TraceWorker TraceWorker::traceWorker_;
static UcollectionWorker g_worker = nullptr;

TraceWorker& TraceWorker::GetInstance()
{
    return traceWorker_;
}

void TraceWorker::DefaultWorker(UcollectionTask task)
{
    task();
}

void TraceWorker::RegisterCollectorWorker(UcollectionWorker ucollectionWorker)
{
    g_worker = ucollectionWorker;
}

void TraceWorker::HandleUcollectionTask(UcollectionTask ucollectionTask)
{
    if (g_worker == nullptr) {
        DefaultWorker(ucollectionTask);
        HIVIEW_LOGI("g_worker is null.");
        return;
    }
    g_worker(ucollectionTask);
}
} // HiViewDFX
} // OHOS
