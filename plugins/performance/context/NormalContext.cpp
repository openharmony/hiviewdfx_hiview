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

#include "AnimatorSceneDataProcessor.h"
#include "JankAnimatorMonitor.h"
#include "AppLaunchSceneDataProcessor.h"
#include "AppLaunchMonitor.h"
#include "AppLaunchSceneDbAdapter.h"
#include "SceneTimerOhImpl.h"
#include "AppTimerAdapter.h"
#include "AppStartReporter.h"
#include "JankAnimatorReporter.h"
#include "NormalContext.h"
#include "AppStartReporterAdapter.h"
#include "JankAnimatorReporterAdapter.h"
#include "JlogId.h"
#include "ActionId.h"

namespace {
    enum TimerUser {
        APP_START,
    };
}

void NormalContext::CreateContext()
{
    /* init monitor */
    NormalContext::CommonParts common = MakeCommonParts();
    /* hold on IEventObservable */
    this->eventObservable = common.eventsPoster;

    InitAppStartMonitor(common);
    InitJankAnimatorMonitor(common);
}

NormalContext::CommonParts NormalContext::MakeCommonParts()
{
    ThrExecutor* thr = new ThrExecutor();
    ISceneTimerInfrastructure* sceneTimerInfrastructure = new SceneTimerOhImpl();
    EventsPoster* eventsPoster = new EventsPoster();
    return NormalContext::CommonParts(thr, sceneTimerInfrastructure, eventsPoster);
}

void NormalContext::InitAppStartMonitor(const NormalContext::CommonParts& common)
{
    IMonitor* appStartMonitor = MakeAppStartMonitor(common);
    RegisterMonitorByLogID(static_cast<int>(JLID_GRAPHIC_INTERACTION_RESPONSE_LATENCY), appStartMonitor);
    RegisterMonitorByLogID(static_cast<int>(JLID_START_ABILITY), appStartMonitor);
    RegisterMonitorByLogID(static_cast<int>(JLID_AAFWK_APP_STARTUP_TYPE), appStartMonitor);
    RegisterMonitorByLogID(static_cast<int>(JLID_AAFWK_PROCESS_START), appStartMonitor);
    RegisterMonitorByLogID(static_cast<int>(JLID_APP_ATTACH), appStartMonitor);
    RegisterMonitorByLogID(static_cast<int>(JLID_APP_FOREGROUND), appStartMonitor);
    RegisterMonitorByLogID(static_cast<int>(JLID_ABILITY_ONFOREGROUND), appStartMonitor);
    RegisterMonitorByLogID(static_cast<int>(JLID_WINDOWMANAGER_START_WINDOW), appStartMonitor);
    RegisterMonitorByLogID(static_cast<int>(JLID_GRAPHIC_FIRST_FRAME_DRAWN), appStartMonitor);
    RegisterMonitorByLogID(static_cast<int>(JLID_AAFWK_DRAWN_COMPLETED), appStartMonitor);
    RegisterMonitorByLogID(static_cast<int>(JLID_GRAPHIC_INTERACTION_COMPLETED_LATENCY), appStartMonitor);
    RegisterMonitorByLogID(static_cast<int>(JLID_ACE_INTERACTION_COMPLETED_LATENCY), appStartMonitor);
}

IMonitor* NormalContext::MakeAppStartMonitor(const NormalContext::CommonParts& common)
{
    IAppLaunchSceneDb* db = new AppLaunchSceneDbAdapter();
    ITimeoutExecutor* exec = common.thr;
    AppTimerAdapter* sceneTimer = new AppTimerAdapter(TimerUser::APP_START, common.timerInfra);
    AppLaunchSceneDataProcessor* processor = new AppLaunchSceneDataProcessor(db, exec, nullptr, sceneTimer);
    sceneTimer->SetCb(processor);
    IAppStartReportInfrastructure* infrastructure = new AppStartReporter();
    IAppStartReporter* reporter = new AppStartReporterAdapter(infrastructure, common.eventsPoster);
    AppLaunchMonitor* appStartMonitor = new AppLaunchMonitor(this, common.thr, reporter, processor);

    processor->SetMetricReporter(appStartMonitor);
    return appStartMonitor;
}

void NormalContext::InitJankAnimatorMonitor(const NormalContext::CommonParts& common)
{
    IMonitor* animatorMonitor = MakeJankAnimatorMonitor(common);
    RegisterMonitorByLogID(static_cast<int>(JLID_ACE_INTERACTION_APP_JANK), animatorMonitor);
    RegisterMonitorByLogID(static_cast<int>(JLID_GRAPHIC_INTERACTION_RENDER_JANK), animatorMonitor);
    RegisterMonitorByLogID(static_cast<int>(JLID_WINDOWMANAGER_FOCUS_WINDOW), animatorMonitor);
}

IMonitor* NormalContext::MakeJankAnimatorMonitor(const NormalContext::CommonParts& common)
{
    AnimatorSceneDataProcessor* dataProcessor = new AnimatorSceneDataProcessor();
    IJankAnimatorReportInfrastructure* reporterImpl = new JankAnimatorReporter();
    IJankAnimatorReporter* reporter = new JankAnimatorReporterAdapter(reporterImpl, common.eventsPoster);
    JankAnimatorMonitor* animatorMonitor = new JankAnimatorMonitor(common.thr, dataProcessor, reporter);
    dataProcessor->SetCb(animatorMonitor);
    return animatorMonitor;
}