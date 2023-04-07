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
#include "platform_monitor.h"

#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "hiview_global.h"
#include "logger.h"
#include "pipeline.h"
#include "sys_event_dao.h"
#include "sys_event.h"
#include "time_util.h"
#include "monitor_config.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-Monitor");
namespace {
constexpr uint8_t SLEEP_TEN_SECONDS = 10;
constexpr char EVENT_SERVICE_PLUGIN[] = "SysEventService";
};
void PlatformMonitor::AccumulateTimeInterval(int64_t costTime, std::map<int8_t, uint32_t> &stat)
{
    std::lock_guard<std::mutex> lock(statMutex_);
    auto it = std::lower_bound(intervals_, intervals_ + sizeof(intervals_) / sizeof(intervals_[0]), costTime);
    int index = it - intervals_;
    stat[index] += 1;
}

void PlatformMonitor::CollectEvent(std::shared_ptr<PipelineEvent> event)
{
    if (event == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(topMutex_);
    topDomains_[event->domain_]++;
    topEvents_[event->eventName_]++;
}

void PlatformMonitor::CollectCostTime(PipelineEvent *event)
{
    // collect data after event destory
    if (event == nullptr) {
        return;
    }
    onceTotalCnt_++;
    onceTotalRealTime_ += event->realtime_;
    onceTotalProcTime_ += event->processTime_;
    onceTotalWaitTime_ += event->processTime_ - event->realtime_;
    AccumulateTimeInterval(event->realtime_, realStat_);
    AccumulateTimeInterval(event->processTime_, processStat_);
    AccumulateTimeInterval((event->processTime_ - event->realtime_), waitTimeStat_);
    if (event->realtime_ > realTimeBenchMark_) {
        overRealTotalCount_++;
    }
    if (event->processTime_ > processTimeBenchMark_) {
        overProcessTotalCount_++;
    }
    finishedCount_++;
    HIVIEW_LOGD("onceTotalCnt_=%{public}u, onceTotalRealTime_=%{public}u, onceTotalProcTime_=%{public}u, "
        "onceTotalWaitTime_=%{public}u, overRealTotalCount_=%{public}u, overProcessTotalCount_=%{public}u, "
        "finishedCount_=%{public}u",
        onceTotalCnt_, onceTotalRealTime_, onceTotalProcTime_,
        onceTotalWaitTime_, overRealTotalCount_, overProcessTotalCount_,
        finishedCount_);
}

void PlatformMonitor::CollectPerfProfiler()
{
    HIVIEW_LOGI("collect performance profiler");
    // collect data every 5 minute
    // collect event max size and max count
    if (maxTotalCount_ < SysEvent::totalCount_) {
        maxTotalCount_.store(SysEvent::totalCount_);
    }
    if (maxTotalSize_ < SysEvent::totalSize_) {
        maxTotalSize_.store(SysEvent::totalSize_);
    }
    // total count, total size
    totalCount_ = SysEvent::totalCount_;
    totalSize_ = static_cast<uint32_t>(SysEvent::totalSize_);
    // min speed, max speed
    uint32_t onceTotalRealTime = onceTotalRealTime_;
    uint32_t onceTotalProcTime = onceTotalProcTime_;
    uint32_t onceTotalWaitTime = onceTotalWaitTime_;
    uint32_t onceTotalCnt = onceTotalCnt_;
    onceTotalRealTime_ = 0;
    onceTotalProcTime_ = 0;
    onceTotalWaitTime_ = 0;
    onceTotalCnt_ = 0;
    if (onceTotalRealTime > 0) {
        curRealSpeed_ = (TimeUtil::SEC_TO_MICROSEC * onceTotalCnt) / onceTotalRealTime;
        if (minSpeed_ == 0 || (minSpeed_ > curRealSpeed_)) {
            minSpeed_ = curRealSpeed_;
        }
        if (curRealSpeed_ > maxSpeed_) {
            maxSpeed_ = curRealSpeed_;
        }
    } else {
        minSpeed_ = 0;
        maxSpeed_ = 0;
        curRealSpeed_ = 0;
    }
    if (onceTotalProcTime > 0) {
        curProcSpeed_ = (TimeUtil::SEC_TO_MICROSEC * onceTotalCnt) / onceTotalProcTime;
    } else {
        curProcSpeed_ = 0;
    }
    if (onceTotalCnt > 0) {
        avgRealTime_ = static_cast<double>(onceTotalRealTime) / onceTotalCnt;
        avgProcessTime_ = static_cast<double>(onceTotalProcTime) / onceTotalCnt;
        avgWaitTime_ = static_cast<double>(onceTotalWaitTime) / onceTotalCnt;
    }
    HIVIEW_LOGD("maxTotalCount_=%{public}u, maxTotalSize_=%{public}u, totalCount_=%{public}u, totalSize_=%{public}u, "
        "onceTotalRealTime=%{public}u, onceTotalProcTime=%{public}u, onceTotalWaitTime=%{public}u, "
        "onceTotalCnt=%{public}u, minSpeed_=%{public}u, maxSpeed_=%{public}u, "
        "curRealSpeed_=%{public}u, curProcSpeed_=%{public}u, "
        "avgRealTime_=%{public}f, avgProcessTime_=%{public}f, avgWaitTime_=%{public}f",
        maxTotalCount_.load(), maxTotalSize_.load(), totalCount_, totalSize_,
        onceTotalRealTime, onceTotalProcTime, onceTotalWaitTime,
        onceTotalCnt, minSpeed_, maxSpeed_,
        curRealSpeed_, curProcSpeed_,
        avgRealTime_, avgProcessTime_, avgWaitTime_);
}

void PlatformMonitor::GetDomainsStat(PerfMeasure &perfMeasure)
{
    std::lock_guard<std::mutex> lock(topMutex_);
    for (auto it = topDomains_.begin(); it != topDomains_.end(); it++) {
        perfMeasure.domains.emplace_back(it->first);
        perfMeasure.domainCounts.emplace_back(it->second);
    }
    topDomains_.clear();
    topEvents_.clear();
}

void PlatformMonitor::GetCostTimeInterval(PerfMeasure &perfMeasure)
{
    std::lock_guard<std::mutex> lock(statMutex_);
    for (int index = 0; index <= static_cast<int>(sizeof(intervals_) / sizeof(intervals_[0])); index++) {
        uint32_t realCount = realStat_[index];
        perfMeasure.realCounts.emplace_back(realCount);
        uint32_t processCount = processStat_[index];
        perfMeasure.processCounts.emplace_back(processCount);
        uint32_t waitCount = waitTimeStat_[index];
        perfMeasure.waitCounts.emplace_back(waitCount);
    }
    realStat_.clear();
    processStat_.clear();
    waitTimeStat_.clear();
}

void PlatformMonitor::CalcOverBenckMarkPct(PerfMeasure &perfMeasure)
{
    perfMeasure.finishedCount = finishedCount_;
    perfMeasure.overRealTotalCount = overRealTotalCount_;
    perfMeasure.overProcessTotalCount = overProcessTotalCount_;
    finishedCount_ = 0;
    overRealTotalCount_ = 0;
    overProcessTotalCount_ = 0;

    if (perfMeasure.finishedCount > 0) {
        perfMeasure.realPercent = (PCT * perfMeasure.overRealTotalCount) / perfMeasure.finishedCount;
    } else if (perfMeasure.overRealTotalCount > 0) {
        perfMeasure.realPercent = PCT;
    }

    if (perfMeasure.finishedCount > 0) {
        perfMeasure.processpercent = (PCT * perfMeasure.overProcessTotalCount) / perfMeasure.finishedCount;
    } else if (perfMeasure.overProcessTotalCount > 0) {
        perfMeasure.processpercent = PCT;
    }
}

void PlatformMonitor::GetMaxTotalMeasure(PerfMeasure &perfMeasure)
{
    perfMeasure.maxTotalCount = maxTotalCount_.load();
    maxTotalCount_.store(0);

    perfMeasure.maxTotalSize = maxTotalSize_.load();
    maxTotalSize_.store(0);
}

void PlatformMonitor::GetBreakStat(PerfMeasure &perfMeasure)
{
    perfMeasure.totalCount = totalCount_;
    totalCount_ = 0;

    perfMeasure.totalSize = totalSize_;
    totalSize_ = 0;

    perfMeasure.breakCount = breakCount_;
    breakCount_ = 0;

    perfMeasure.breakDuration = breakDuration_;
    breakDuration_ = 0;
}

void PlatformMonitor::GetMaxSpeed(PerfMeasure &perfMeasure) const
{
    perfMeasure.minSpeed = minSpeed_;
    perfMeasure.maxSpeed = maxSpeed_;
}

std::shared_ptr<SysEvent> PlatformMonitor::CreateProfileReport(PerfMeasure &perfMeasure)
{
    SysEventCreator eventCreator("HIVIEWDFX", "PROFILE_STAT", SysEventCreator::STATISTIC);
    eventCreator.SetKeyValue("MAX_TOTAL_COUNT", perfMeasure.maxTotalCount);
    eventCreator.SetKeyValue("MAX_TOTAL_SIZE", perfMeasure.maxTotalSize);
    eventCreator.SetKeyValue("DOMAINS", perfMeasure.domains);
    eventCreator.SetKeyValue("DOMAIN_DETAIL", perfMeasure.domainCounts);
    eventCreator.SetKeyValue("TOTAL_COUNT", perfMeasure.totalCount);
    eventCreator.SetKeyValue("TOTAL_SIZE", perfMeasure.totalSize);
    eventCreator.SetKeyValue("BREAK_COUNT", perfMeasure.breakCount);
    eventCreator.SetKeyValue("BREAK_DURATION", perfMeasure.breakDuration);
    eventCreator.SetKeyValue("MIN_SPEED", perfMeasure.minSpeed);
    eventCreator.SetKeyValue("MAX_SPEED", perfMeasure.maxSpeed);
    eventCreator.SetKeyValue("REAL_COUNT", perfMeasure.realCounts);
    eventCreator.SetKeyValue("PROCESS_COUNT", perfMeasure.processCounts);
    eventCreator.SetKeyValue("WAIT_COUNT", perfMeasure.waitCounts);
    eventCreator.SetKeyValue("FINISHED_COUNT", perfMeasure.finishedCount);
    eventCreator.SetKeyValue("OVER_REAL_COUNT", perfMeasure.overRealTotalCount);
    eventCreator.SetKeyValue("OVER_REAL_PCT", perfMeasure.realPercent);
    eventCreator.SetKeyValue("OVER_PROC_COUNT", perfMeasure.overProcessTotalCount);
    eventCreator.SetKeyValue("OVER_PROC_PCT", perfMeasure.processpercent);
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("", nullptr, eventCreator);
    return sysEvent;
}

void PlatformMonitor::ReportCycleProfile()
{
    HIVIEW_LOGI("report performance profile");
    PerfMeasure perfMeasure;
    // report max event size and count
    GetMaxTotalMeasure(perfMeasure);

    // report event number of each domain
    GetDomainsStat(perfMeasure);

    // report total number of event, time of break, duration of break
    GetBreakStat(perfMeasure);

    // report min speed, max speed
    GetMaxSpeed(perfMeasure);

    // report real time, process time, wait time of cost time interval
    GetCostTimeInterval(perfMeasure);

    // report percent and total number of over benchmark
    CalcOverBenckMarkPct(perfMeasure);

    std::shared_ptr<SysEvent> sysEvent = CreateProfileReport(perfMeasure);
    HIVIEW_LOGI("report=%{public}s", sysEvent->jsonExtraInfo_.c_str());
    HiviewGlobal::GetInstance()->PostSyncEventToTarget(EVENT_SERVICE_PLUGIN, sysEvent);
    HIVIEW_LOGI("report performance profile have done");
}

void PlatformMonitor::GetTopDomains(std::vector<std::string> &domains, std::vector<uint32_t> &counts)
{
    std::lock_guard<std::mutex> lock(topMutex_);
    uint8_t topN = 3; // top n
    if (topDomains_.size() <= topN) {
        for (auto it = topDomains_.begin(); it != topDomains_.end(); it++) {
            domains.emplace_back(it->first);
            counts.emplace_back(it->second);
        }
        return;
    }

    for (auto it = topDomains_.begin(); it != topDomains_.end(); it++) {
        counts.emplace_back(it->second);
    }
    std::sort(counts.begin(), counts.end(), std::greater<int>());
    counts.resize(topN);
    for (auto it = topDomains_.begin(); it != topDomains_.end(); it++) {
        if (domains.size() >= topN) {
            break;
        }
        if (std::find(counts.begin(), counts.end(), it->second) != counts.end()) {
            domains.emplace_back(it->first);
        }
    }
    return;
}

void PlatformMonitor::GetTopEvents(std::vector<std::string> &events, std::vector<uint32_t> &counts)
{
    std::lock_guard<std::mutex> lock(topMutex_);
    uint8_t topN = 3; // top n
    if (topEvents_.size() <= topN) {
        for (auto it = topEvents_.begin(); it != topEvents_.end(); it++) {
            events.emplace_back(it->first);
            counts.emplace_back(it->second);
        }
        return;
    }

    for (auto it = topEvents_.begin(); it != topEvents_.end(); it++) {
        counts.emplace_back(it->second);
    }
    std::sort(counts.begin(), counts.end(), std::greater<int>());
    counts.resize(topN);
    for (auto it = topEvents_.begin(); it != topEvents_.end(); it++) {
        if (events.size() >= topN) {
            break;
        }
        if (std::find(counts.begin(), counts.end(), it->second) != counts.end()) {
            events.emplace_back(it->first);
        }
    }
    return;
}

void PlatformMonitor::ReportBreakProfile()
{
    // report current event size and count
    uint32_t curTotalCount_ = SysEvent::totalCount_;
    uint32_t curTotalSize_ = static_cast<uint32_t>(SysEvent::totalSize_);

    // report current speed
    uint32_t curRealSpeed = curRealSpeed_;
    uint32_t curProcessSpeed = curProcSpeed_;

    // report average real time, process time, wait time
    double avgRealTime = avgRealTime_;
    double avgProcessTime = avgProcessTime_;
    double avgWaitTime = avgWaitTime_;

    // report topk cost time event
    std::vector<std::string> events;
    std::vector<uint32_t> eventCounts;
    GetTopEvents(events, eventCounts);

    // report topk event and count
    std::vector<std::string> domains;
    std::vector<uint32_t> domainCounts;
    GetTopDomains(domains, domainCounts);
    SysEventCreator eventCreator("HIVIEWDFX", "BREAK", SysEventCreator::BEHAVIOR);
    eventCreator.SetKeyValue("TOTAL_COUNT", curTotalCount_);
    eventCreator.SetKeyValue("TOTAL_SIZE", curTotalSize_);
    eventCreator.SetKeyValue("REAL_SPEED", curRealSpeed);
    eventCreator.SetKeyValue("PROC_SPEED", curProcessSpeed);
    eventCreator.SetKeyValue("AVG_REAL_TIME", avgRealTime);
    eventCreator.SetKeyValue("AVG_PROC_TIME", avgProcessTime);
    eventCreator.SetKeyValue("AVG_WAIT_TIME", avgWaitTime);
    eventCreator.SetKeyValue("TOP_EVENT", events);
    eventCreator.SetKeyValue("TOP_EVENT_COUNT", eventCounts);
    eventCreator.SetKeyValue("TOP_DOMAIN", domains);
    eventCreator.SetKeyValue("TOP_DOMAIN_COUNT", domainCounts);
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("", nullptr, eventCreator);
    HIVIEW_LOGI("report=%{public}s", sysEvent->jsonExtraInfo_.c_str());
    HiviewGlobal::GetInstance()->PostSyncEventToTarget(EVENT_SERVICE_PLUGIN, sysEvent);
}

void PlatformMonitor::ReportRecoverProfile()
{
    // report break duration when recovery
    int64_t duration = static_cast<int64_t>(recoverTimestamp_ - breakTimestamp_);
    SysEventCreator eventCreator("HIVIEWDFX", "RECOVER", SysEventCreator::BEHAVIOR);
    eventCreator.SetKeyValue("DURATION", duration);
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("", nullptr, eventCreator);
    HIVIEW_LOGI("report=%{public}s", sysEvent->jsonExtraInfo_.c_str());
    HiviewGlobal::GetInstance()->PostSyncEventToTarget(EVENT_SERVICE_PLUGIN, sysEvent);
}

void PlatformMonitor::Breaking()
{
    // collect break count and duration every break
    if (SysEvent::totalSize_ <= totalSizeBenchMark_) {
        return;
    }

    HIVIEW_LOGE("break as event reach critical size %{public}u", SysEvent::totalSize_.load());
    breakTimestamp_ = TimeUtil::GenerateTimestamp();
    ReportBreakProfile();
    int64_t recoveryBenchMark = static_cast<int64_t>(totalSizeBenchMark_ * 0.8); // 0.8 of total size will recover
    while (true) {
        if (SysEvent::totalSize_ <= recoveryBenchMark) {
            break;
        }
        TimeUtil::Sleep(SLEEP_TEN_SECONDS);
    }
    breakCount_++;
    recoverTimestamp_ = TimeUtil::GenerateTimestamp();
    breakDuration_ += recoverTimestamp_ - breakTimestamp_;
    HIVIEW_LOGW("recover after break duration %{public}ld", breakDuration_);
    ReportRecoverProfile();
}

void PlatformMonitor::InitData()
{
    MonitorConfig monitorConfig("/system/etc/hiview/monitor.cfg");
    if (!monitorConfig.Parse()) {
        return;
    }

    monitorConfig.ReadParam("collectPeriod", collectPeriod_);
    monitorConfig.ReadParam("reportPeriod", reportPeriod_);
    monitorConfig.ReadParam("totalSizeBenchMark", totalSizeBenchMark_);
    monitorConfig.ReadParam("realTimeBenchMark", realTimeBenchMark_);
    monitorConfig.ReadParam("processTimeBenchMark", processTimeBenchMark_);
}

void PlatformMonitor::StartMonitor(std::shared_ptr<EventLoop> looper)
{
    if (looper == nullptr) {
        HIVIEW_LOGE("can not get share looper");
        return;
    }
    InitData();

    looper_ = looper;

    auto collectTask = std::bind(&PlatformMonitor::CollectPerfProfiler, this);
    looper_->AddTimerEvent(nullptr, nullptr, collectTask, collectPeriod_, true);
    auto reportTask = std::bind(&PlatformMonitor::ReportCycleProfile, this);
    looper_->AddTimerEvent(nullptr, nullptr, reportTask, reportPeriod_, true);
}
} // namespace HiviewDFX
} // namespace OHOS