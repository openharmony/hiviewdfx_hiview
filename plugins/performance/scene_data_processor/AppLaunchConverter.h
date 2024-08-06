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
#ifndef APP_LAUNCH_CONVERTER_H
#define APP_LAUNCH_CONVERTER_H

#include "IAppLaunchSceneDataProcessor.h"
#include "IAppLaunchSceneDb.h"

namespace OHOS {
namespace HiviewDFX {
using AppStartCheckPointData = IAppLaunchSceneDataProcessor::AppStartCheckPointData;

class AppLaunchConverter {
public:
    static IAppLaunchSceneDb::InteractionResponse ConvertToInteractionResponse(const AppStartCheckPointData& data)
    {
        IAppLaunchSceneDb::InteractionResponse point;

        point.time = data.time;
        point.domain = data.domain;
        point.eventName = data.eventName;

        point.appPid = data.appPid;
        point.versionCode = data.versionCode;
        point.versionName = data.versionName;
        point.bundleName = data.bundleName;
        point.abilityName = data.abilityName;
        point.processName = data.processName;
        point.pageUrl = data.pageUrl;
        point.sceneId = data.sceneId;
        point.sourceType = data.sourceType;
        point.inputTime = data.inputTime;
        point.animationStartTime =data.animationStartTime;
        point.renderTime = data.renderTime;
        point.responseLatency = data.responseLatency;
        point.note = data.note;

        return point;
    }
    static IAppLaunchSceneDb::StartAbility ConvertToStartAbility(const AppStartCheckPointData& data)
    {
        StartAbility point;
        point.time = data.time;
        point.domain = data.domain;
        point.eventName = data.eventName;
        point.bundleName = data.bundleName;
        point.moduleName = data.moduleName;
        point.abilityName = data.abilityName;

        return point;
    }
    static IAppLaunchSceneDb::AppStartupType ConvertToAppStartupType(const AppStartCheckPointData& data)
    {
        AppStartupType point;
        point.time = data.time;
        point.domain = data.domain;
        point.eventName = data.eventName;

        point.appPid = data.appPid;
        point.versionCode = data.versionCode;
        point.versionName = data.versionName;
        point.bundleName = data.bundleName;
        point.abilityName = data.abilityName;
        point.startType = data.startType;
        return point;
    }
    static IAppLaunchSceneDb::ProcessStart ConvertToProcessStart(const AppStartCheckPointData& data)
    {
        ProcessStart point;
        point.time = data.time;
        point.domain = data.domain;
        point.eventName = data.eventName;
        point.bundleName = data.bundleName;
        point.startupTime = data.startupTime;
        point.startupAbilityType = data.startupAbilityType;
        point.startupExtensionType = data.startupExtensionType;
        point.callerBundleName= data.callerBundleName;
        point.callerUid = data.callerUid;
        point.callerProcessName = data.callerProcessName;

        return point;
    }
    static IAppLaunchSceneDb::AppAttach ConvertToAppAttach(const AppStartCheckPointData& data)
    {
        AppAttach point;
        point.time = data.time;
        point.domain = data.domain;
        point.eventName = data.eventName;
        point.bundleName = data.bundleName;
        point.appPid = data.appPid;
        point.versionCode = data.versionCode;
        point.versionName = data.versionName;
        point.processName= data.processName;

        return point;
    }
    static IAppLaunchSceneDb::AppForeground ConvertToAppForeground(const AppStartCheckPointData& data)
    {
        AppForeground point;
        point.time = data.time;
        point.domain = data.domain;
        point.eventName = data.eventName;
        point.bundleName = data.bundleName;
        point.appPid = data.appPid;
        point.versionCode = data.versionCode;
        point.versionName = data.versionName;
        point.processName= data.processName;
        point.bundleType = data.bundleType;
        point.callerBundleName = data.callerBundleName;

        return point;
    }
    static IAppLaunchSceneDb::AbilityForeground ConvertToAbilityForeground(const AppStartCheckPointData& data)
    {
        AbilityForeground point;
        point.time = data.time;
        point.domain = data.domain;
        point.eventName = data.eventName;
        point.bundleName = data.bundleName;
        point.moduleName = data.moduleName;
        point.abilityName = data.abilityName;
        point.bundleType = data.bundleType;
        point.callerBundleName = data.callerBundleName;

        return point;
    }
    static IAppLaunchSceneDb::StartWindow ConvertToStartWindow(const AppStartCheckPointData& data)
    {
        StartWindow point;
        point.time = data.time;
        point.domain = data.domain;
        point.eventName = data.eventName;
        point.bundleName = data.bundleName;
        point.appPid = data.appPid;
        point.processName = data.processName;
        point.windowName = data.windowName;

        return point;
    }
    static IAppLaunchSceneDb::DrawnCompleted ConvertToDrawnCompleted(const AppStartCheckPointData& data)
    {
        DrawnCompleted point;
        point.time = data.time;
        point.domain = data.domain;
        point.eventName = data.eventName;

        point.appUid = data.appUid;
        point.appPid = data.appPid;
        point.bundleName = data.bundleName;
        point.moduleName = data.moduleName;
        point.abilityName = data.abilityName;

        return point;
    }
    static IAppLaunchSceneDb::FirstFrameDrawn ConvertToFirstFrameDrawn(const AppStartCheckPointData& data)
    {
        FirstFrameDrawn point;
        point.time = data.time;
        point.domain = data.domain;
        point.eventName = data.eventName;
        point.appPid = data.appPid;

        return point;
    }
    static IAppLaunchSceneDb::InteractionCompleted ConvertToInteractionCompleted(const AppStartCheckPointData& data)
    {
        InteractionCompleted point;
        point.time = data.time;
        point.domain = data.domain;
        point.eventName = data.eventName;
        point.bundleName = data.bundleName;
        point.appPid = data.appPid;
        point.versionCode = data.versionCode;
        point.versionName = data.versionName;
        point.abilityName = data.abilityName;
        point.processName = data.processName;
        point.pageUrl = data.pageUrl;
        point.sceneId = data.sceneId;
        point.sourceType = data.sourceType;
        point.inputTime = data.inputTime;
        point.animationStartLatency = data.animationStartLatency;
        point.animationEndLatency = data.animationEndLatency;
        point.e2eLatency = data.e2eLatency;
        point.note = data.note;

        return point;
    }
};
} // HiviewDFX
} // OHOS
#endif