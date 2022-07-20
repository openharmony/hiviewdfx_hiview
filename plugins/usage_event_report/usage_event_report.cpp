/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include "usage_event_report.h"

#include "hiview_event_report.h"
#include "hiview_event_cacher.h"
#include "hiview_shutdown_callback.h"
#include "logger.h"
#include "plugin_factory.h"
#include "power_mgr_client.h"
#include "time_util.h"
#include "usage_event_cacher.h"
#include "usage_event_common.h"

namespace OHOS {
namespace HiviewDFX {
REGISTER(UsageEventReport);
DEFINE_LOG_TAG("HiView-UsageEventReport");
uint64_t UsageEventReport::lastReportTime_ = 0;
uint64_t UsageEventReport::nextReportTime_ = 0;
std::string UsageEventReport::workPath_ = "";
namespace {
constexpr int TRIGGER_CYCLE = 5 * 60; // 5 min
const std::string SERVICE_NAME = "usage_report";
}

UsageEventReport::UsageEventReport() : callback_(nullptr)
{}

void UsageEventReport::OnLoad()
{
    HIVIEW_LOGI("start to init the env");
    Init();
    Start();
}

void UsageEventReport::OnUnload()
{
    HIVIEW_LOGI("start to clean up the env");
    if (workLoop_ != nullptr) {
        workLoop_->StopLoop();
        workLoop_.reset();
    }
    if (callback_ != nullptr) {
        PowerMgr::PowerMgrClient::GetInstance().UnRegisterShutdownCallback(callback_);
        callback_ = nullptr;
    }
}

void UsageEventReport::Init()
{
    auto context = GetHiviewContext();
    if (context != nullptr) {
        workPath_ = context->GetHiViewDirectory(HiviewContext::DirectoryType::WORK_DIRECTORY);

        // get plugin stats event from db if any
        UsageEventCacher cacher(workPath_);
        std::vector<std::shared_ptr<LoggerEvent>> pluginStatEvents;
        cacher.GetPluginStatsEvents(pluginStatEvents);
        HiviewEventCacher::GetInstance().AddPluginStatsEvent(pluginStatEvents);
        HIVIEW_LOGI("get plugin stats event=%{public}d", pluginStatEvents.size());

        // get last report time from db if any
        auto sysUsageEvent = cacher.GetSysUsageEvent();
        if (sysUsageEvent != nullptr) {
            HIVIEW_LOGI("get sys usage event=%{public}s", sysUsageEvent->ToJsonString().c_str());
            lastReportTime_ = sysUsageEvent->GetValue(SysUsageEventSpace::KEY_OF_START).GetUint64();
        } else {
            lastReportTime_ = TimeUtil::GetMilliseconds();
        }
    }
    nextReportTime_ = static_cast<uint64_t>(TimeUtil::Get0ClockStampMs()) + TimeUtil::MILLISECS_PER_DAY;

    // more than one day since the last report
    if (TimeUtil::GetMilliseconds() >= (lastReportTime_ + TimeUtil::MILLISECS_PER_DAY)) {
        HIVIEW_LOGI("lastReportTime=%{public}" PRIu64 ", need to report event now", lastReportTime_);
        ReportEvent();
    }
}

void UsageEventReport::InitCallback()
{
    HIVIEW_LOGI("start to init shutdown callback");
    callback_ = new (std::nothrow) HiViewShutdownCallback();
    PowerMgr::PowerMgrClient::GetInstance().RegisterShutdownCallback(callback_,
        PowerMgr::IShutdownCallback::ShutdownPriority::POWER_SHUTDOWN_PRIORITY_HIGH);
}

void UsageEventReport::Start()
{
    auto task = bind(&UsageEventReport::TimeOut, this);
    workLoop_->AddTimerEvent(nullptr, nullptr, task, TRIGGER_CYCLE, true);
}

void UsageEventReport::TimeOut()
{
    HIVIEW_LOGI("start checking whether events need to be reported");
    if (TimeUtil::GetMilliseconds() >= nextReportTime_) {
        HIVIEW_LOGI("start to report event");
        ReportEvent();
        nextReportTime_ += TimeUtil::MILLISECS_PER_DAY;
    }

    // init shutdown callback if necessary
    if (callback_ == nullptr) {
        InitCallback();
    }
}

void UsageEventReport::ReportEvent()
{
    // report plugin stats event
    HiviewEventReport::ReportPluginStats();

    // report sys usage event and app usage event
    StartServiceByOption("-r");

    // delete the cache event in db after report
    DeleteCacheEvent();

    // update the start time of the next report
    lastReportTime_ = TimeUtil::GetMilliseconds();
}

void UsageEventReport::DeleteCacheEvent()
{
    UsageEventCacher cacher(workPath_);
    cacher.DeletePluginStatsEventsFromDb();
    cacher.DeleteSysUsageEventFromDb();
}

void UsageEventReport::SaveEventToDb()
{
    HIVIEW_LOGI("start to save the event to db");
    SavePluginStatsEvents();
    SaveSysUsageEvent();
}

void UsageEventReport::SavePluginStatsEvents()
{
    std::vector<std::shared_ptr<LoggerEvent>> events;
    HiviewEventCacher::GetInstance().GetPluginStatsEvents(events);
    if (events.empty()) {
        return;
    }
    UsageEventCacher cacher(workPath_);
    cacher.SavePluginStatsEventsToDb(events);
}

void UsageEventReport::SaveSysUsageEvent()
{
    StartServiceByOption("-s");
}

void UsageEventReport::StartServiceByOption(const std::string& opt)
{
    std::stringstream ss;
    ss << SERVICE_NAME << " -p " << workPath_ << " -t " << lastReportTime_ << " " << opt;
    HIVIEW_LOGI("start service cmd=%{public}s", ss.str().c_str());
    if (system(ss.str().c_str()) < 0) {
        HIVIEW_LOGE("failed to start the service=%{public}s", SERVICE_NAME.c_str());
    }
}
}  // namespace HiviewDFX
}  // namespace OHOS
