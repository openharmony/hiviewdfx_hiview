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
#ifndef APP_LAUNCH_SCENE_DATA_PROCESSOR_H
#define APP_LAUNCH_SCENE_DATA_PROCESSOR_H

#include <string>
#include <map>
#include "IAppLaunchSceneDataProcessor.h"
#include "IAppLaunchSceneDb.h"
#include "SceneDataMapDbAdapter.h"
#include "ITimeoutExecutor.h"
#include "IAppTimer.h"

using AppStartRecord = IAppLaunchSceneDb::AppStartRecord;
using InteractionResponse = IAppLaunchSceneDb::InteractionResponse;
using StartAbility = IAppLaunchSceneDb::StartAbility;
using AppStartupType = IAppLaunchSceneDb::AppStartupType;
using ProcessStart = IAppLaunchSceneDb::ProcessStart;
using AppAttach = IAppLaunchSceneDb::AppAttach;
using AppForeground = IAppLaunchSceneDb::AppForeground;
using AbilityForeground = IAppLaunchSceneDb::AbilityForeground;
using StartWindow = IAppLaunchSceneDb::StartWindow;
using DrawnCompleted = IAppLaunchSceneDb::DrawnCompleted;
using FirstFrameDrawn = IAppLaunchSceneDb::FirstFrameDrawn;
using InteractionCompleted = IAppLaunchSceneDb::InteractionCompleted;
using AppStartMetrics = IAppLaunchSceneDataProcessor::AppStartMetrics;
using ITimeoutHandler = ITimeoutExecutor::ITimeoutHandler;

class AppLaunchSceneDataProcessor : public IAppLaunchSceneDataProcessor, public IAppTimer::ICb, public ITimeoutHandler,
        public SceneDataMapDbAdapter {
public:
    AppLaunchSceneDataProcessor(IAppLaunchSceneDb* db, ITimeoutExecutor* exec,
                               IAppLaunchSceneDataProcessor::MetricReporter* metricReporter,
                               IAppTimer* timer);
    AppLaunchSceneDataProcessor(IAppLaunchSceneDb* db, ITimeoutExecutor* exec, IAppTimer* timer);
    void ProcessSceneData(const AppStartCheckPointData& data) override;
    void SetMetricReporter(MetricReporter* metricReporter) override;
    void Expired(std::string key) override;
    void HandleTimeoutInMainThr(std::string name) override;
private:
    IAppLaunchSceneDataProcessor::MetricReporter* metricReporter;
    IAppLaunchSceneDb* db;
    ITimeoutExecutor* exec;
    IAppTimer* timer;

    bool CheckValidCheckPoint(const AppStartCheckPointData& data);
    void ValidateDuplication(const AppStartCheckPointData& data);
    void SaveCheckPoint(const std::string& bundleName, const AppStartCheckPointData& data);
    void SaveCheckPoint(AppStartRecord& record, const AppStartCheckPointData& data);
    bool IsStartPoint(const AppStartCheckPointData& data);
    bool AllPointsReceived(const std::string& bundleName);
    AppStartMetrics CalcMetrics(const std::string& bundleName);
    void Report(const AppStartMetrics& metrics);
    bool ReportConditionMet(const std::string& bundleName);
    void CreateRecord(const AppStartCheckPointData& data);
    void CreateRecord(const std::string& bundleName, const AppStartCheckPointData& data);
    AppStartRecord GetRecord(const std::string& bundleName);
    void DeleteRecord(const std::string& bundleName);
    bool RecordExist(const std::string& bundleName);
    void StartTimer(std::string bundle);
    void StopTimer(std::string bundle);
    bool HaveStartPoint(const AppStartRecord& record);
    bool HaveAllEndPoints(const AppStartRecord& record);
    bool HaveEndPoint(const AppStartRecord& record);
    bool HaveResponseOrCompletedPoint(const AppStartRecord& record);
    std::string GetProcessName(const AppStartRecord& record);
    int32_t GetAppPid(const AppStartRecord& record);
    uint64_t GetInputTime(const AppStartRecord& record);
    std::string GetBundleName(const AppStartCheckPointData& data);
    // The second application starts settlement points that already exist.
    void CheckOutExistStartPoint(const AppStartCheckPointData& data);
    void CalcLatency(AppStartMetrics& appStartMetrics, const AppStartRecord& record, const uint64_t inputTime);
};
#endif