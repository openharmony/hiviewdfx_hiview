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
#ifndef HIVIEWDFX_HIVIEW_UC_TELEMETRY_LISTENER_H
#define HIVIEWDFX_HIVIEW_UC_TELEMETRY_LISTENER_H

#include <map>

#include "event.h"
#include "plugin.h"
#include "trace_flow_controller.h"
#include "trace_state_machine.h"

namespace OHOS::HiviewDFX {
struct StartMsg {
    int64_t beginTime;
    int64_t endTime;
    std::string telemetryId;
    std::string bundleName;
};

class TelemetryListener : public EventListener {
public:
    explicit TelemetryListener(std::shared_ptr<Plugin> myPlugin) : myPlugin_(myPlugin) {}
    void OnUnorderedEvent(const Event &msg) override;

    std::string GetListenerName() override
    {
        return "TelemetryListener";
    }

private:
    std::string GetValidParam(const Event &msg, bool &isCloseMsg);
    bool InitAndCorrectTimes(const Event &msg);
    bool SendStartEvent(const Event &msg, int64_t traceDuration, int64_t delayTime = 0);
    void SendStopEvent();
    void WriteErrorEvent(const std::string &error);

private:
    std::weak_ptr<Plugin> myPlugin_;
    std::string telemetryId_;
    std::string bundleName_;
    int64_t beginTime_ = -1;
    int64_t endTime_ = -1;
};

}

#endif //HIVIEWDFX_HIVIEW_UC_TELEMETRY_LISTENER_H
