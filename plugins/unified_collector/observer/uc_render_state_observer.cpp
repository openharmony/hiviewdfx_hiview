/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "uc_render_state_observer.h"

#include "logger.h"
#include "process_status.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-UnifiedCollector");
namespace {
using namespace OHOS::HiviewDFX::UCollectUtil;

ProcessState GetProcessState(int32_t state)
{
    const std::unordered_map<int32_t, ProcessState> renderStates = {
        {0, FOREGROUND},
        {1, BACKGROUND},
    };
    if (renderStates.find(state) == renderStates.end()) {
        HIVIEW_LOGD("invalid render state=%{public}d", state);
        return INVALID;
    }
    return renderStates.at(state);
}
}

void UcRenderStateObserver::OnRenderStateChanged(const AppExecFwk::RenderStateData &renderStateData)
{
    ProcessState procState = GetProcessState(renderStateData.state);
    if (procState == INVALID) {
        return;
    }
    ProcessStatus::GetInstance().NotifyProcessState(renderStateData.pid, procState);
}
}  // namespace HiviewDFX
}  // namespace OHOS
