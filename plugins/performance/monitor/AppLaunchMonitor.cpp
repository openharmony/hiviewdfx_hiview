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

#include "AppLaunchMonitor.h"
#include "AppLaunchMonitorConverter.h"
#include "ActionId.h"
#include "JlogId.h"
#include "hiview_logger.h"

DEFINE_LOG_LABEL(0xD002D66, "Hiview-XPerformance");

using AppStartCheckPointData = IAppLaunchSceneDataProcessor::AppStartCheckPointData;

AppLaunchMonitor::AppLaunchMonitor(IMonitorRegistry* registry, IAppThrExecutor* exec, IAppStartReporter* reporter,
                                   IAppLaunchSceneDataProcessor* scene)
{
    this->registry = registry;
    this->exec = exec;
    this->reporter = reporter;
    this->scene = scene;
    this->actionId = APP_START;
}

void AppLaunchMonitor::HandleEvt(std::shared_ptr<XperfEvt> evt)
{
    HIVIEW_LOGI("AppLaunchMonitor::HandleEvt");
    if (exec != nullptr) {
        IAppThrExecutor::AppEvtData appEvtData = AppLaunchMonitorConverter::ConvertXperfEvtToAppEvtData(*evt.get());
        exec->ExecuteHandleEvtInMainThr(this, appEvtData);
    } else {
        HIVIEW_LOGE("[AppLaunchMonitor::HandleEvt] exec is null");
    }
}

void AppLaunchMonitor::ExecuteProcessAppEvtTaskInMainThr(const IAppThrExecutor::AppEvtData& data)
{
    HIVIEW_LOGI("AppLaunchMonitor::ExecuteProcessAppEvtTaskInMainThr");
    try {
        AppStartCheckPointData cpData = AppLaunchMonitorConverter::ConvertAppEvtDataToCheckPointData(data);
        if (scene != nullptr) {
            scene->ProcessSceneData(cpData);
        } else {
            HIVIEW_LOGE("[AppLaunchMonitor::ExecuteProcessAppEvtTaskInMainThr] scene is null");
        }
    } catch (std::logic_error& ex) {
        HIVIEW_LOGD("exception error:%{public}s", std::string(ex.what()).c_str());
    }
}

void AppLaunchMonitor::ReportMetrics(const AppStartMetrics& metrics)
{
    HIVIEW_LOGI("AppLaunchMonitor::ReportMetrics");
    try {
        ReportNormal(metrics);
    } catch (std::logic_error& ex) {
        HIVIEW_LOGE("ReportMetrics error: %{public}s", std::string(ex.what()).c_str());
    }
}

void AppLaunchMonitor::ReportNormal(const AppStartMetrics& metrics)
{
    HIVIEW_LOGI("AppLaunchMonitor::ReportNormal");
    if (reporter == nullptr) {
        HIVIEW_LOGE("reporter is null.");
        return;
    }
    AppStartReportEvent reportEvent = ConstructReportEvent(metrics);
    reporter->ReportNormal(reportEvent);
}


AppStartReportEvent AppLaunchMonitor::ConstructReportEvent(const AppStartMetrics& metrics)
{
    AppStartReportEvent re = AppLaunchMonitorConverter::ConvertMetricToReportEvent(metrics);
    return re;
}