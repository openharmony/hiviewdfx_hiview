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
#include "uc_app_state_observer.h"

#include "logger.h"
#include "process_status.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-UnifiedCollector");
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace OHOS::AppExecFwk;

void UcAppStateObserver::OnForegroundApplicationChanged(const AppStateData& appStateData)
{
#if !(PC_APP_STATE_COLLECT_ENABLE)
    HIVIEW_LOGD("process=%{public}d, state=%{public}d", appStateData.pid, appStateData.state);
    if (appStateData.state == static_cast<int32_t>(ApplicationState::APP_STATE_FOREGROUND)) {
        ProcessStatus::GetInstance().NotifyProcessState(appStateData.pid, FOREGROUND);
    } else if (appStateData.state == static_cast<int32_t>(ApplicationState::APP_STATE_BACKGROUND)) {
        ProcessStatus::GetInstance().NotifyProcessState(appStateData.pid, BACKGROUND);
    }
#endif
}

void UcAppStateObserver::OnProcessCreated(const ProcessData& processData)
{
    HIVIEW_LOGD("process=%{public}d created", processData.pid);
    ProcessStatus::GetInstance().NotifyProcessState(processData.pid, CREATED);
}

void UcAppStateObserver::OnProcessDied(const ProcessData& processData)
{
    HIVIEW_LOGD("process=%{public}d died", processData.pid);
}
}  // namespace HiviewDFX
}  // namespace OHOS
