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
#include "SimpleAppStartReporterAdapter.h"

#include "hiview_logger.h"
DEFINE_LOG_TAG("XPerformance");

SimpleAppStartReporterAdapter::SimpleAppStartReporterAdapter(IAppStartReportInfrastructure* impl)
{
    this->reporter = impl;
}

void SimpleAppStartReporterAdapter::ReportNormal(const AppStartReportEvent& event)
{
    try {
        AppStartReportData data = ConvertReportEventToData(event);
        reporter->ReportNormal(data);
    } catch (std::logic_error& ex) {
        HIVIEW_LOGE("[SimpleAppStartReporterAdapter::ReportNormal] exception:%{public}s", ex.what());
    }
}

void SimpleAppStartReporterAdapter::ReportCritical(const AppStartReportEvent& event)
{
    try {
        AppStartReportData data = ConvertReportEventToData(event);
        reporter->ReportFault(data);
    } catch (std::logic_error& ex) {
        HIVIEW_LOGE("[SimpleAppStartReporterAdapter::ReportCritical] exception:%{public}s", ex.what());
    }
}

AppStartReportData SimpleAppStartReporterAdapter::ConvertReportEventToData(const AppStartReportEvent& event)
{
    AppStartReportData data;
    data.appPid = event.appPid;
    data.versionCode = event.versionCode;
    data.versionName = event.versionName;
    data.processName = event.processName;
    data.bundleName = event.bundleName;
    data.abilityName = event.abilityName;
    data.pageUrl = event.pageUrl;
    data.sceneId = event.sceneId;
    data.startType = event.startType;
    data.sourceType = event.sourceType;
    data.inputTime = event.inputTime;
    data.responseLatency = event.responseLatency;
    data.launcherToAmsStartAbilityDur = event.launcherToAmsStartAbilityDur;
    data.amsStartAbilityToProcessStartDuration = event.amsStartAbilityToProcessStartDuration;
    data.amsProcessStartToAppAttachDuration = event.amsProcessStartToAppAttachDuration;
    data.amsAppAttachToAppForegroundDuration = event.amsAppAttachToAppForegroundDuration;
    data.amsStartAbilityToAppForegroundDuration = event.amsStartAbilityToAppForegroundDuration;
    data.amsAppFgToAbilityFgDur = event.amsAppFgToAbilityFgDur;
    data.amsAbilityFgToWmsStartWinDur = event.amsAbilityFgToWmsStartWinDur;
    data.drawnLatency = event.drawnLatency;
    data.firstFrameDrawnLatency = event.firstFrameDrawnLatency;
    data.animationLatency = event.animationEndLatency;
    data.e2eLatency = event.e2eLatency;
    data.actionId = event.actionId;
    data.eventId = event.eventId;
    data.traceFileName = event.traceFileName;
    data.infoFileName = event.infoFileName;
    data.happenTime = event.happenTime;
    return data;
}