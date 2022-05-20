/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "hiview_timer_info.h"

#include "event_cacher.h"
#include "hilog/log.h"
#include "sys_event_common.h"
#include "sys_event_logger.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, LABEL_DOMAIN, "HiViewTimerInfo" };
}
HiViewTimerInfo::HiViewTimerInfo() : lastReportTime_(0), nextReportTime_(0)
{
    Init();
}

void HiViewTimerInfo::SetType(const int& type)
{
    this->type = type;
}

void HiViewTimerInfo::SetRepeat(bool repeat)
{
    this->repeat = repeat;
}
void HiViewTimerInfo::SetInterval(const uint64_t& interval)
{
    this->interval = interval;
}
void HiViewTimerInfo::SetWantAgent(std::shared_ptr<OHOS::AbilityRuntime::WantAgent::WantAgent> wantAgent)
{
    this->wantAgent = wantAgent;
}

void HiViewTimerInfo::OnTrigger()
{
    HiLog::Debug(LABEL, "hiview timer OnTrigger start");
    if (CheckIfNeedReport()) {
        HiLog::Info(LABEL, "nextReportTime=%{public}" PRIu64 ", start to report event", nextReportTime_);
        Report();
        UpdateNextReportTime();
    } else {
        SaveCacheTimely();
    }
}

void HiViewTimerInfo::Init()
{
    auto sysUsageEvent = EventCacher::GetInstance().GetSysUsageEvent();
    lastReportTime_ = sysUsageEvent->GetValue(SysUsageEventSpace::KEY_OF_START).GetUint64();
    if (CheckLastReportTime()) {
        HiLog::Info(LABEL, "lastReportTime=%{public}" PRIu64 ", start to report event", lastReportTime_);
        Report();
    }
    nextReportTime_ = static_cast<uint64_t>(TimeUtil::Get0ClockStampMs()) + TimeUtil::MILLISECS_PER_DAY;
}

void HiViewTimerInfo::UpdateNextReportTime()
{
    nextReportTime_ += TimeUtil::MILLISECS_PER_DAY;
}

bool HiViewTimerInfo::CheckLastReportTime()
{
    return TimeUtil::GetMilliseconds() >= (lastReportTime_ + TimeUtil::MILLISECS_PER_DAY);
}

bool HiViewTimerInfo::CheckIfNeedReport()
{
    return TimeUtil::GetMilliseconds() >= nextReportTime_;
}

void HiViewTimerInfo::Report()
{
    SysEventLogger::ReportPluginStats();
    SysEventLogger::ReportAppUsage();
    SysEventLogger::ReportSysUsage();
}

void HiViewTimerInfo::SaveCacheTimely()
{
    HiLog::Debug(LABEL, "save the cache to db timely");
    EventCacher::GetInstance().SavePluginStatsEventsToDb();

    // get the last usage use time from cache
    auto sysUsageEvent = EventCacher::GetInstance().GetSysUsageEvent();
    auto endTime = sysUsageEvent->GetValue(SysUsageEventSpace::KEY_OF_END).GetUint64();
    auto powerTime = sysUsageEvent->GetValue(SysUsageEventSpace::KEY_OF_POWER).GetUint64();
    auto runningTime = sysUsageEvent->GetValue(SysUsageEventSpace::KEY_OF_RUNNING).GetUint64();
    auto screenTime = sysUsageEvent->GetValue(SysUsageEventSpace::KEY_OF_SCREEN).GetUint64();

    // update cur usage time to db
    EventCacher::GetInstance().UpdateSysUsageEvent();
    EventCacher::GetInstance().SaveSysUsageEventToDb();

    // restore last usage time
    sysUsageEvent->Update(SysUsageEventSpace::KEY_OF_END, endTime);
    sysUsageEvent->Update(SysUsageEventSpace::KEY_OF_POWER, powerTime);
    sysUsageEvent->Update(SysUsageEventSpace::KEY_OF_RUNNING, runningTime);
    sysUsageEvent->Update(SysUsageEventSpace::KEY_OF_SCREEN, screenTime);
}
} // namespace HiviewDFX
} // namespace OHOS
