/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "cpu_collector.h"

#include <algorithm>
#include <cinttypes>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#include "collect_device_client.h"
#include "cpu_calculator.h"
#include "logger.h"
#include "process_status.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
DEFINE_LOG_TAG("UCollectUtil-CpuCollector");
namespace {
constexpr int32_t INVALID_PID = 0;
}

struct ProcessCpuTimeInfo {
    uint64_t cpuUsageTime = 0;
    uint64_t cpuLoadTime = 0;
    uint64_t collectionTime = 0;
};

class CpuCollectorImpl : public CpuCollector {
public:
    CpuCollectorImpl();
    virtual ~CpuCollectorImpl() = default;

public:
    virtual CollectResult<SysCpuLoad> CollectSysCpuLoad() override;
    virtual CollectResult<SysCpuUsage> CollectSysCpuUsage() override;
    virtual CollectResult<ProcessCpuUsage> CollectProcessCpuUsage(int32_t pid) override;
    virtual CollectResult<ProcessCpuLoad> CollectProcessCpuLoad(int32_t pid) override;
    virtual CollectResult<ProcessCpuStatInfo> CollectProcessCpuStatInfo(int32_t pid) override;
    virtual CollectResult<CpuFreqStat> CollectCpuFreqStat() override;
    virtual CollectResult<std::vector<CpuFreq>> CollectCpuFrequency() override;
    virtual CollectResult<std::vector<ProcessCpuUsage>> CollectProcessCpuUsages() override;
    virtual CollectResult<std::vector<ProcessCpuLoad>> CollectProcessCpuLoads() override;
    virtual CollectResult<std::vector<ProcessCpuStatInfo>> CollectProcessCpuStatInfos(
        bool isNeedUpdate = false) override;

private:
    bool InitDeviceClient();
    void InitLastProcCpuTimeInfos();
    std::shared_ptr<ProcessCpuData> FetchProcessCpuData(int32_t pid = INVALID_PID);
    void UpdateCollectionTime();
    void UpdateLastProcCpuTimeInfo(const ucollection_process_cpu_item* procCpuItem);
    void CalculateProcessCpuStatInfos(
        std::vector<ProcessCpuStatInfo>& processCpuStatInfos,
        std::shared_ptr<ProcessCpuData> processCpuData,
        bool isNeedUpdate);
    std::optional<ProcessCpuStatInfo> CalculateProcessCpuStatInfo(
        const ucollection_process_cpu_item* procCpuItem, uint64_t startTime, uint64_t endTime);
    void UpdateClearTime();
    void TryToDeleteDeadProcessInfo();
    bool NeedDeleteDeadProcessInfo();

private:
    std::mutex collectMutex_;
    uint64_t currCollectionTime_ = 0;
    uint64_t lastCollectionTime_ = 0;
    uint64_t clearTime_ = 0;
    std::unique_ptr<CollectDeviceClient> deviceClient_;
    /* map<pid, ProcessCpuTimeInfo> */
    std::unordered_map<int32_t, ProcessCpuTimeInfo> lastProcCpuTimeInfos_;
    CpuCalculator cpuCalculator_;
};

