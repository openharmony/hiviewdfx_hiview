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
#ifndef APP_MONITOR_DATA_CONVERTER_H
#define APP_MONITOR_DATA_CONVERTER_H

#include "XperfEvt.h"
#include "IAppThrExecutor.h"

using AppEvtData = IAppThrExecutor::AppEvtData;

class AppMonitorDataConverter {
public:
    static AppEvtData ConvertXperfEvtToAppEvtData(const XperfEvt& evt)
    {
        AppEvtData appEvtData;
        appEvtData.time = evt.time;
        appEvtData.domain = evt.domain;
        appEvtData.eventName = evt.eventName;
        appEvtData.bundleName= evt.bundleName;
        appEvtData.abilityName = evt.abilityName;
        appEvtData.processName = evt.processName;
        appEvtData.pageUrl = evt.pageUrl;
        appEvtData.sceneId = evt.sceneId;
        appEvtData.sourceType = evt.sourceType;
        appEvtData.inputTime = evt.inputTime;
        appEvtData.animationStartTime = evt.animationStartTime;
        appEvtData.renderTime = evt.renderTime;
        appEvtData.responseLatency = evt.responseLatency;
        appEvtData.moduleName = evt.moduleName;
        appEvtData.versionCode = evt.versionCode;
        appEvtData.versionName = evt.versionName;
        appEvtData.startType = evt.startType;
        appEvtData.startupTime = evt.startupTime;
        appEvtData.startupAbilityType = evt.startupAbilityType;
        appEvtData.startupExtensionType = evt.startupExtensionType;
        appEvtData.callerBundleName = evt.callerBundleName;
        appEvtData.callerUid = evt.callerUid;
        appEvtData.callerProcessName = evt.callerProcessName;
        appEvtData.appPid = evt.appPid;
        appEvtData.appUid = evt.appUid;
        appEvtData.windowName = evt.windowName;
        appEvtData.bundleType = evt.bundleType;
        appEvtData.animationEndLatency = evt.animationEndLatency;
        appEvtData.e2eLatency = evt.e2eLatency;
        appEvtData.exitTime = evt.exitTime;
        appEvtData.exitResult = evt.exitResult;
        appEvtData.exitPid = evt.exitPid;
        appEvtData.note = evt.note;
        appEvtData.animationStartLatency = evt.animationStartLatency;

        return appEvtData;
    }
};
#endif