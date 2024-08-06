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
#ifndef JANK_ANIMATOR_MONITOR_CONVERTER_H
#define JANK_ANIMATOR_MONITOR_CONVERTER_H

#include "IJankAnimatorReporter.h"
#include "IAnimatorSceneDataProcessor.h"

namespace OHOS {
namespace HiviewDFX {
using AnimatorMetrics = IAnimatorSceneDataProcessor::AnimatorMetrics;

class JankAnimatorMonitorConverter {
public:
    static JankAnimatorReportEvent ConverterReportData(AnimatorMetrics metricData)
    {
        return ConverterReportData(metricData, "", "");
    }

    static JankAnimatorReportEvent ConverterReportData(AnimatorMetrics metrics,
                                                       std::string traceFileName, std::string infoFileName)
    {
        JankAnimatorReportEvent event;
        event.appPid = metrics.appInfo.appPid;
        event.versionCode = metrics.basicInfo.versionCode;
        event.versionName = metrics.basicInfo.versionName;
        event.bundleName = metrics.basicInfo.moduleName;
        event.processName = metrics.basicInfo.processName;
        event.abilityName = metrics.basicInfo.abilityName;
        event.pageUrl = metrics.basicInfo.pageUrl;
        event.sceneId = metrics.basicInfo.sceneId;
        event.bundleNameEx = metrics.basicInfo.bundleNameEx;
        event.isFocus = metrics.focus;
        event.startTime = metrics.appInfo.startTime;
        event.durition = metrics.appInfo.durition;
        event.totalAppFrames = metrics.appInfo.totalFrames;
        event.totalAppMissedFrames = metrics.appInfo.totalMissedFrames;
        event.maxAppFrameTime = metrics.appInfo.maxFrameTime;
        event.maxAppSeqMissedFrames = metrics.appInfo.maxSeqMissedFrames;
        event.isDisplayAnimator = metrics.appInfo.isDisplayAnimator;
        event.totalRenderFrames = metrics.rsInfo.totalFrames;
        event.totalRenderMissedFrames = metrics.rsInfo.totalMissedFrames;
        event.maxRenderFrameTime = metrics.rsInfo.maxFrameTime;
        event.averageRenderFrameTime = metrics.rsInfo.averageFrameTime;
        event.maxRenderSeqMissedFrames = metrics.rsInfo.maxSeqMissedFrames;
        event.isFoldDisp = metrics.rsInfo.isFoldDisp;
        event.happenTime = (metrics.appInfo.startTime == 0) ? metrics.rsInfo.happenTime : metrics.appInfo.startTime;
        event.traceFileName = traceFileName;
        event.infoFileName = infoFileName;
        return event;
    }
};
} // HiviewDFX
} // OHOS
#endif