/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "cpu_collector_impl.h"

#include <algorithm>
#include <cinttypes>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#include "cpu_decorator.h"
#include "logger.h"
#include "process_status.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
DEFINE_LOG_TAG("CpuCollector");
CpuCollectorImpl::CpuCollectorImpl()
{
    if (InitDeviceClient()) {
        InitLastProcCpuTimeInfos();
    }

    // init sys cpu usage infos
    (void)CollectSysCpuUsage(true);
}

bool CpuCollectorImpl::InitDeviceClient()
{
    deviceClient_ = std::make_unique<CollectDeviceClient>();
    if (deviceClient_->Open() != 0) {
        HIVIEW_LOGE("failed to open device client");
        deviceClient_ = nullptr;
        return false;
    }
    auto cpuDmipses = cpuCalculator_.GetCpuDmipses();
    if (cpuDmipses.empty()) {
        HIVIEW_LOGE("failed to get cpu dmipses");
        deviceClient_ = nullptr;
        return false;
    }
    std::vector<char> clientCpuDmipses;
    std::transform(cpuDmipses.begin(), cpuDmipses.end(), std::back_inserter(clientCpuDmipses), [](uint32_t cpuDmipse) {
        return static_cast<char>(cpuDmipse);
    });
    deviceClient_->SetDmips(clientCpuDmipses);
    return true;
}

void CpuCollectorImpl::InitLastProcCpuTimeInfos()
{
    auto processCpuData = FetchProcessCpuData();
    if (processCpuData == nullptr) {
        return;
    }

    // init cpu time information for each process in the system
    CalculationTimeInfo calcTimeInfo = InitCalculationTimeInfo();
    auto procCpuItem = processCpuData->GetNextProcess();
    while (procCpuItem != nullptr) {
        UpdateLastProcCpuTimeInfo(procCpuItem, calcTimeInfo);
        procCpuItem = processCpuData->GetNextProcess();
    }
    UpdateCollectionTime(calcTimeInfo);
}

std::shared_ptr<ProcessCpuData> CpuCollectorImpl::FetchProcessCpuData(int32_t pid)
{
    if (deviceClient_ == nullptr) {
        HIVIEW_LOGW("device client is null");
        return nullptr;
    }
    return (pid == INVALID_PID)
        ? deviceClient_->FetchProcessCpuData()
        : deviceClient_->FetchProcessCpuData(pid);
}

void CpuCollectorImpl::UpdateCollectionTime(const CalculationTimeInfo& calcTimeInfo)
{
    lastCollectionTime_ = calcTimeInfo.endTime;
    lastCollectionMonoTime_ = calcTimeInfo.endMonoTime;
}

void CpuCollectorImpl::UpdateLastProcCpuTimeInfo(const ucollection_process_cpu_item* procCpuItem,
    const CalculationTimeInfo& calcTimeInfo)
{
    lastProcCpuTimeInfos_[procCpuItem->pid] = {
        .minFlt = procCpuItem->min_flt,
        .majFlt = procCpuItem->maj_flt,
        .uUsageTime = procCpuItem->cpu_usage_utime,
        .sUsageTime = procCpuItem->cpu_usage_stime,
        .loadTime = procCpuItem->cpu_load_time,
        .collectionTime = calcTimeInfo.endTime,
        .collectionMonoTime = calcTimeInfo.endMonoTime,
    };
}

std::shared_ptr<CpuCollector> CpuCollector::Create()
{
    static std::shared_ptr<CpuCollector> instance_ =
        std::make_shared<CpuDecorator>(std::make_shared<CpuCollectorImpl>());
    return instance_;
}

CollectResult<SysCpuLoad> CpuCollectorImpl::CollectSysCpuLoad()
{
    CollectResult<SysCpuLoad> result;
    result.retCode = CpuUtil::GetSysCpuLoad(result.data);
    return result;
}

CollectResult<SysCpuUsage> CpuCollectorImpl::CollectSysCpuUsage(bool isNeedUpdate)
{
    CollectResult<SysCpuUsage> result;
    std::vector<CpuTimeInfo> currCpuTimeInfos;
    result.retCode = CpuUtil::GetCpuTimeInfos(currCpuTimeInfos);
    if (result.retCode != UCollect::UcError::SUCCESS) {
        return result;
    }

    SysCpuUsage& sysCpuUsage = result.data;
    sysCpuUsage.startTime = lastSysCpuUsageTime_;
    sysCpuUsage.endTime = TimeUtil::GetMilliseconds();
    CalculateSysCpuUsageInfos(sysCpuUsage.cpuInfos, currCpuTimeInfos);

    if (isNeedUpdate) {
        lastSysCpuUsageTime_ = sysCpuUsage.endTime;
        lastSysCpuTimeInfos_ = currCpuTimeInfos;
    }
    HIVIEW_LOGD("collect system cpu usage, size=%{public}zu, isNeedUpdate=%{public}d",
        sysCpuUsage.cpuInfos.size(), isNeedUpdate);
    return result;
}

