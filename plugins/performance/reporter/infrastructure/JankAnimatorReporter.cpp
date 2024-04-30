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

#include "JankAnimatorReporter.h"
#include "hisysevent.h"
#include "hiview_logger.h"
DEFINE_LOG_TAG("XPerformance");

static constexpr char EXEC_DOMAIN[] = "PERFORMANCE";

using OHOS::HiviewDFX::HiSysEvent;


void JankAnimatorReporter::ReportNormal(const JankAnimatorReportData& data)
{
    HIVIEW_LOGD("JankAnimatorReporter ReportNormal Start");
    HiSysEventWrite(EXEC_DOMAIN, "INTERACTION_JANK", HiSysEvent::EventType::BEHAVIOR,
                    "APP_PID", data.appPid,
                    "VERSION_CODE", data.versionCode,
                    "VERSION_NAME", data.versionName,
                    "BUNDLE_NAME", data.bundleName,
                    "PROCESS_NAME", data.processName,
                    "ABILITY_NAME", data.abilityName,
                    "PAGE_URL", data.pageUrl,
                    "SCENE_ID", data.sceneId,
                    "STARTTIME", data.startTime,
                    "DURATION", data.durition,
                    "TOTAL_APP_FRAMES", data.totalAppFrames,
                    "TOTAL_APP_MISSED_FRAMES", data.totalAppMissedFrames,
                    "MAX_APP_FRAMETIME", data.maxAppFrameTime,
                    "MAX_APP_SEQ_MISSED_FRAMES", data.maxAppSeqMissedFrames,
                    "TOTAL_RENDER_FRAMES", data.totalRenderFrames,
                    "TOTAL_RENDER_MISSED_FRAMES", data.totalRenderMissedFrames,
                    "MAX_RENDER_FRAMETIME", data.maxRenderFrameTime,
                    "AVERAGE_RENDER_FRAMETIME", data.averageRenderFrameTime,
                    "MAX_RENDER_SEQ_MISSED_FRAMES", data.maxRenderSeqMissedFrames,
                    "IS_FOLD_DISP", data.isFoldDisp,
                    "BUNDLE_NAME_EX", data.bundleNameEx,
                    "IS_FOCUS", data.isFocus,
                    "DISPLAY_ANIMATOR", data.isDisplayAnimator);
}

void JankAnimatorReporter::ReportCritical(const JankAnimatorReportData& data)
{
    HIVIEW_LOGD("JankAnimatorReporter ReportCritical Start");
    HiSysEventWrite(EXEC_DOMAIN, "INTERACTION_JANK_FAULT", HiSysEvent::EventType::FAULT,
                    "APP_PID", data.appPid,
                    "VERSION_CODE", data.versionCode,
                    "VERSION_NAME", data.versionName,
                    "BUNDLE_NAME", data.bundleName,
                    "PROCESS_NAME", data.processName,
                    "ABILITY_NAME", data.abilityName,
                    "PAGE_URL", data.pageUrl,
                    "SCENE_ID", data.sceneId,
                    "STARTTIME", data.startTime,
                    "DURATION", data.durition,
                    "TOTAL_APP_FRAMES", data.totalAppFrames,
                    "TOTAL_APP_MISSED_FRAMES", data.totalAppMissedFrames,
                    "MAX_APP_FRAMETIME", data.maxAppFrameTime,
                    "MAX_APP_SEQ_MISSED_FRAMES", data.maxAppSeqMissedFrames,
                    "TOTAL_RENDER_FRAMES", data.totalRenderFrames,
                    "TOTAL_RENDER_MISSED_FRAMES", data.totalRenderMissedFrames,
                    "MAX_RENDER_FRAMETIME", data.maxRenderFrameTime,
                    "AVERAGE_RENDER_FRAMETIME", data.averageRenderFrameTime,
                    "MAX_RENDER_SEQ_MISSED_FRAMES", data.maxRenderSeqMissedFrames,
                    "IS_FOLD_DISP", data.isFoldDisp,
                    "TRACE_NAME",  data.traceFileName,
                    "INFO_FILE",  data.infoFileName,
                    "BUNDLE_NAME_EX", data.bundleNameEx,
                    "IS_FOCUS", data.isFocus,
                    "DISPLAY_ANIMATOR", data.isDisplayAnimator,
                    "HAPPEN_TIME", data.happenTime);
}