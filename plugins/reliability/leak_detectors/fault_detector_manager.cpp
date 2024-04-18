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
#include "fault_detector_manager.h"

#include <memory>
#include <string>
#include <vector>

#include <sys/epoll.h>

#include "app_event_publisher.h"
#include "app_event_publisher_factory.h"
#include "event_loop.h"
#include "fault_detector_util.h"
#include "fault_state_base.h"
#include "hiview_logger.h"
#include "native_leak_detector.h"
#include "native_leak_state_context.h"
#include "plugin.h"
#include "plugin_factory.h"
#include "sys_event.h"

namespace OHOS {
namespace HiviewDFX {
using std::string;
REGISTER(FaultDetectorManager);
REGISTER_PUBLISHER(FaultDetectorManager);
DEFINE_LOG_TAG("FaultDetectorManager");

bool FaultDetectorManager::OnEvent(std::shared_ptr<Event> &event)
{
    return true;
}

bool FaultDetectorManager::ReadyToLoad()
{
    if (!FaultDetectorUtil::IsMemLeakEnable()) {
        HIVIEW_LOGW("fault detector not enable, Load is not ready");
        return false;
    }
    HIVIEW_LOGI("Load is ready");
    isLoopContinue_ = true;
    return true;
}

void FaultDetectorManager::PrepareFaultDetectorEnv()
{
    if (FaultDetectorUtil::IsMemLeakEnable()) {
        HIVIEW_LOGI("Prepare Enviroment for memory leak");
        NativeLeakDetector::GetInstance().PrepareNativeLeakEnv();
        detectorList_.push_back(NATIVE_LEAK_DETECTOR);
    }
}

void FaultDetectorManager::HandleNativeLeakDetection()
{
    ffrt::submit([] {
        ErrCode ret = NativeLeakDetector::GetInstance().MainProcess();
        if (ret) {
            HIVIEW_LOGE("NativeLeakDetector MainProcess failed, err is %{public}d", ret);
        }
    });
}

void FaultDetectorManager::MonitorProcess()
{
    for (auto item : detectorList_) {
        switch (item) {
            case NATIVE_LEAK_DETECTOR:
                HandleNativeLeakDetection();
                break;
            default:
                break;
        }
    }
}


void FaultDetectorManager::OnLoad()
{
    SetName("FaultDetectorManager");
    SetVersion("FaultDetectorManager1.0");
    HIVIEW_LOGI("FaultDetectorManager OnLoad");

    PrepareFaultDetectorEnv();

    workLoop_ = GetHiviewContext()->GetSharedWorkLoop();
    if (!workLoop_) {
        HIVIEW_LOGI("workLoop_ is nullptr. stop load.");
        return;
    }

    auto detectorProcessLoop = [this]() {
        HIVIEW_LOGI("start detectorProcessLoop task");
        while (isLoopContinue_) {
            FaultDetectorManager::MonitorProcess();
            ffrt::this_task::sleep_for(std::chrono::microseconds(TASK_TIMER_INTERVAL * 1000 * 1000)); // 5s
        }
    };
    processTaskHandle_ = ffrt::submit_h(detectorProcessLoop);
}

void FaultDetectorManager::OnUnload()
{
    isLoopContinue_ = false;
    ffrt::wait({processTaskHandle_});
    HIVIEW_LOGI("FaultDetectorManager OnUnload");
}

void FaultDetectorManager::OnEventListeningCallback(const Event &msg)
{
    Event& eventRef = const_cast<Event&>(msg);
    SysEvent& sysEvent = static_cast<SysEvent&>(eventRef);

    string eventMsg = sysEvent.GetEventValue("MSG");
    if (eventMsg.empty()) {
        eventMsg = sysEvent.eventName_;
    }
    string processName = sysEvent.GetEventValue("PROCESS_NAME");
    HIVIEW_LOGD("FaultDetector receive watchpoint report, jsonExtraInfo is %{public}s", sysEvent.AsJsonStr().c_str());

    if (sysEvent.eventName_ == eventNameLowmem) {
        NativeLeakDetector::GetInstance().ProcessUserEvent(processName, eventMsg, sysEvent.GetEventIntValue("PID"));
    }
}

void FaultDetectorManager::AddAppEventHandler(std::shared_ptr<AppEventHandler> handler)
{
    HIVIEW_LOGI("FaultDetectorManager::AddAppEventHandler");
    FaultStateBase *baseState = NativeLeakStateContext::GetInstance().GetStateObj(PROC_REPORT_STATE);
    NativeLeakReportState *reportObj = static_cast<NativeLeakReportState *>(baseState);
    reportObj->setEventHandler(handler);
}

#if defined(__HIVIEW_OHOS__)
bool FaultDetectorManager::OnFileDescriptorEvent(int fd, int type)
{
    return true;
}

int32_t FaultDetectorManager::GetPollFd()
{
    return 0;
}

int32_t FaultDetectorManager::GetPollType()
{
    return EPOLLIN;
}
#endif
} // namespace HiviewDFX
} // namespace OHOS