CollectResult<double> CpuCollectorImpl::GetSysCpuUsage()
{
    auto sysCpuUsage = CollectSysCpuUsage();
    double retValue = 0;
    if (!sysCpuUsage.data.cpuInfos.empty()) {
        auto &totalCpuUsageInfo = sysCpuUsage.data.cpuInfos.at(0);
        retValue += (totalCpuUsageInfo.userUsage + totalCpuUsageInfo.niceUsage + totalCpuUsageInfo.systemUsage);
    }
    CollectResult<double> result = {};
    result.retCode = UCollect::UcError::SUCCESS;
    result.data = retValue;
    return result;
}

void CpuCollectorImpl::CalculateSysCpuUsageInfos(std::vector<CpuUsageInfo>& cpuInfos,
    const std::vector<CpuTimeInfo>& currCpuTimeInfos)
{
    if (currCpuTimeInfos.size() != lastSysCpuTimeInfos_.size()) {
        return;
    }

    for (size_t i = 0; i < currCpuTimeInfos.size(); ++i) {
        cpuInfos.emplace_back(cpuCalculator_.CalculateSysCpuUsageInfo(currCpuTimeInfos[i],
            lastSysCpuTimeInfos_[i]));
    }
}

CollectResult<std::vector<CpuFreq>> CpuCollectorImpl::CollectCpuFrequency()
{
    CollectResult<std::vector<CpuFreq>> result;
    result.retCode = CpuUtil::GetCpuFrequency(result.data);
    return result;
}

CollectResult<std::vector<ProcessCpuStatInfo>> CpuCollectorImpl::CollectProcessCpuStatInfos(bool isNeedUpdate)
{
    CollectResult<std::vector<ProcessCpuStatInfo>> cpuCollectResult = {
        .retCode = UCollect::UcError::UNSUPPORT,
        .data = {},
    };

    std::unique_lock<std::mutex> lock(collectMutex_);
    auto processCpuData = FetchProcessCpuData();
    if (processCpuData == nullptr) {
        return cpuCollectResult;
    }

    CalculateProcessCpuStatInfos(cpuCollectResult.data, processCpuData, isNeedUpdate);
    HIVIEW_LOGI("collect process cpu statistics information size=%{public}zu, isNeedUpdate=%{public}d",
        cpuCollectResult.data.size(), isNeedUpdate);
    if (!cpuCollectResult.data.empty()) {
        cpuCollectResult.retCode = UCollect::UcError::SUCCESS;
        TryToDeleteDeadProcessInfo();
    }
    return cpuCollectResult;
}

void CpuCollectorImpl::CalculateProcessCpuStatInfos(
    std::vector<ProcessCpuStatInfo>& processCpuStatInfos,
    std::shared_ptr<ProcessCpuData> processCpuData,
    bool isNeedUpdate)
{
    CalculationTimeInfo calcTimeInfo = InitCalculationTimeInfo();
    HIVIEW_LOGI("startTime=%{public}" PRIu64 ", endTime=%{public}" PRIu64 ", startMonoTime=%{public}" PRIu64
        ", endMonoTime=%{public}" PRIu64 ", period=%{public}" PRIu64, calcTimeInfo.startTime,
        calcTimeInfo.endTime, calcTimeInfo.startMonoTime, calcTimeInfo.endMonoTime, calcTimeInfo.period);
    auto procCpuItem = processCpuData->GetNextProcess();
    while (procCpuItem != nullptr) {
        auto processCpuStatInfo = CalculateProcessCpuStatInfo(procCpuItem, calcTimeInfo);
        if (processCpuStatInfo.has_value()) {
            processCpuStatInfos.emplace_back(processCpuStatInfo.value());
        }
        if (isNeedUpdate) {
            UpdateLastProcCpuTimeInfo(procCpuItem, calcTimeInfo);
        }
        procCpuItem = processCpuData->GetNextProcess();
    }

    if (isNeedUpdate) {
        UpdateCollectionTime(calcTimeInfo);
    }
}

CalculationTimeInfo CpuCollectorImpl::InitCalculationTimeInfo()
{
    CalculationTimeInfo calcTimeInfo = {
        .startTime = lastCollectionTime_,
        .endTime = TimeUtil::GetMilliseconds(),
        .startMonoTime = lastCollectionMonoTime_,
        .endMonoTime = TimeUtil::GetSteadyClockTimeMs(),
    };
    calcTimeInfo.period = calcTimeInfo.endMonoTime > calcTimeInfo.startMonoTime
        ? (calcTimeInfo.endMonoTime - calcTimeInfo.startMonoTime) : 0;
    return calcTimeInfo;
}

