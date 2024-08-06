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
#ifndef APP_LAUNCH_MONITOR_H
#define APP_LAUNCH_MONITOR_H

#include "DefaultMonitor.h"
#include "IAppThrExecutor.h"
#include "IAppLaunchSceneDataProcessor.h"
#include "MonitorConfig.h"
#include "IAppStartReporter.h"
#include "IMonitorRegistry.h"

namespace OHOS {
namespace HiviewDFX {
using AppStartMetrics = IAppLaunchSceneDataProcessor::AppStartMetrics;

class AppLaunchMonitor :
    public DefaultMonitor,
    public IAppLaunchSceneDataProcessor::MetricReporter,
    public IAppThrExecutor::IProcessAppEvtTask {
public:
    AppLaunchMonitor(IMonitorRegistry* registry, IAppThrExecutor* exec, IAppStartReporter* reporter,
                     IAppLaunchSceneDataProcessor* scene);
    void HandleEvt(std::shared_ptr<XperfEvt> evt) override;
    void ReportMetrics(const AppStartMetrics& metrics) override;
    void ExecuteProcessAppEvtTaskInMainThr(const IAppThrExecutor::AppEvtData& data) override;

private:
    IAppThrExecutor* exec{nullptr};
    IAppLaunchSceneDataProcessor* scene{nullptr};
    IAppStartReporter* reporter{nullptr};
    IMonitorRegistry* registry{nullptr};
    int actionId{0};
    AppStartReportEvent ConstructReportEvent(const AppStartMetrics& metrics);
    void ReportNormal(const AppStartMetrics& metrics);
};
} // HiviewDFX
} // OHOS
#endif