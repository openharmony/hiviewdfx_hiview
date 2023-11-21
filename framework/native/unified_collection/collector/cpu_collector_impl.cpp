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
#include "cpu_util.h"
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
    uint32_t minFlt = 0;
    uint32_t majFlt = 0;
    uint64_t uUsageTime = 0;
    uint64_t sUsageTime = 0;
    uint64_t loadTime = 0;
    uint64_t collectionTime = 0;
};

class CpuCollectorImpl : public CpuCollector {
public:
    CpuCollectorImpl();
    virtual ~CpuCollectorImpl() = default;

public:
    virtual CollectResult<SysCpuLoad> CollectSysCpuLoad() override;
    virtual CollectResult<SysCpuUsage> CollectSysCpuUsage(bool isNeedUpdate = false) override;
    virtual CollectResult<ProcessCpuStatInfo> CollectProcessCpuStatInfo(int32_t pid) override;
    virtual CollectResult<std::vector<CpuFreq>> CollectCpuFrequency() override;
    virtual CollectResult<std::vector<ProcessCpuStatInfo>> CollectProcessCpuStatInfos(
        bool isNeedUpdate = false) override;

private:
    bool InitDeviceClient();
    void InitLastProcCpuTimeInfos();
    std::shared_ptr<ProcessCpuData> FetchProcessCpuData(int32_t pid = INVALID_PID);
    void UpdateCollectionTime();
    void UpdateLastProcCpuTimeInfo(const ucollection_process_cpu_item* procCpuItem);
    void CalculateSysCpuUsageInfos(std::vector<CpuUsageInfo>& cpuInfos,
        const std::vector<CpuUsageInfo>& currCpuInfos);
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
    uint64_t lastSysCpuUsageTime_ = 0;
    uint64_t currCollectionTime_ = 0;
    uint64_t lastCollectionTime_ = 0;
    uint64_t clearTime_ = 0;
    std::unique_ptr<CollectDeviceClient> deviceClient_;
    /* map<pid, ProcessCpuTimeInfo> */
    std::unordered_map<int32_t, ProcessCpuTimeInfo> lastProcCpuTimeInfos_;
    std::vector<CpuUsageInfo> lastSysCpuUsageInfos_;
    CpuCalculator cpuCalculator_;
};

CpuCollectorImpl::CpuCollectorImpl()
{
    if (InitDeviceClient()) {
        InitLastProcCpuTimeInfos();
    }
    UpdateClearTime();

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
    lastProcCpuTimeInfos_[procCpuItem->pid] = {
        .minFlt = procCpuItem->min_flt,
        .majFlt = procCpuItem->maj_flt,
        .uUsageTime = procCpuItem->cpu_usage_utime,
        .sUsageTime = procCpuItem->cpu_usage_stime,
        .loadTime = procCpuItem->cpu_load_time,
        .collectionTime = currCollectionTime_,
    };
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
    result.retCode = CpuUtil::GetSysCpuLoad(result.data);
    return result;
}

CollectResult<SysCpuUsage> CpuCollectorImpl::CollectSysCpuUsage(bool isNeedUpdate)
{
    CollectResult<SysCpuUsage> result;
    std::vector<CpuUsageInfo> currCpuInfos;
    result.retCode = CpuUtil::GetCpuUsageInfos(currCpuInfos);
    if (result.retCode != UCollect::UcError::SUCCESS) {
        return result;
    }

    SysCpuUsage& sysCpuUsage = result.data;
    sysCpuUsage.startTime = lastSysCpuUsageTime_;
    sysCpuUsage.endTime = TimeUtil::GetMilliseconds();
    CalculateSysCpuUsageInfos(sysCpuUsage.cpuInfos, currCpuInfos);

    if (isNeedUpdate) {
        lastSysCpuUsageTime_ = sysCpuUsage.endTime;
        lastSysCpuUsageInfos_ = currCpuInfos;
    }
    HIVIEW_LOGD("collect system cpu usage, size=%{public}zu, isNeedUpdate=%{public}d",
        sysCpuUsage.cpuInfos.size(), isNeedUpdate);
    return result;
}

void CpuCollectorImpl::CalculateSysCpuUsageInfos(std::vector<CpuUsageInfo>& cpuInfos,
    const std::vector<CpuUsageInfo>& currCpuInfos)
{
    if (currCpuInfos.size() != lastSysCpuUsageInfos_.size()) {
        return;
    }

    for (size_t i = 0; i < currCpuInfos.size(); ++i) {
        cpuInfos.emplace_back(cpuCalculator_.CalculateSysCpuUsageInfo(currCpuInfos[i], lastSysCpuUsageInfos_[i]));
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
    if (isNeedUpdate) {
        UpdateCollectionTime();
    }
    CalculateProcessCpuStatInfos(cpuCollectResult.data, processCpuData, isNeedUpdate);
    HIVIEW_LOGD("collect process cpu statistics information size=%{public}zu, isNeedUpdate=%{public}d",
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
    uint64_t lastCollectionTime = isNeedUpdate ? lastCollectionTime_ : currCollectionTime_;
    uint64_t currCollectionTime = isNeedUpdate ? currCollectionTime_ : TimeUtil::GetMilliseconds();
    auto procCpuItem = processCpuData->GetNextProcess();
    while (procCpuItem != nullptr) {
        auto processCpuStatInfo = CalculateProcessCpuStatInfo(procCpuItem, lastCollectionTime, currCollectionTime);
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
    processCpuStatInfo.minFlt = procCpuItem->min_flt;
    processCpuStatInfo.majFlt = procCpuItem->maj_flt;
    processCpuStatInfo.procName = ProcessStatus::GetInstance().GetProcessName(procCpuItem->pid);
    uint64_t collectionPeriod = endTime > startTime ? (endTime - startTime) : 0;
    processCpuStatInfo.cpuLoad = cpuCalculator_.CalculateCpuLoad(procCpuItem->cpu_load_time,
        lastProcCpuTimeInfos_[procCpuItem->pid].loadTime, collectionPeriod);
    processCpuStatInfo.uCpuUsage = cpuCalculator_.CalculateCpuUsage(procCpuItem->cpu_usage_utime,
        lastProcCpuTimeInfos_[procCpuItem->pid].uUsageTime, collectionPeriod);
    processCpuStatInfo.sCpuUsage = cpuCalculator_.CalculateCpuUsage(procCpuItem->cpu_usage_stime,
        lastProcCpuTimeInfos_[procCpuItem->pid].sUsageTime, collectionPeriod);
    processCpuStatInfo.cpuUsage = processCpuStatInfo.uCpuUsage + processCpuStatInfo.sCpuUsage;
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