std::optional<ProcessCpuStatInfo> CpuCollectorImpl::CalculateProcessCpuStatInfo(
    const ucollection_process_cpu_item* procCpuItem, const CalculationTimeInfo& calcTimeInfo)
{
    if (lastProcCpuTimeInfos_.find(procCpuItem->pid) == lastProcCpuTimeInfos_.end()) {
        return std::nullopt;
    }
    ProcessCpuStatInfo processCpuStatInfo;
    processCpuStatInfo.startTime = calcTimeInfo.startTime;
    processCpuStatInfo.endTime = calcTimeInfo.endTime;
    processCpuStatInfo.pid = procCpuItem->pid;
    processCpuStatInfo.minFlt = procCpuItem->min_flt;
    processCpuStatInfo.majFlt = procCpuItem->maj_flt;
    processCpuStatInfo.procName = ProcessStatus::GetInstance().GetProcessName(procCpuItem->pid);
    processCpuStatInfo.cpuLoad = cpuCalculator_.CalculateCpuLoad(procCpuItem->cpu_load_time,
        lastProcCpuTimeInfos_[procCpuItem->pid].loadTime, calcTimeInfo.period);
    processCpuStatInfo.uCpuUsage = cpuCalculator_.CalculateCpuUsage(procCpuItem->cpu_usage_utime,
        lastProcCpuTimeInfos_[procCpuItem->pid].uUsageTime, calcTimeInfo.period);
    processCpuStatInfo.sCpuUsage = cpuCalculator_.CalculateCpuUsage(procCpuItem->cpu_usage_stime,
        lastProcCpuTimeInfos_[procCpuItem->pid].sUsageTime, calcTimeInfo.period);
    processCpuStatInfo.cpuUsage = processCpuStatInfo.uCpuUsage + processCpuStatInfo.sCpuUsage;
    if (processCpuStatInfo.cpuLoad >= 1) { // 1: max cpu load
        HIVIEW_LOGI("invalid cpu load=%{public}f, name=%{public}s, last_load=%{public}" PRIu64
            ", curr_load=%{public}" PRIu64, processCpuStatInfo.cpuLoad, processCpuStatInfo.procName.c_str(),
            lastProcCpuTimeInfos_[procCpuItem->pid].loadTime, static_cast<uint64_t>(procCpuItem->cpu_load_time));
    }
    return std::make_optional<ProcessCpuStatInfo>(processCpuStatInfo);
}

void CpuCollectorImpl::TryToDeleteDeadProcessInfo()
{
    for (auto it = lastProcCpuTimeInfos_.begin(); it != lastProcCpuTimeInfos_.end();) {
        // if the latest collection operation does not update the process collection time, delete it
        if (it->second.collectionTime != lastCollectionTime_) {
            it = lastProcCpuTimeInfos_.erase(it);
        } else {
            it++;
        }
    }
    HIVIEW_LOGI("end to delete dead process, size=%{public}zu", lastProcCpuTimeInfos_.size());
}

CollectResult<ProcessCpuStatInfo> CpuCollectorImpl::CollectProcessCpuStatInfo(int32_t pid, bool isNeedUpdate)
{
    CollectResult<ProcessCpuStatInfo> cpuCollectResult;
    cpuCollectResult.retCode = UCollect::UcError::UNSUPPORT;

    std::unique_lock<std::mutex> lock(collectMutex_);
    auto processCpuData = FetchProcessCpuData(pid);
    if (processCpuData == nullptr) {
        return cpuCollectResult;
    }
    auto procCpuItem = processCpuData->GetNextProcess();
    if (procCpuItem == nullptr) {
        return cpuCollectResult;
    }

    CalculationTimeInfo calcTimeInfo = InitCalculationTimeInfo(pid);
    auto processCpuStatInfo = CalculateProcessCpuStatInfo(procCpuItem, calcTimeInfo);
    if (processCpuStatInfo.has_value()) {
        cpuCollectResult.retCode = UCollect::UcError::SUCCESS;
        cpuCollectResult.data = processCpuStatInfo.value();
    }
    if (isNeedUpdate) {
        UpdateLastProcCpuTimeInfo(procCpuItem, calcTimeInfo);
    }
    return cpuCollectResult;
}

CalculationTimeInfo CpuCollectorImpl::InitCalculationTimeInfo(int32_t pid)
{
    CalculationTimeInfo calcTimeInfo = {
        .startTime = lastProcCpuTimeInfos_.find(pid) != lastProcCpuTimeInfos_.end()
            ? lastProcCpuTimeInfos_[pid].collectionTime : 0,
        .endTime = TimeUtil::GetMilliseconds(),
        .startMonoTime = lastProcCpuTimeInfos_.find(pid) != lastProcCpuTimeInfos_.end()
            ? lastProcCpuTimeInfos_[pid].collectionMonoTime : 0,
        .endMonoTime = TimeUtil::GetSteadyClockTimeMs(),
    };
    calcTimeInfo.period = calcTimeInfo.endMonoTime > calcTimeInfo.startMonoTime
        ? (calcTimeInfo.endMonoTime - calcTimeInfo.startMonoTime) : 0;
    return calcTimeInfo;
}
} // UCollectUtil
} // HiViewDFX
} // OHOS
