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

#include "AppLaunchSceneDataProcessor.h"
#include "AppLaunchConverter.h"

namespace OHOS {
namespace HiviewDFX {
static const std::string EVENT_INTERACTION_RESPONSE_LATENCY = "INTERACTION_RESPONSE_LATENCY";
static const std::string EVENT_START_ABILITY = "START_ABILITY";
static const std::string EVENT_APP_STARTUP_TYPE = "APP_STARTUP_TYPE";
static const std::string EVENT_PROCESS_START = "PROCESS_START";
static const std::string EVENT_APP_ATTACH = "APP_ATTACH";
static const std::string EVENT_APP_FOREGROUND = "APP_FOREGROUND";
static const std::string EVENT_ABILITY_ONFOREGROUND = "ABILITY_ONFOREGROUND";
static const std::string EVENT_START_WINDOW = "START_WINDOW";
static const std::string EVENT_DRAWN_COMPLETED = "DRAWN_COMPLETED";
static const std::string EVENT_FIRST_FRAME_DRAWN = "FIRST_FRAME_DRAWN";
static const std::string EVENT_INTERACTION_COMPLETED_LATENCY = "INTERACTION_COMPLETED_LATENCY";

static const std::string SCENE_LAUNCHER_APP_LAUNCH_FROM_ICON = "LAUNCHER_APP_LAUNCH_FROM_ICON";
static const std::string SCENE_LAUNCHER_APP_LAUNCH_FROM_NOTIFICATIONBAR = "LAUNCHER_APP_LAUNCH_FROM_NOTIFICATIONBAR";
static const std::string SCENE_LAUNCHER_APP_LAUNCH_FROM_DOCK = "LAUNCHER_APP_LAUNCH_FROM_DOCK";
static const std::string SCENE_LAUNCHER_APP_LAUNCH_FROM_APPCENTER = "LAUNCHER_APP_LAUNCH_FROM_APPCENTER";
static const std::string SCENE_LAUNCHER_APP_LAUNCH_FROM_RECENT = "LAUNCHER_APP_LAUNCH_FROM_RECENT";

AppLaunchSceneDataProcessor::AppLaunchSceneDataProcessor(IAppLaunchSceneDb* db, ITimeoutExecutor* exec,
                                                         IAppLaunchSceneDataProcessor::MetricReporter* metricReporter,
                                                         IAppTimer* timer)
{
    this->db = db;
    this->exec = exec;
    this->metricReporter = metricReporter;
    this->timer = timer;
}

AppLaunchSceneDataProcessor::AppLaunchSceneDataProcessor(IAppLaunchSceneDb* db, ITimeoutExecutor* exec,
                                                         IAppTimer* timer)
{
    this->db = db;
    this->exec = exec;
    this->timer = timer;
}

std::string AppLaunchSceneDataProcessor::GetBundleName(const AppStartCheckPointData& data)
{
    std::string bundleName = data.bundleName;
    if (data.eventName == EVENT_FIRST_FRAME_DRAWN) {
        if (pidBundleMap.find(data.appPid) != pidBundleMap.end()) {
            bundleName = pidBundleMap[data.appPid];
        }
    } else if ((data.eventName == EVENT_INTERACTION_RESPONSE_LATENCY) ||
               (data.eventName == EVENT_INTERACTION_COMPLETED_LATENCY)) {
        bundleName = data.note;
    }
    return bundleName;
}

void AppLaunchSceneDataProcessor::ProcessSceneData(const AppStartCheckPointData& data)
{
    if (!CheckValidCheckPoint(data)) {
        return;
    }
    ValidateDuplication(data);
    std::string bundleName = GetBundleName(data);
    if (bundleName.empty()) {
        return;
    }
    AppStartRecord record = GetRecord(bundleName);
    if (IsStartPoint(data)) {
        CheckOutExistStartPoint(data);
        CreateRecord(bundleName, data);
        StartTimer(bundleName);
        return;
    }
    // Filter out non-user-triggered application startup. This event does not have the start ability.
    if (record.bundleName.empty()) {
        return;
    }
    SaveCheckPoint(record, data);
    if (AllPointsReceived(bundleName)) {
        StopTimer(bundleName);
        AppStartMetrics metrics = CalcMetrics(bundleName);
        Report(metrics);
        DeleteRecord(bundleName);
        ClearMapByBundleName(bundleName);
    }
}

void AppLaunchSceneDataProcessor::Expired(std::string bundleName)
{
    exec->ExecuteTimeoutInMainThr(this, bundleName);
}

void AppLaunchSceneDataProcessor::HandleTimeoutInMainThr(std::string name)
{
    if (ReportConditionMet(name)) {
        AppStartMetrics metrics = CalcMetrics(name);
        Report(metrics);
    }
    DeleteRecord(name);
    ClearMapByBundleName(name);
}

bool AppLaunchSceneDataProcessor::CheckValidCheckPoint(const AppStartCheckPointData& data)
{
    if (data.domain.empty() || data.eventName.empty()) {
        return false;
    }
    bool checkBundle;
    if (data.eventName == EVENT_FIRST_FRAME_DRAWN) {
        checkBundle = (data.appPid > 0);
    } else {
        checkBundle = !data.bundleName.empty();
    }
    bool checkSceneId;
    if (!data.sceneId.empty()) {
        checkSceneId = ((data.sceneId == SCENE_LAUNCHER_APP_LAUNCH_FROM_ICON)
            || (data.sceneId == SCENE_LAUNCHER_APP_LAUNCH_FROM_NOTIFICATIONBAR)
            || (data.sceneId == SCENE_LAUNCHER_APP_LAUNCH_FROM_DOCK)
            || (data.sceneId == SCENE_LAUNCHER_APP_LAUNCH_FROM_RECENT)
            || (data.sceneId == SCENE_LAUNCHER_APP_LAUNCH_FROM_APPCENTER));
    } else {
        checkSceneId = true;
    }

    return (checkBundle && checkSceneId);
}

void AppLaunchSceneDataProcessor::ValidateDuplication(const AppStartCheckPointData& data)
{
    // if duplicate, throw a logic error
}

void AppLaunchSceneDataProcessor::SaveCheckPoint(const std::string& bundleName, const AppStartCheckPointData& data)
{
    AppStartRecord record = GetRecord(bundleName);
    if (record.bundleName.empty()) {
        return;
    }
    SaveCheckPoint(record, data);
}

void AppLaunchSceneDataProcessor::SaveCheckPoint(AppStartRecord& record, const AppStartCheckPointData& data)
{
    if (data.eventName == EVENT_INTERACTION_RESPONSE_LATENCY) {
        InteractionResponse interactionResponse = AppLaunchConverter::ConvertToInteractionResponse(data);
        record.interactionResponse = interactionResponse;
    } else if (data.eventName == EVENT_START_ABILITY) {
        StartAbility startAbility = AppLaunchConverter::ConvertToStartAbility(data);
        record.startAbility = startAbility;
    } else if (data.eventName == EVENT_APP_STARTUP_TYPE) {
        if (!record.appStartupType.bundleName.empty()) {
            return;
        }
        record.appStartupType = AppLaunchConverter::ConvertToAppStartupType(data);
        SaveAppIdIntoMap(record.appStartupType.appPid, record.appStartupType.bundleName);
        record.appPid = record.appStartupType.appPid;
    } else if (data.eventName == EVENT_PROCESS_START) {
        ProcessStart processStart = AppLaunchConverter::ConvertToProcessStart(data);
        record.processStart = processStart;
    } else if (data.eventName == EVENT_APP_ATTACH) {
        record.appAttach = AppLaunchConverter::ConvertToAppAttach(data);
        SaveAppIdIntoMap(record.appAttach.appPid, record.appAttach.bundleName);
        record.appPid = record.appAttach.appPid;
    } else if (data.eventName == EVENT_APP_FOREGROUND) {
        record.appForeground = AppLaunchConverter::ConvertToAppForeground(data);
        SaveAppIdIntoMap(record.appForeground.appPid, record.appForeground.bundleName);
        record.appPid = record.appForeground.appPid;
    } else if (data.eventName == EVENT_ABILITY_ONFOREGROUND) {
        AbilityForeground abilityForeground = AppLaunchConverter::ConvertToAbilityForeground(data);
        record.abilityForeground = abilityForeground;
    } else if (data.eventName == EVENT_START_WINDOW) {
        record.startWindow = AppLaunchConverter::ConvertToStartWindow(data);
        SaveAppIdIntoMap(record.startWindow.appPid, record.startWindow.bundleName);
        record.appPid = record.startWindow.appPid;
    } else if (data.eventName == EVENT_DRAWN_COMPLETED) {
        record.drawnCompleted = AppLaunchConverter::ConvertToDrawnCompleted(data);
        SaveAppIdIntoMap(record.drawnCompleted.appPid, record.drawnCompleted.bundleName);
        record.appPid = record.drawnCompleted.appPid;
    } else if (data.eventName == EVENT_FIRST_FRAME_DRAWN) {
        FirstFrameDrawn firstFrameDrawn = AppLaunchConverter::ConvertToFirstFrameDrawn(data);
        record.firstFrameDrawn = firstFrameDrawn;
    } else if (data.eventName == EVENT_INTERACTION_COMPLETED_LATENCY) {
        InteractionCompleted interactionCompleted = AppLaunchConverter::ConvertToInteractionCompleted(data);
        record.interactionCompleted = interactionCompleted;
    } else {
        return;
    }
    db->UpdateRecord(record);
}

bool AppLaunchSceneDataProcessor::IsStartPoint(const AppStartCheckPointData& data)
{
    bool isStartAbilityPoint = ((data.domain == "AAFWK") && (data.eventName == EVENT_START_ABILITY));
    return isStartAbilityPoint;
}

bool AppLaunchSceneDataProcessor::HaveStartPoint(const AppStartRecord& record)
{
    return (!record.interactionResponse.bundleName.empty() || !record.startAbility.bundleName.empty());
}

bool AppLaunchSceneDataProcessor::HaveAllEndPoints(const AppStartRecord& record)
{
    return (record.firstFrameDrawn.appPid > 0) && (!record.interactionCompleted.bundleName.empty());
}

bool AppLaunchSceneDataProcessor::HaveEndPoint(const AppStartRecord& record)
{
    return (record.firstFrameDrawn.appPid > 0) || (!record.interactionCompleted.bundleName.empty());
}

bool AppLaunchSceneDataProcessor::HaveResponseOrCompletedPoint(const AppStartRecord& record)
{
    return (!record.interactionResponse.bundleName.empty() || !record.interactionCompleted.bundleName.empty());
}

bool AppLaunchSceneDataProcessor::AllPointsReceived(const std::string& bundleName)
{
    AppStartRecord record = GetRecord(bundleName);
    bool hasTypePoint = !record.appStartupType.bundleName.empty();
    bool hasDrawnCompletedPoint = !record.drawnCompleted.bundleName.empty();
    return (HaveStartPoint(record) && hasTypePoint && hasDrawnCompletedPoint && HaveAllEndPoints(record));
}


std::string AppLaunchSceneDataProcessor::GetProcessName(const AppStartRecord& record)
{
    if (!record.appAttach.processName.empty()) {
        return record.appAttach.processName;
    }
    if (!record.appForeground.processName.empty()) {
        return record.appForeground.processName;
    }
    if (!record.startWindow.processName.empty()) {
        return record.startWindow.processName;
    }
    return "";
}

int32_t AppLaunchSceneDataProcessor::GetAppPid(const AppStartRecord& record)
{
    if (record.drawnCompleted.appPid > 0) {
        return record.drawnCompleted.appPid;
    }
    if (record.firstFrameDrawn.appPid > 0) {
        return record.firstFrameDrawn.appPid;
    }
    return 0;
}

uint64_t AppLaunchSceneDataProcessor::GetInputTime(const AppStartRecord& record)
{
    if (!record.interactionCompleted.bundleName.empty()) {
        return record.interactionCompleted.inputTime;
    } else {
        return record.interactionResponse.inputTime;
    }
}

AppStartMetrics AppLaunchSceneDataProcessor::CalcMetrics(const std::string& bundleName)
{
    AppStartRecord record = GetRecord(bundleName);
    AppStartMetrics appStartMetrics;
    appStartMetrics.appPid = GetAppPid(record);
    appStartMetrics.versionCode = record.appStartupType.versionCode;
    appStartMetrics.versionName = record.appStartupType.versionName;
    appStartMetrics.processName = GetProcessName(record);
    appStartMetrics.bundleName = record.appStartupType.bundleName;
    appStartMetrics.abilityName = record.appStartupType.abilityName;
    appStartMetrics.startType = record.appStartupType.startType;
    appStartMetrics.happenTime = record.startAbility.time;
    appStartMetrics.responseLatency = (!record.interactionResponse.note.empty()) ?
        record.interactionResponse.responseLatency : record.interactionCompleted.animationStartLatency;
    uint64_t inputTime = GetInputTime(record);
    if (!record.startAbility.bundleName.empty() && (inputTime > 0)) {
        appStartMetrics.launcherToAmsStartAbilityDur = (record.startAbility.time - inputTime);
    }
    if ((!record.startAbility.bundleName.empty()) && (!record.processStart.bundleName.empty())) {
        appStartMetrics.amsStartAbilityToProcessStartDuration = (record.processStart.time - record.startAbility.time);
    }
    if ((!record.processStart.bundleName.empty()) && (!record.appAttach.bundleName.empty())) {
        appStartMetrics.amsProcessStartToAppAttachDuration = (record.appAttach.time - record.processStart.time);
    }
    if ((!record.appAttach.bundleName.empty()) && (!record.appForeground.bundleName.empty())) {
        appStartMetrics.amsAppAttachToAppForegroundDuration = (record.appForeground.time - record.appAttach.time);
    }
    if ((!record.startAbility.bundleName.empty()) && (!record.appForeground.bundleName.empty())) {
        appStartMetrics.amsStartAbilityToAppForegroundDuration = (record.appForeground.time - record.startAbility.time);
    }
    if ((!record.appForeground.bundleName.empty()) && (!record.abilityForeground.bundleName.empty())) {
        appStartMetrics.amsAppFgToAbilityFgDur = (record.abilityForeground.time - record.appForeground.time);
    }
    if ((!record.abilityForeground.bundleName.empty()) && (!record.startWindow.bundleName.empty())) {
        appStartMetrics.amsAbilityFgToWmsStartWinDur = (record.abilityForeground.time - record.startWindow.time);
    }
    if (!record.drawnCompleted.bundleName.empty()  && (inputTime > 0)) {
        appStartMetrics.drawnLatency = (record.drawnCompleted.time - inputTime);
    }
    CalcLatency(appStartMetrics, record, inputTime);
    return appStartMetrics;
}

void AppLaunchSceneDataProcessor::CalcLatency(AppStartMetrics& appStartMetrics, const AppStartRecord& record,
                                              const uint64_t inputTime)
{
    uint64_t e2eLatency = 0;
    if ((record.firstFrameDrawn.appPid > 0)  && (inputTime > 0)) {
        appStartMetrics.firstFrameDrawnLatency = (record.firstFrameDrawn.time - inputTime);
        if (appStartMetrics.firstFrameDrawnLatency > e2eLatency) {
            e2eLatency = appStartMetrics.firstFrameDrawnLatency;
        }
    }
    if (!record.interactionCompleted.bundleName.empty()) {
        appStartMetrics.sceneId = record.interactionCompleted.sceneId;
        appStartMetrics.sourceType = record.interactionCompleted.sourceType;
        appStartMetrics.inputTime = record.interactionCompleted.inputTime;
        appStartMetrics.pageUrl = record.interactionCompleted.pageUrl;
        appStartMetrics.animationEndLatency = record.interactionCompleted.animationEndLatency;
        appStartMetrics.animationLatency = record.interactionCompleted.e2eLatency;
        uint64_t latency = (record.interactionCompleted.time - inputTime);
        if (latency > e2eLatency) {
            e2eLatency = latency;
        }
    } else if (!record.interactionResponse.bundleName.empty()) {
        appStartMetrics.sceneId = record.interactionResponse.sceneId;
        appStartMetrics.sourceType = record.interactionResponse.sourceType;
        appStartMetrics.inputTime = record.interactionResponse.inputTime;
        appStartMetrics.pageUrl = record.interactionResponse.pageUrl;
    }
    appStartMetrics.e2eLatency = e2eLatency;
}

void AppLaunchSceneDataProcessor::Report(const AppStartMetrics& metrics)
{
    metricReporter->ReportMetrics(metrics);
}

bool AppLaunchSceneDataProcessor::ReportConditionMet(const std::string& bundleName)
{
    AppStartRecord record = GetRecord(bundleName);
    bool hasTypePoint = !record.appStartupType.bundleName.empty();
    return (HaveStartPoint(record) && hasTypePoint && HaveEndPoint(record) && HaveResponseOrCompletedPoint(record));
}

AppStartRecord AppLaunchSceneDataProcessor::GetRecord(const std::string& bundleName)
{
    return db->QueryRecord(bundleName);
}

void AppLaunchSceneDataProcessor::DeleteRecord(const std::string& bundleName)
{
    db->DeleteRecord(bundleName);
}

void AppLaunchSceneDataProcessor::CreateRecord(const AppStartCheckPointData& data)
{
    CreateRecord(data.bundleName, data);
}

void AppLaunchSceneDataProcessor::CreateRecord(const std::string& bundleName, const AppStartCheckPointData& data)
{
    AppStartRecord record;
    record.bundleName = bundleName;
    if (data.eventName == EVENT_INTERACTION_RESPONSE_LATENCY) {
        InteractionResponse interactionResponse = AppLaunchConverter::ConvertToInteractionResponse(data);
        record.interactionResponse = interactionResponse;
    } else if (data.eventName == EVENT_START_ABILITY) {
        IAppLaunchSceneDb::StartAbility startAbility = AppLaunchConverter::ConvertToStartAbility(data);
        record.startAbility = startAbility;
    } else {
        return;
    }
    db->CreateRecord(record);
}

void AppLaunchSceneDataProcessor::StartTimer(std::string bundle)
{
    try {
        timer->Start(bundle);
    } catch (const std::invalid_argument& ex) {
        throw std::logic_error("AppLaunchSceneDataProcessor error " + std::string(ex.what()));
    }
}

void AppLaunchSceneDataProcessor::StopTimer(std::string bundle)
{
    try {
        timer->Stop(bundle);
    } catch (const std::invalid_argument& ex) {
        throw std::logic_error("AppLaunchSceneDataProcessor error " + std::string(ex.what()));
    }
}

void AppLaunchSceneDataProcessor::SetMetricReporter(MetricReporter* metricReporter)
{
    this->metricReporter = metricReporter;
}

void AppLaunchSceneDataProcessor::CheckOutExistStartPoint(const AppStartCheckPointData& data)
{
    AppStartRecord record = GetRecord(data.bundleName);
    if (!record.interactionResponse.bundleName.empty()) {
        // stop old timer
        StopTimer(data.bundleName);
        if (ReportConditionMet(data.bundleName)) {
            AppStartMetrics metrics = CalcMetrics(data.bundleName);
            Report(metrics);
        }
        db->DeleteRecord(data.bundleName);
        ClearMapByBundleName(data.bundleName);
    }
}
} // HiviewDFX
} // OHOS