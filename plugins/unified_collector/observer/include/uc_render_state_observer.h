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

#ifndef HIVIEW_PLUGINS_UNIFIED_COLLECTOR_OBSERVER_INCLUDE_UC_RENDER_STATE_OBSERVER_H
#define HIVIEW_PLUGINS_UNIFIED_COLLECTOR_OBSERVER_INCLUDE_UC_RENDER_STATE_OBSERVER_H

#include "render_state_observer_stub.h"

namespace OHOS {
namespace HiviewDFX {
class UcRenderStateObserver : public AppExecFwk::RenderStateObserverStub {
public:
    UcRenderStateObserver() = default;
    virtual ~UcRenderStateObserver() = default;
    void OnRenderStateChanged(pid_t renderPid, int32_t state) override;
}; // UcRenderStateObserver
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_UNIFIED_COLLECTOR_OBSERVER_INCLUDE_UC_RENDER_STATE_OBSERVER_H
