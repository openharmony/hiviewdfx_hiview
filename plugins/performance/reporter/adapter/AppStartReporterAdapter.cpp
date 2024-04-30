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
#include "AppStartReporterAdapter.h"

#include "hiview_logger.h"
DEFINE_LOG_TAG("XPerformance");

AppStartReporterAdapter::AppStartReporterAdapter(IAppStartReportInfrastructure* impl, IAppStartEventPoster* poster)
    : SimpleAppStartReporterAdapter(impl)
{
    this->eventPoster = poster;
}

void AppStartReporterAdapter::ReportNormal(const AppStartReportEvent& event)
{
    try {
        AppStartReportData data = ConvertReportEventToData(event);
        reporter->ReportNormal(data);

        AppStartEventInfo evt = ConvertReportEventToEventInfo(event);
        HIVIEW_LOGD("[AppStartReporterAdapter::ReportNormal] PostAppStartEvent begin");
        eventPoster->PostAppStartEvent(evt);
        HIVIEW_LOGD("[AppStartReporterAdapter::ReportNormal] PostAppStartEvent end");
    } catch (std::logic_error& ex) {
        HIVIEW_LOGE("[AppStartReporterAdapter::ReportNormal] exception:%{public}s", ex.what());
    }
}

AppStartEventInfo AppStartReporterAdapter::ConvertReportEventToEventInfo(const AppStartReportEvent& event)
{
    AppStartEventInfo info;
    info.appPid = event.appPid;
    info.versionCode = event.versionCode;
    info.versionName = event.versionName;
    info.processName = event.processName;
    info.bundleName = event.bundleName;
    info.abilityName = event.abilityName;
    info.pageUrl = event.pageUrl;
    info.sceneId = event.sceneId;
    info.startType = event.startType;
    info.sourceType = event.sourceType;
    info.inputTime = event.inputTime;
    info.responseLatency = event.responseLatency;
    info.launcherToAmsStartAbilityDur = event.launcherToAmsStartAbilityDur;
    info.amsStartAbilityToProcessStartDuration = event.amsStartAbilityToProcessStartDuration;
    info.amsProcessStartToAppAttachDuration = event.amsProcessStartToAppAttachDuration;
    info.amsAppAttachToAppForegroundDuration = event.amsAppAttachToAppForegroundDuration;
    info.amsStartAbilityToAppForegroundDuration = event.amsStartAbilityToAppForegroundDuration;
    info.amsAppFgToAbilityFgDur = event.amsAppFgToAbilityFgDur;
    info.amsAbilityFgToWmsStartWinDur = event.amsAbilityFgToWmsStartWinDur;
    info.drawnLatency = event.drawnLatency;
    info.firstFrameDrawnLatency = event.firstFrameDrawnLatency;
    info.animationLatency = event.animationLatency;
    info.e2eLatency = event.e2eLatency;
    info.actionId = event.actionId;
    info.eventId = event.eventId;
    info.traceFileName = event.traceFileName;
    info.infoFileName = event.infoFileName;
    info.happenTime = event.happenTime;
    return info;
}