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
#include "EventsPoster.h"
#include "hiview_logger.h"

DEFINE_LOG_LABEL(0xD002D66, "Hiview-XPerformance");

void EventsPoster::RegObserver(IEventObserver* observer)
{
    this->observer = observer;
}

void EventsPoster::PostAppStartEvent(const AppStartEventInfo& evt)
{
    HIVIEW_LOGI("EventsPoster::PostAppStartEvent");
    AppStartInfo appStartInfo = ConvertToAppStartInfo(evt);
    observer->PostAppStartEvent(appStartInfo);
}

void EventsPoster::PostScrollJankEvent(const ScrollJankEventInfo& evt)
{
    ScrollJankInfo scrollJankInfo = ConvertToScrollJankInfo(evt);
    observer->PostScrollJankEvent(scrollJankInfo);
}

AppStartInfo EventsPoster::ConvertToAppStartInfo(const AppStartEventInfo& event)
{
    AppStartInfo info;
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

ScrollJankInfo EventsPoster::ConvertToScrollJankInfo(const ScrollJankEventInfo& event)
{
    ScrollJankInfo info;
    info.appPid = event.appPid;
    info.versionCode = event.versionCode;
    info.versionName = event.versionName;
    info.bundleName = event.bundleName;
    info.processName = event.processName;
    info.abilityName = event.abilityName;
    info.pageUrl = event.pageUrl;
    info.sceneId = event.sceneId;
    info.bundleNameEx = event.bundleNameEx;
    info.isFocus = event.isFocus;
    info.startTime = event.startTime;
    info.duration = event.duration;
    info.totalAppFrames = event.totalAppFrames;
    info.totalAppMissedFrames = event.totalAppMissedFrames;
    info.maxAppFrameTime = event.maxAppFrameTime;
    info.maxAppSeqMissedFrames = event.maxAppSeqMissedFrames;
    info.isDisplayAnimator = event.isDisplayAnimator;
    info.totalRenderFrames = event.totalRenderFrames;
    info.totalRenderMissedFrames = event.totalRenderMissedFrames;
    info.maxRenderFrameTime = event.maxRenderFrameTime;
    info.averageRenderFrameTime = event.averageRenderFrameTime;
    info.maxRenderSeqMissedFrames = event.maxRenderSeqMissedFrames;
    info.isFoldDisp = event.isFoldDisp;
    info.traceFileName = event.traceFileName;
    info.infoFileName = event.infoFileName;
    info.happenTime = event.happenTime;
    return info;
}