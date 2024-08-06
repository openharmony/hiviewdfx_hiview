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
#include "AppStartReporter.h"
#include "hisysevent.h"
#include "hiview_logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_LABEL(0xD002D66, "Hiview-XPerformance");

static constexpr char EXEC_DOMAIN[] = "PERFORMANCE";
using OHOS::HiviewDFX::HiSysEvent;

void AppStartReporter::ReportNormal(const AppStartReportData& data)
{
    HIVIEW_LOGD("AppStartReporter::ReportNormal");
    HiSysEventWrite(EXEC_DOMAIN, "APP_START", HiSysEvent::EventType::BEHAVIOR,
                    "APP_PID", data.appPid,
                    "VERSION_CODE", data.versionCode,
                    "VERSION_NAME", data.versionName,
                    "PROCESS_NAME", data.processName,
                    "BUNDLE_NAME", data.bundleName,
                    "ABILITY_NAME", data.abilityName,
                    "PAGE_URL", data.pageUrl,
                    "SCENE_ID", data.sceneId,
                    "START_TYPE", data.startType,
                    "SOURCE_TYPE", data.sourceType,
                    "INPUT_TIME", data.inputTime,
                    "RESPONSE_LATENCY", data.responseLatency,
                    "LAUN_TO_START_ABILITY_DUR", data.launcherToAmsStartAbilityDur,
                    "STARTABILITY_PROCESSSTART_DUR", data.amsStartAbilityToProcessStartDuration,
                    "PROCESSSTART_TO_APPATTACH_DUR", data.amsProcessStartToAppAttachDuration,
                    "APPATTACH_TO_APPFOREGROUND_DUR", data.amsAppAttachToAppForegroundDuration,
                    "STARTABILITY_APPFOREGROUND_DUR", data.amsStartAbilityToAppForegroundDuration,
                    "APPFOREGR_ABILITYONFOREGR_DUR", data.amsAppFgToAbilityFgDur,
                    "ABILITYONFOREG_STARTWINDOW_DUR", data.amsAbilityFgToWmsStartWinDur,
                    "DRAWN_LATENCY", data.drawnLatency,
                    "FIRST_FRAEM_DRAWN_LATENCY", data.firstFrameDrawnLatency,
                    "ANIMATION_LATENCY", data.animationLatency,
                    "E2E_LATENCY", data.e2eLatency);
}

void AppStartReporter::ReportFault(const AppStartReportData& data)
{
    HIVIEW_LOGD("AppStartReporter::ReportFault");
    HiSysEventWrite(EXEC_DOMAIN, "APP_START_SLOW", HiSysEvent::EventType::FAULT,
                    "APP_PID", data.appPid,
                    "VERSION_CODE", data.versionCode,
                    "VERSION_NAME", data.versionName,
                    "PROCESS_NAME", data.processName,
                    "BUNDLE_NAME", data.bundleName,
                    "ABILITY_NAME", data.abilityName,
                    "PAGE_URL", data.pageUrl,
                    "SCENE_ID", data.sceneId,
                    "START_TYPE", data.startType,
                    "SOURCE_TYPE", data.sourceType,
                    "INPUT_TIME", data.inputTime,
                    "HAPPEN_TIME", data.happenTime,
                    "RESPONSE_LATENCY", data.responseLatency,
                    "LAUN_TO_START_ABILITY_DUR", data.launcherToAmsStartAbilityDur,
                    "STARTABILITY_PROCESSSTART_DUR", data.amsStartAbilityToProcessStartDuration,
                    "PROCESSSTART_TO_APPATTACH_DUR", data.amsProcessStartToAppAttachDuration,
                    "APPATTACH_TO_APPFOREGROUND_DUR", data.amsAppAttachToAppForegroundDuration,
                    "STARTABILITY_APPFOREGROUND_DUR", data.amsStartAbilityToAppForegroundDuration,
                    "APPFOREGR_ABILITYONFOREGR_DUR", data.amsAppFgToAbilityFgDur,
                    "ABILITYONFOREG_STARTWINDOW_DUR", data.amsAbilityFgToWmsStartWinDur,
                    "DRAWN_LATENCY", data.drawnLatency,
                    "FIRST_FRAEM_DRAWN_LATENCY", data.firstFrameDrawnLatency,
                    "ANIMATION_LATENCY", data.animationLatency,
                    "E2E_LATENCY", data.e2eLatency,
                    "TRACE_NAME", data.traceFileName,
                    "INFO_FILE", data.infoFileName);
}
} // HiviewDFX
} // OHOS