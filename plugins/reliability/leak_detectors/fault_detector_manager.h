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
#ifndef FAULT_DETECTOR_MANAGER_H
#define FAULT_DETECTOR_MANAGER_H

#include <map>
#include <string>
#include <vector>

#include "plugin.h"
#include "plugin_factory.h"
#include "app_event_publisher.h"
#include "event_loop.h"
#include "event_source.h"
#include "fault_common_base.h"
#include "ffrt.h"
#include "sys_event.h"

namespace OHOS {
namespace HiviewDFX {
class FaultDetectorManager : public FileDescriptorEventCallback, public AppEventPublisher  {
public:
    bool OnEvent(std::shared_ptr<Event> &event) override;
    bool ReadyToLoad() override;
    void OnLoad() override;
    void OnUnload() override;
    void OnEventListeningCallback(const Event &msg) override;
    void AddAppEventHandler(std::shared_ptr<AppEventHandler> handler) override;
#if defined(__HIVIEW_OHOS__)
    bool OnFileDescriptorEvent(int fd, int type) override;
    int32_t GetPollFd() override;
    int32_t GetPollType() override;
#endif

private:
    void PrepareFaultDetectorEnv();
    void InitDetectorTask();
    void MonitorProcess();
    void HandleNativeLeakDetection();

private:
    std::vector<DetectorType> detectorList_;
    static constexpr uint64_t TASK_TIMER_INTERVAL = 5; // 5s
    static const inline std::string eventNameLowmem = "LOWMEM";

    std::map<std::string, int> fileMap_;
    bool isLoopContinue_ { false };
    ffrt::task_handle processTaskHandle_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // FAULT_DETECTOR_MANAGER_H
