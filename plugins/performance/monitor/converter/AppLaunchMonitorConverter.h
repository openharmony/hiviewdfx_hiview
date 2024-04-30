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
#ifndef APP_LAUNCH_MONITOR_CONVERTER_H
#define APP_LAUNCH_MONITOR_CONVERTER_H

#include "IAppLaunchSceneDataProcessor.h"
#include "IAppStartReporter.h"
#include "AppMonitorDataConverter.h"

const std::string COMMENT_APP_START = "APP_START";
const std::string COMMENT_APP_START_SLOW = "APP_START_SLOW";

const unsigned int APP_START_ACTION_ID = 11;
const unsigned int APP_START_EVENT_ID = 902500019;

using AppStartCheckPointData = IAppLaunchSceneDataProcessor::AppStartCheckPointData;
using AppEvtData = IAppThrExecutor::AppEvtData;
using AppStartMetrics = IAppLaunchSceneDataProcessor::AppStartMetrics;

class AppLaunchMonitorConverter : public AppMonitorDataConverter {
public:
    static AppStartCheckPointData ConvertAppEvtDataToCheckPointData(const AppEvtData& appEvtData)
    {
        AppStartCheckPointData checkPointData;
        checkPointData.time = appEvtData.time;
        checkPointData.domain = appEvtData.domain;
        checkPointData.eventName = appEvtData.eventName;
        checkPointData.bundleName= appEvtData.bundleName;
        checkPointData.abilityName = appEvtData.abilityName;
        checkPointData.processName = appEvtData.processName;
        checkPointData.pageUrl = appEvtData.pageUrl;
        checkPointData.sceneId = appEvtData.sceneId;
        checkPointData.sourceType = appEvtData.sourceType;
        checkPointData.inputTime = appEvtData.inputTime;
        checkPointData.animationStartTime = appEvtData.animationStartTime;
        checkPointData.renderTime = appEvtData.renderTime;
        checkPointData.responseLatency = appEvtData.responseLatency;
        checkPointData.moduleName = appEvtData.moduleName;
        checkPointData.versionCode = appEvtData.versionCode;
        checkPointData.versionName = appEvtData.versionName;
        checkPointData.startType = appEvtData.startType;
        checkPointData.startupTime = appEvtData.startupTime;
        checkPointData.startupAbilityType = appEvtData.startupAbilityType;
        checkPointData.startupExtensionType = appEvtData.startupExtensionType;
        checkPointData.callerBundleName = appEvtData.callerBundleName;
        checkPointData.callerUid = appEvtData.callerUid;
        checkPointData.callerProcessName = appEvtData.callerProcessName;
        checkPointData.appPid = appEvtData.appPid;
        checkPointData.appUid = appEvtData.appUid;
        checkPointData.windowName = appEvtData.windowName;
        checkPointData.bundleType = appEvtData.bundleType;
        checkPointData.animationEndLatency = appEvtData.animationEndLatency;
        checkPointData.e2eLatency = appEvtData.e2eLatency;
        checkPointData.note = appEvtData.note;
        checkPointData.animationStartLatency = appEvtData.animationStartLatency;

        return checkPointData;
    }

    static AppStartReportEvent ConvertMetricToReportEvent(IAppLaunchSceneDataProcessor::AppStartMetrics metrics)
    {
        AppStartReportEvent reportEvent;
        reportEvent.appPid = metrics.appPid;
        reportEvent.versionCode = metrics.versionCode;
        reportEvent.versionName = metrics.versionName;
        reportEvent.processName = metrics.processName;
        reportEvent.bundleName = metrics.bundleName;
        reportEvent.abilityName = metrics.abilityName;
        reportEvent.pageUrl = metrics.pageUrl;
        reportEvent.sceneId = metrics.sceneId;
        reportEvent.startType = metrics.startType;
        reportEvent.sourceType = metrics.sourceType;
        reportEvent.inputTime = metrics.inputTime;
        reportEvent.responseLatency = metrics.responseLatency;
        reportEvent.launcherToAmsStartAbilityDur = metrics.launcherToAmsStartAbilityDur;
        reportEvent.amsStartAbilityToProcessStartDuration = metrics.amsStartAbilityToProcessStartDuration;
        reportEvent.amsProcessStartToAppAttachDuration = metrics.amsProcessStartToAppAttachDuration;
        reportEvent.amsAppAttachToAppForegroundDuration = metrics.amsAppAttachToAppForegroundDuration;
        reportEvent.amsStartAbilityToAppForegroundDuration = metrics.amsStartAbilityToAppForegroundDuration;
        reportEvent.amsAppFgToAbilityFgDur = metrics.amsAppFgToAbilityFgDur;
        reportEvent.amsAbilityFgToWmsStartWinDur = metrics.amsAbilityFgToWmsStartWinDur;
        reportEvent.drawnLatency = metrics.drawnLatency;
        reportEvent.firstFrameDrawnLatency = metrics.firstFrameDrawnLatency;
        reportEvent.animationEndLatency = metrics.animationEndLatency;
        reportEvent.animationLatency = metrics.animationLatency;
        reportEvent.e2eLatency = metrics.e2eLatency;
        reportEvent.happenTime = metrics.happenTime;

        reportEvent.actionId = APP_START_ACTION_ID;
        reportEvent.eventId = APP_START_EVENT_ID;
        return reportEvent;
    }
};
#endif