CpuCollectorImpl::CpuCollectorImpl()
{
    if (InitDeviceClient()) {
        InitLastProcCpuTimeInfos();
    }
    UpdateClearTime();
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
    UpdateCollectionTime();

    // init cpu time information for each process in the system
    auto procCpuItem = processCpuData->GetNextProcess();
    while (procCpuItem != nullptr) {
        UpdateLastProcCpuTimeInfo(procCpuItem);
        procCpuItem = processCpuData->GetNextProcess();
    }
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

void CpuCollectorImpl::UpdateCollectionTime()
{
    lastCollectionTime_ = currCollectionTime_;
    currCollectionTime_ = TimeUtil::GetMilliseconds();
}

void CpuCollectorImpl::UpdateLastProcCpuTimeInfo(const ucollection_process_cpu_item* procCpuItem)
{
    if (lastProcCpuTimeInfos_.find(procCpuItem->pid) == lastProcCpuTimeInfos_.end()) {
        lastProcCpuTimeInfos_.insert({procCpuItem->pid, {
            .cpuUsageTime = procCpuItem->cpu_usage_utime + procCpuItem->cpu_usage_stime,
            .cpuLoadTime = procCpuItem->cpu_load_time,
            .collectionTime = currCollectionTime_,
        }});
    } else {
        lastProcCpuTimeInfos_[procCpuItem->pid].cpuUsageTime =
            procCpuItem->cpu_usage_utime + procCpuItem->cpu_usage_stime;
        lastProcCpuTimeInfos_[procCpuItem->pid].cpuLoadTime = procCpuItem->cpu_load_time;
        lastProcCpuTimeInfos_[procCpuItem->pid].collectionTime = currCollectionTime_;
    }
}

void CpuCollectorImpl::UpdateClearTime()
{
    clearTime_ = TimeUtil::GetMilliseconds();
}

std::shared_ptr<CpuCollector> CpuCollector::Create()
{
    static std::shared_ptr<CpuCollector> instance_ = std::make_shared<CpuCollectorImpl>();
    return instance_;
}

CollectResult<SysCpuLoad> CpuCollectorImpl::CollectSysCpuLoad()
{
    CollectResult<SysCpuLoad> result;
    result.retCode = UCollect::UcError::SUCCESS;
    return result;
}

CollectResult<SysCpuUsage> CpuCollectorImpl::CollectSysCpuUsage()
{
    CollectResult<SysCpuUsage> result;
    result.retCode = UCollect::UcError::SUCCESS;
    return result;
}

CollectResult<ProcessCpuUsage> CpuCollectorImpl::CollectProcessCpuUsage(int32_t pid)
{
    CollectResult<ProcessCpuUsage> result;
    return result;
}

CollectResult<ProcessCpuLoad> CpuCollectorImpl::CollectProcessCpuLoad(int32_t pid)
{
    CollectResult<ProcessCpuLoad> result;
    return result;
}

CollectResult<CpuFreqStat> CpuCollectorImpl::CollectCpuFreqStat()
{
    CollectResult<CpuFreqStat> result;
    return result;
}

CollectResult<std::vector<CpuFreq>> CpuCollectorImpl::CollectCpuFrequency()
{
    CollectResult<std::vector<CpuFreq>> result;
    return result;
}

CollectResult<std::vector<ProcessCpuUsage>> CpuCollectorImpl::CollectProcessCpuUsages()
{
    CollectResult<std::vector<ProcessCpuUsage>> result;
    return result;
}

CollectResult<std::vector<ProcessCpuLoad>> CpuCollectorImpl::CollectProcessCpuLoads()
{
    CollectResult<std::vector<ProcessCpuLoad>> result;
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
    if (isNeedUpdate) {
        UpdateCollectionTime();
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
    auto procCpuItem = processCpuData->GetNextProcess();
    while (procCpuItem != nullptr) {
        auto processCpuStatInfo = CalculateProcessCpuStatInfo(procCpuItem, lastCollectionTime_, currCollectionTime_);
        if (processCpuStatInfo.has_value()) {
            processCpuStatInfos.emplace_back(processCpuStatInfo.value());
        }
        if (isNeedUpdate) {
            UpdateLastProcCpuTimeInfo(procCpuItem);
        }
        procCpuItem = processCpuData->GetNextProcess();
    }
}

std::optional<ProcessCpuStatInfo> CpuCollectorImpl::CalculateProcessCpuStatInfo(
    const ucollection_process_cpu_item* procCpuItem, uint64_t startTime, uint64_t endTime)
{
    if (lastProcCpuTimeInfos_.find(procCpuItem->pid) == lastProcCpuTimeInfos_.end()) {
        return std::nullopt;
    }
    ProcessCpuStatInfo processCpuStatInfo;
    processCpuStatInfo.startTime = startTime;
    processCpuStatInfo.endTime = endTime;
    processCpuStatInfo.pid = procCpuItem->pid;
    processCpuStatInfo.procName = ProcessStatus::GetInstance().GetProcessName(procCpuItem->pid);
    uint64_t collectionPeriod = endTime > startTime ? (endTime - startTime) : 0;
    double procCpuLoad = cpuCalculator_.CalculateCpuLoad(procCpuItem->cpu_load_time,
        lastProcCpuTimeInfos_[procCpuItem->pid].cpuLoadTime, collectionPeriod);
    processCpuStatInfo.cpuLoad = procCpuLoad;
    uint64_t procCpuUsageTime = procCpuItem->cpu_usage_utime + procCpuItem->cpu_usage_stime;
    double procCpuUsage = cpuCalculator_.CalculateCpuUsage(procCpuUsageTime,
        lastProcCpuTimeInfos_[procCpuItem->pid].cpuUsageTime, collectionPeriod);
    processCpuStatInfo.cpuUsage = procCpuUsage;
    return std::make_optional<ProcessCpuStatInfo>(processCpuStatInfo);
}

void CpuCollectorImpl::TryToDeleteDeadProcessInfo()
{
    if (!NeedDeleteDeadProcessInfo()) {
        return;
    }
    HIVIEW_LOGI("start to delete dead process, size=%{public}zu", lastProcCpuTimeInfos_.size());
    for (auto it = lastProcCpuTimeInfos_.begin(); it != lastProcCpuTimeInfos_.end();) {
        // if the latest collection operation does not update the process collection time, delete it
        if (it->second.collectionTime != currCollectionTime_) {
            lastProcCpuTimeInfos_.erase(it++);
        } else {
            it++;
        }
    }
    UpdateClearTime();
    HIVIEW_LOGI("end to delete dead process, size=%{public}zu", lastProcCpuTimeInfos_.size());
}

bool CpuCollectorImpl::NeedDeleteDeadProcessInfo()
{
    constexpr uint64_t maxClearInterval = 3600 * 1000; // ms per hour
    uint64_t clearInterval = clearTime_ > currCollectionTime_ ?
        (clearTime_ - currCollectionTime_) : (currCollectionTime_ - clearTime_);
    return clearInterval > maxClearInterval;
}

CollectResult<ProcessCpuStatInfo> CpuCollectorImpl::CollectProcessCpuStatInfo(int32_t pid)
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
    uint64_t lastCollectionTime = currCollectionTime_;
    uint64_t currCollectionTime = TimeUtil::GetMilliseconds();
    auto processCpuStatInfo = CalculateProcessCpuStatInfo(procCpuItem, lastCollectionTime, currCollectionTime);
    if (processCpuStatInfo.has_value()) {
        cpuCollectResult.retCode = UCollect::UcError::SUCCESS;
        cpuCollectResult.data = processCpuStatInfo.value();
    }
    return cpuCollectResult;
}
} // UCollectUtil
} // HiViewDFX
} // OHOS