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
#include "thread_state_info_collector.h"

#include <sys/types.h>
#include <unistd.h>

#include "logger.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
DEFINE_LOG_TAG("CpuCollector");

ThreadStateInfoCollector::ThreadStateInfoCollector(std::shared_ptr<CollectDeviceClient> deviceClient,
    std::shared_ptr<CpuCalculator> cpuCalculator, int collectPid): deviceClient_(deviceClient),
    cpuCalculator_(cpuCalculator), collectPid_(collectPid)
{
    InitLastThreadCpuTimeInfos();
}

void ThreadStateInfoCollector::InitLastThreadCpuTimeInfos()
{
    auto threadCpuData = FetchThreadCpuData();
    if (threadCpuData == nullptr) {
        return;
    }

    // init cpu time information for each thread in the system
    CalculationTimeInfo calcTimeInfo = InitCalculationTimeInfo();
    auto threadCpuItem = threadCpuData->GetNextThread();
    while (threadCpuItem != nullptr) {
        UpdateLastThreadTimeInfo(threadCpuItem, calcTimeInfo);
        threadCpuItem = threadCpuData->GetNextThread();
    }
    UpdateCollectionTime(calcTimeInfo);
}

CalculationTimeInfo ThreadStateInfoCollector::InitCalculationTimeInfo()
{
    CalculationTimeInfo calcTimeInfo = {
        .startTime = lastCollectionTime_,
        .endTime = TimeUtil::GetMilliseconds(),
        .startBootTime = lastCollectionBootTime_,
        .endBootTime = TimeUtil::GetBootTimeMs(),
    };
    calcTimeInfo.period = calcTimeInfo.endBootTime > calcTimeInfo.startBootTime
        ? (calcTimeInfo.endBootTime - calcTimeInfo.startBootTime) : 0;
    return calcTimeInfo;
}

void ThreadStateInfoCollector::UpdateLastThreadTimeInfo(const ucollection_thread_cpu_item* threadCpuItem,
    const CalculationTimeInfo& calcTimeInfo)
{
    lastThreadCpuTimeInfos_[threadCpuItem->tid] = {
        .uUsageTime = threadCpuItem->cpu_usage_utime,
        .sUsageTime = threadCpuItem->cpu_usage_stime,
        .loadTime = threadCpuItem->cpu_load_time,
        .collectionTime = calcTimeInfo.endTime,
        .collectionBootTime = calcTimeInfo.endBootTime,
    };
}

void ThreadStateInfoCollector::UpdateCollectionTime(const CalculationTimeInfo& calcTimeInfo)
{
    lastCollectionTime_ = calcTimeInfo.endTime;
    lastCollectionBootTime_ = calcTimeInfo.endBootTime;
}

CollectResult<std::vector<ThreadCpuStatInfo>> ThreadStateInfoCollector::CollectThreadStatInfos(bool isNeedUpdate)
{
    CollectResult<std::vector<ThreadCpuStatInfo>> threadCollectResult;
    std::unique_lock<std::mutex> lock(collectMutex_);
    auto threadCpuData = FetchThreadCpuData();
    if (threadCpuData == nullptr) {
        return threadCollectResult;
    }
    CalculateThreadCpuStatInfos(threadCollectResult.data, threadCpuData, isNeedUpdate);
    HIVIEW_LOGI("collect thread cpu statistics information size=%{public}zu, isNeedUpdate=%{public}d",
                threadCollectResult.data.size(), isNeedUpdate);
    if (!threadCollectResult.data.empty()) {
        threadCollectResult.retCode = UCollect::UcError::SUCCESS;
        TryToDeleteDeadThreadInfo();
    }
    return threadCollectResult;
}

int ThreadStateInfoCollector::GetCollectPid()
{
    return collectPid_;
}

std::shared_ptr<ThreadCpuData> ThreadStateInfoCollector::FetchThreadCpuData()
{
    if (deviceClient_ == nullptr) {
        HIVIEW_LOGW("device client is null");
        return nullptr;
    }
    return (collectPid_ == getprocpid()) ? deviceClient_->FetchSelfThreadCpuData(collectPid_)
        : deviceClient_->FetchThreadCpuData(collectPid_);
}

