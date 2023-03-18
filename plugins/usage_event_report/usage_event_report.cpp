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

#include <sys/wait.h>

#include "hiview_event_report.h"
#include "hiview_event_cacher.h"
#include "hiview_shutdown_callback.h"
#include "logger.h"
#include "plugin_factory.h"
#include "power_mgr_client.h"
#include "securec.h"
#include "string_util.h"
#include "time_util.h"
#include "usage_event_cacher.h"
#include "usage_event_common.h"

namespace OHOS {
namespace HiviewDFX {
REGISTER(UsageEventReport);
DEFINE_LOG_TAG("HiView-UsageEventReport");
uint64_t UsageEventReport::lastReportTime_ = 0;
uint64_t UsageEventReport::lastSysReportTime_ = 0;
uint64_t UsageEventReport::nextReportTime_ = 0;
std::string UsageEventReport::workPath_ = "";
namespace {
constexpr int TRIGGER_CYCLE = 5 * 60; // 5 min
constexpr uint32_t TRIGGER_ONE_HOUR = 12; // 1h = 5min * 12
}
using namespace SysUsageDbSpace;
using namespace SysUsageEventSpace;

UsageEventReport::UsageEventReport() : callback_(nullptr), timeOutCnt_(0)
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
    auto nowTime = TimeUtil::GetMilliseconds();
    if (auto context = GetHiviewContext(); context != nullptr) {
        workPath_ = context->GetHiViewDirectory(HiviewContext::DirectoryType::WORK_DIRECTORY);

        // get plugin stats event from db if any
        UsageEventCacher cacher(workPath_);
        std::vector<std::shared_ptr<LoggerEvent>> pluginStatEvents;
        cacher.GetPluginStatsEvents(pluginStatEvents);
        HiviewEventCacher::GetInstance().AddPluginStatsEvent(pluginStatEvents);
        HIVIEW_LOGI("get plugin stats event=%{public}d", pluginStatEvents.size());

        // get last report time from db if any
        if (auto event = cacher.GetSysUsageEvent(LAST_SYS_USAGE_COLL); event != nullptr) {
            HIVIEW_LOGI("get cache sys usage event=%{public}s", event->ToJsonString().c_str());
            lastReportTime_ = event->GetValue(KEY_OF_START).GetUint64();
        } else {
            lastReportTime_ = nowTime;
        }

        // get last sys report time from db if any
        if (auto event = cacher.GetSysUsageEvent(); event != nullptr) {
            HIVIEW_LOGI("get last sys usage event=%{public}s", event->ToJsonString().c_str());
            lastSysReportTime_ = event->GetValue(KEY_OF_START).GetUint64();
        } else {
            lastSysReportTime_ = nowTime;
        }
    }
    nextReportTime_ = static_cast<uint64_t>(TimeUtil::Get0ClockStampMs()) + TimeUtil::MILLISECS_PER_DAY;

    // more than one day since the last report
    if (nowTime >= (lastReportTime_ + TimeUtil::MILLISECS_PER_DAY)) {
        HIVIEW_LOGI("lastReportTime=%{public}" PRIu64 ", need to report daily event now", lastReportTime_);
        ReportDailyEvent();
    }

    // more than one hours since the shutdown time
    if (nowTime >= (lastSysReportTime_ + 3600000)) { // 3600000ms: 1h
        HIVIEW_LOGI("lastSysReportTime=%{public}" PRIu64 ", need to report sys usage event now", lastReportTime_);
        ReportSysUsageEvent();
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
    ReportTimeOutEvent();
    ReportDailyEvent();

    // init shutdown callback if necessary
    if (callback_ == nullptr) {
        InitCallback();
    }
}

void UsageEventReport::ReportDailyEvent()
{
    // check whether time step occurs. If yes, update the next report time
    auto nowTime = TimeUtil::GetMilliseconds();
    if (nowTime > (nextReportTime_ + TimeUtil::MILLISECS_PER_DAY)
        || nowTime < (nextReportTime_ - TimeUtil::MILLISECS_PER_DAY)) {
        HIVIEW_LOGW("start to update the next daily report time");
        nextReportTime_ = static_cast<uint64_t>(TimeUtil::Get0ClockStampMs()) + TimeUtil::MILLISECS_PER_DAY;
    } else if (nowTime >= nextReportTime_) {
        // report plugin stats event
        HIVIEW_LOGI("start to report daily event");
        HiviewEventReport::ReportPluginStats();
        DeletePluginStatsEvents();

        // report app usage event
        StartServiceByOption("-A");

        // update report time
        lastReportTime_ = TimeUtil::GetMilliseconds();
        nextReportTime_ += TimeUtil::MILLISECS_PER_DAY;
    } else {
        HIVIEW_LOGI("No need to report daily events");
    }
}

void UsageEventReport::ReportTimeOutEvent()
{
    ++timeOutCnt_;
    SaveSysUsageEvent();
    if (timeOutCnt_ >= TRIGGER_ONE_HOUR) {
        ReportSysUsageEvent();
        timeOutCnt_ = 0;
    }
}

void UsageEventReport::ReportSysUsageEvent()
{
    StartServiceByOption("-S");
    lastSysReportTime_ = TimeUtil::GetMilliseconds();
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

void UsageEventReport::DeletePluginStatsEvents()
{
    UsageEventCacher cacher(workPath_);
    cacher.DeletePluginStatsEventsFromDb();
}

void UsageEventReport::SaveSysUsageEvent()
{
    StartServiceByOption("-s");
}

void UsageEventReport::StartServiceByOption(const std::string& opt)
{
    HIVIEW_LOGI("start service, opt=%{public}s", opt.c_str());
    if (pid_t pid = fork(); pid < 0) {
        HIVIEW_LOGE("failed to fork child process");
        return;
    } else if (pid == 0) {
        const size_t len = 20; // 20: max_len(uint64_t) + '\0'
        char lastRTBuf[len] = {0};
        if (sprintf_s(lastRTBuf, len, "%" PRIu64, lastReportTime_) < 0) {
            HIVIEW_LOGE("failed to convert lastReportTime_=%{public}" PRIu64 " to string", lastReportTime_);
            _exit(-1);
        }
        char lastSRTBuf[len] = {0};
        if (sprintf_s(lastSRTBuf, len, "%" PRIu64, lastSysReportTime_) < 0) {
            HIVIEW_LOGE("failed to convert lastSysReportTime_=%{public}" PRIu64 " to string", lastSysReportTime_);
            _exit(-1);
        }
        const std::string serviceName = "usage_report";
        const std::string servicePath = "/system/bin/usage_report";
        if (execl(servicePath.c_str(), serviceName.c_str(),
            "-p", workPath_.c_str(),
            "-t", lastRTBuf,
            "-T", lastSRTBuf,
            opt.c_str(), nullptr) < 0) {
            HIVIEW_LOGE("failed to execute %{public}s", serviceName.c_str());
            _exit(-1);
        }
    } else {
        if (waitpid(pid, nullptr, 0) != pid) {
            HIVIEW_LOGE("failed to waitpid, pid=%{public}d, err=%{public}d", pid, errno);
        } else {
            HIVIEW_LOGI("succ to waitpid, pid=%{public}d", pid);
        }
    }
}
}  // namespace HiviewDFX
}  // namespace OHOS
