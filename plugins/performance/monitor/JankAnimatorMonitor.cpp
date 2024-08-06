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

#include "JankAnimatorMonitor.h"
#include "AnimatorSceneDataProcessor.h"
#include "hisysevent.h"
#include "ActionId.h"
#include "JankAnimatorMonitorConverter.h"
#include "hiview_logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_LABEL(0xD002D66, "Hiview-XPerformance");

using ActionId::JANK_ANIMATOR_FRAME;
using OHOS::HiviewDFX::HiSysEvent;

namespace {
    constexpr const uint64_t C_LEVEL = 50;
}

JankAnimatorMonitor::JankAnimatorMonitor(IMonitorThrExecutor* thr, IAnimatorSceneDataProcessor* stats,
                                         IJankAnimatorReporter* report)
{
    this->exec = thr;
    this->stats = stats;
    this->reporter = report;
    this->actionId = JANK_ANIMATOR_FRAME;
}

void JankAnimatorMonitor::HandleEvt(std::shared_ptr<XperfEvt> evt)
{
    HIVIEW_LOGD("[JankAnimatorMonitor::HandleEvt]");
    if (exec != nullptr) {
        exec->ExecuteMonitorInMainThr(this, evt);
    } else {
        HIVIEW_LOGE("[JankAnimatorMonitor::HandleEvt] exec is null");
    }
}

void JankAnimatorMonitor::HandleMainThrEvt(std::shared_ptr<XperfEvt> evt)
{
    HIVIEW_LOGD("[JankAnimatorMonitor::HandleMainThrEvt]");
    try {
        std::shared_ptr<XperfEvt> event = evt;
        ProcessStats(event);
    } catch (std::invalid_argument& ex) {
        HIVIEW_LOGE("invalid argument");
    }
}

void JankAnimatorMonitor::ProcessStats(std::shared_ptr<XperfEvt> evt)
{
    HIVIEW_LOGD("[JankAnimatorMonitor::ProcessStats]");
    if (stats != nullptr) {
        stats->ProcessSceneData(evt);
    } else {
        HIVIEW_LOGE("[JankAnimatorMonitor::ProcessStats] stats is null");
    }
}

void JankAnimatorMonitor::ReportMetrics(const AnimatorMetrics& metrics)
{
    HIVIEW_LOGD("[JankAnimatorMonitor::ReportMetrics]");
    ReportNormal(metrics);
    try {
        uint64_t maxFrame = (metrics.appInfo.maxFrameTime > metrics.rsInfo.maxFrameTime) ?
                metrics.appInfo.maxFrameTime : metrics.rsInfo.maxFrameTime;
        uint64_t val = metrics.appInfo.isDisplayAnimator ? maxFrame : metrics.rsInfo.maxFrameTime;
        if (val > C_LEVEL) {
            ReportCritical(metrics, "", "");
        }
    } catch (std::logic_error& ex) {
        HIVIEW_LOGE("JankAnimatorMonitor ObtainId error: %{public}s", std::string(ex.what()).c_str());
        return;
    }
}

void JankAnimatorMonitor::ReportNormal(const AnimatorMetrics& metrics)
{
    JankAnimatorReportEvent event = JankAnimatorMonitorConverter::ConverterReportData(metrics);
    if (reporter != nullptr) {
        reporter->ReportNormal(event);
    } else {
        HIVIEW_LOGE("JankAnimatorMonitor ReportNormal reporter is null.");
    }
}

void JankAnimatorMonitor::ReportCritical(const AnimatorMetrics& metrics, const std::string& traceFileName,
    const std::string& infoFileName)
{
    JankAnimatorReportEvent event = JankAnimatorMonitorConverter::ConverterReportData(metrics, traceFileName,
                                                                                      infoFileName);
    if (reporter != nullptr) {
        reporter->ReportCritical(event);
    } else {
        HIVIEW_LOGE("JankAnimatorMonitor ReportCritical reporter is null.");
    }
}
} // HiviewDFX
} // OHOS