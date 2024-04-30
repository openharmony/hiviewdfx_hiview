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
#ifndef EVENT_OBSERVER_CONVERTER_H
#define EVENT_OBSERVER_CONVERTER_H

#include "IEventObserver.h"
#include "app_event_handler.h"

using AppLaunchInfo = OHOS::HiviewDFX::AppEventHandler::AppLaunchInfo;
using AppStartInfo = IEventObserver::AppStartInfo;
using ScrollJankEvtInfo = OHOS::HiviewDFX::AppEventHandler::ScrollJankInfo;
using ScrollJankInfo = IEventObserver::ScrollJankInfo;

class EventObserverConverter {
public:
    static AppLaunchInfo ConvertToAppStartInfo(const AppStartInfo& data)
    {
        AppLaunchInfo info;
        info.bundleName = data.bundleName;
        info.bundleVersion = data.versionName;
        info.processName = data.processName;
        info.startType = data.startType;
        info.iconInputTime = data.inputTime;
        info.animationFinishTime = data.animationLatency;
        info.extendTime = data.drawnLatency;
        return info;
    }

    static ScrollJankEvtInfo ConvertToScrollJankEvtInfo(const ScrollJankInfo& data)
    {
        ScrollJankEvtInfo info;
        info.bundleVersion = data.versionName;
        info.bundleName = data.bundleName;
        info.processName = data.processName;
        info.abilityName = data.abilityName;
        info.beginTime = data.startTime;
        info.duration = data.duration;
        info.totalAppFrames = data.totalAppFrames;
        info.totalAppMissedFrames = data.totalAppMissedFrames;
        info.maxAppFrametime = data.maxAppFrameTime;
        info.maxAppSeqFrames = data.maxAppSeqMissedFrames;
        info.totalRenderFrames = data.totalRenderFrames;
        info.totalRenderMissedFrames = data.totalRenderMissedFrames;
        info.maxRenderFrametime = data.maxRenderFrameTime;
        info.maxRenderSeqFrames = data.maxRenderSeqMissedFrames;
        return info;
    }
};
#endif