void ThreadStateInfoCollector::CalculateThreadCpuStatInfos(
    std::vector<ThreadCpuStatInfo>& threadCpuStatInfos,
    std::shared_ptr<ThreadCpuData> threadCpuData,
    bool isNeedUpdate)
{
    CalculationTimeInfo calcTimeInfo = InitCalculationTimeInfo();
    HIVIEW_LOGI("startTime=%{public}" PRIu64 ", endTime=%{public}" PRIu64 ", startBootTime=%{public}" PRIu64
        ", endBootTime=%{public}" PRIu64 ", period=%{public}" PRIu64, calcTimeInfo.startTime,
        calcTimeInfo.endTime, calcTimeInfo.startBootTime, calcTimeInfo.endBootTime, calcTimeInfo.period);
    auto procCpuItem = threadCpuData->GetNextThread();
    while (procCpuItem != nullptr) {
        auto threadCpuStatInfo = CalculateThreadCpuStatInfo(procCpuItem, calcTimeInfo);
        if (threadCpuStatInfo.has_value()) {
            threadCpuStatInfos.emplace_back(threadCpuStatInfo.value());
        }
        if (isNeedUpdate) {
            UpdateLastThreadTimeInfo(procCpuItem, calcTimeInfo);
        }
        procCpuItem = threadCpuData->GetNextThread();
    }

    if (isNeedUpdate) {
        UpdateCollectionTime(calcTimeInfo);
    }
}

void ThreadStateInfoCollector::TryToDeleteDeadThreadInfo()
{
    for (auto it = lastThreadCpuTimeInfos_.begin(); it != lastThreadCpuTimeInfos_.end();) {
        // if the latest collection operation does not update the process collection time, delete it
        if (it->second.collectionTime != lastCollectionTime_) {
            it = lastThreadCpuTimeInfos_.erase(it);
        } else {
            it++;
        }
    }
    HIVIEW_LOGI("end to delete dead thread, size=%{public}zu", lastThreadCpuTimeInfos_.size());
}

std::optional<ThreadCpuStatInfo> ThreadStateInfoCollector::CalculateThreadCpuStatInfo(
    const ucollection_thread_cpu_item* threadCpuItem, const CalculationTimeInfo& calcTimeInfo)
{
    if (cpuCalculator_ == nullptr) {
        return std::nullopt;
    }
    if (lastThreadCpuTimeInfos_.find(threadCpuItem->tid) == lastThreadCpuTimeInfos_.end()) {
        return std::nullopt;
    }
    ThreadCpuStatInfo threadCpuStatInfo;
    threadCpuStatInfo.startTime = calcTimeInfo.startTime;
    threadCpuStatInfo.endTime = calcTimeInfo.endTime;
    threadCpuStatInfo.tid = threadCpuItem->tid;
    threadCpuStatInfo.cpuLoad = cpuCalculator_->CalculateCpuLoad(threadCpuItem->cpu_load_time,
        lastThreadCpuTimeInfos_[threadCpuItem->tid].loadTime, calcTimeInfo.period);
    threadCpuStatInfo.uCpuUsage = cpuCalculator_->CalculateCpuUsage(threadCpuItem->cpu_usage_utime,
        lastThreadCpuTimeInfos_[threadCpuItem->tid].uUsageTime, calcTimeInfo.period);
    threadCpuStatInfo.sCpuUsage = cpuCalculator_->CalculateCpuUsage(threadCpuItem->cpu_usage_stime,
        lastThreadCpuTimeInfos_[threadCpuItem->tid].sUsageTime, calcTimeInfo.period);
    threadCpuStatInfo.cpuUsage = threadCpuStatInfo.uCpuUsage + threadCpuStatInfo.sCpuUsage;
    if (threadCpuStatInfo.cpuLoad >= 1) { // 1: max cpu load
        HIVIEW_LOGI("invalid cpu load=%{public}f, last_load=%{public}" PRIu64
            ", curr_load=%{public}" PRIu64, threadCpuStatInfo.cpuLoad,
            lastThreadCpuTimeInfos_[threadCpuItem->tid].loadTime, static_cast<uint64_t>(threadCpuItem->cpu_load_time));
    }
    return std::make_optional<ThreadCpuStatInfo>(threadCpuStatInfo);
}
} // UCollectUtil
} // HiViewDFX
} // OHOS