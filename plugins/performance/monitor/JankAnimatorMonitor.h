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
#ifndef JANK_ANIMATOR_MONITOR_H
#define JANK_ANIMATOR_MONITOR_H

#include "IAnimatorSceneDataProcessor.h"
#include "DefaultMonitor.h"
#include "IMonitorThrExecutor.h"
#include "ITask.h"
#include "MonitorConfig.h"
#include "XperfEvt.h"
#include "DefaultMonitor.h"
#include "IJankAnimatorReporter.h"

using AnimatorMetrics = IAnimatorSceneDataProcessor::AnimatorMetrics;

class JankAnimatorMonitor : public DefaultMonitor, public IAnimatorSceneDataProcessor::MetricReporter,
    public IMonitorThrExecutor::IHandleMonitorEvt {
public:
    JankAnimatorMonitor(IMonitorThrExecutor* thr, IAnimatorSceneDataProcessor* stats, IJankAnimatorReporter* report);

    void HandleEvt(std::shared_ptr <XperfEvt> evt) override;
    void HandleMainThrEvt(std::shared_ptr <XperfEvt> evt) override;

protected:
    void ProcessStats(std::shared_ptr<XperfEvt> evt);
    void ReportMetrics(const AnimatorMetrics &metrics) override;

private:
    int actionId{0};
    IMonitorThrExecutor* exec{nullptr};
    IAnimatorSceneDataProcessor* stats{nullptr};
    IJankAnimatorReporter* reporter{nullptr};

    void ReportNormal(const AnimatorMetrics& metrics);
    void ReportCritical(const AnimatorMetrics& metrics, const std::string& traceFileName,
                        const std::string& infoFileName);
};

#endif