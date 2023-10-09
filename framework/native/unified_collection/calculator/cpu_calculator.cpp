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
#include "cpu_calculator.h"

#include <cinttypes>

#include "file_util.h"
#include "logger.h"
#include "string_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
DEFINE_LOG_TAG("UCollectUtil-CpuCalculator");
namespace {
const std::string SYS_CPU_DIR_PREFIX = "/sys/devices/system/cpu/cpu";
}

CpuCalculator::CpuCalculator()
{
    InitNumOfCpuCores();
    InitCpuDmipses();
    InitMaxCpuLoadUnit();
}

void CpuCalculator::InitNumOfCpuCores()
{
    const std::string cpuCoresFilePath = "/sys/devices/system/cpu/possible";
    std::string cpuCoresFileFirstLine = FileUtil::GetFirstLine(cpuCoresFilePath);
    if (cpuCoresFileFirstLine.empty()) {
        HIVIEW_LOGE("failed to get cpu cores content from file=%{public}s", cpuCoresFilePath.c_str());
        return;
    } else if (cpuCoresFileFirstLine.length() == 1) { // 1: '0'
        constexpr uint32_t singleCpuCores = 1;
        numOfCpuCores_ = singleCpuCores;
    } else if (cpuCoresFileFirstLine.length() >= 3) { // '0-7' '0-11'
        constexpr uint32_t maxCoreIndex = 2; // next char of '0-'
        numOfCpuCores_ = StringUtil::StringToUl(cpuCoresFileFirstLine.substr(maxCoreIndex)) + 1; // 1 for real num
    } else {
        HIVIEW_LOGE("invalid cpu cores content=%{public}s", cpuCoresFileFirstLine.c_str());
    }
    HIVIEW_LOGI("init number of cpu cores=%{public}u", numOfCpuCores_);
}

void CpuCalculator::InitCpuDmipses()
{
    for (uint32_t i = 0; i < numOfCpuCores_; i++) {
        std::string cpuCapacityFilePath = SYS_CPU_DIR_PREFIX + std::to_string(i) + "/cpu_capacity";
        std::string cpuCapacityFileContent = FileUtil::GetFirstLine(cpuCapacityFilePath);
        if (cpuCapacityFileContent.empty()) {
            HIVIEW_LOGE("failed to get cpu capacity content from file=%{public}s", cpuCapacityFileContent.c_str());
            return;
        }
        uint32_t dmipse = StringUtil::StringToUl(cpuCapacityFileContent);
        cpuDmipses_.emplace_back(dmipse);
        HIVIEW_LOGI("get cpu=%{public}u capacity value=%{public}u", i, dmipse);
    }
}

void CpuCalculator::InitMaxCpuLoadUnit()
{
    maxCpuLoadUnit_ = IsSMTEnabled() ? GetMaxStCpuLoadWithSMT() : GetMaxStCpuLoad();
    HIVIEW_LOGI("init max cpu load unit=%{public}" PRIu64, maxCpuLoadUnit_);
}

bool CpuCalculator::IsSMTEnabled()
{
    constexpr uint32_t numOfCpuCoresWithSMT = 12;
    return numOfCpuCores_ == numOfCpuCoresWithSMT;
}

uint64_t CpuCalculator::GetMaxStCpuLoad()
{
    std::unordered_map<uint32_t, uint64_t> cpuMaxFreqs;
    GetCpuMaxFrequencies(cpuMaxFreqs);

    uint64_t maxStCpuLoadSum = 0;
    for (uint32_t cpuCoreIndex = 0; cpuCoreIndex < numOfCpuCores_; cpuCoreIndex++) {
        uint64_t perMaxStCpuLoad = 0;
        if (cpuCoreIndex >= cpuMaxFreqs.size() || cpuCoreIndex >= cpuDmipses_.size()) {
            HIVIEW_LOGW("failed to get max st load from cpu=%{public}u", cpuCoreIndex);
            continue;
        }
        perMaxStCpuLoad = cpuMaxFreqs[cpuCoreIndex] * cpuDmipses_[cpuCoreIndex];
        maxStCpuLoadSum += perMaxStCpuLoad;
    }
    return maxStCpuLoadSum;
}

uint64_t CpuCalculator::GetMaxStCpuLoadWithSMT()
{
    std::unordered_map<uint32_t, uint64_t> cpuMaxFreqs;
    GetCpuMaxFrequencies(cpuMaxFreqs);

    // When smt is enable, only physical core information needs to be collected
    const std::vector<uint32_t> cpuCoreListWithSMT = { 0, 1, 2, 3, 4, 6, 8, 10 };
    uint64_t maxStCpuLoadSum = 0;
    for (auto cpuCoreIndex : cpuCoreListWithSMT) {
        uint64_t perMaxStCpuLoad = 0;
        if (cpuCoreIndex >= cpuMaxFreqs.size() || cpuCoreIndex >= cpuDmipses_.size()) {
            HIVIEW_LOGW("failed to get max st load from cpu=%{public}u", cpuCoreIndex);
            continue;
        }
        perMaxStCpuLoad = cpuMaxFreqs[cpuCoreIndex] * cpuDmipses_[cpuCoreIndex];

        // After SMT is enabled, the dmipses calculation method of big-core and medium-core is changed
        constexpr uint32_t littleCpuCoreEndNum = 3;
        if (cpuCoreIndex > littleCpuCoreEndNum) {
            constexpr double smtCpuExpansionMultiple = 1.3;
            perMaxStCpuLoad *= smtCpuExpansionMultiple;
        }
        maxStCpuLoadSum += perMaxStCpuLoad;
    }
    return maxStCpuLoadSum;
}

void CpuCalculator::GetCpuMaxFrequencies(std::unordered_map<uint32_t, uint64_t>& cpuMaxFreqs)
{
    for (uint32_t i = 0; i < numOfCpuCores_; i++) {
        cpuMaxFreqs[i] = GetPerCpuMaxFrequency(i);
    }
}

uint64_t CpuCalculator::GetPerCpuMaxFrequency(uint32_t cpuIndex)
{
    std::string cpuFreqFilePath = SYS_CPU_DIR_PREFIX + std::to_string(cpuIndex) +
        "/cpufreq/scaling_available_frequencies";
    std::string cpuFreqFileContent = FileUtil::GetFirstLine(cpuFreqFilePath);
    if (cpuFreqFileContent.empty()) {
        HIVIEW_LOGE("failed to get cpu frequency content from file=%{public}s", cpuFreqFileContent.c_str());
        return 0;
    }
    auto cpuFreqs = StringUtil::SplitStr(cpuFreqFileContent);
    if (cpuFreqs.empty()) {
        HIVIEW_LOGE("cpu frequency content is null, from file=%{public}s", cpuFreqFileContent.c_str());
        return 0;
    }
    uint64_t maxCpuFreq = StringUtil::StringToUl(cpuFreqs.back());
    HIVIEW_LOGI("get cpu=%{public}u max frequency value=%{public}" PRIu64, cpuIndex, maxCpuFreq);
    return maxCpuFreq;
}

double CpuCalculator::CalculateCpuLoad(uint64_t currCpuLoad, uint64_t lastCpuLoad, uint64_t statPeriod)
{
    if (lastCpuLoad > currCpuLoad || statPeriod == 0) {
        HIVIEW_LOGW("invalid params, currCpuLoad=%{public}" PRIu64 ", lastCpuLoad=%{public}" PRIu64
            ", statPeriod=%{public}" PRIu64, currCpuLoad, lastCpuLoad, statPeriod);
        return 0;
    }
    if (maxCpuLoadUnit_ == 0) {
        HIVIEW_LOGW("invalid num of max cpu load unit");
        return 0;
    }
    uint64_t cpuLoadInStatPeriod = currCpuLoad - lastCpuLoad;
    uint64_t maxCpuLoadOfSystemInStatPeriod = statPeriod * maxCpuLoadUnit_;
    return ((cpuLoadInStatPeriod * 1.0) / maxCpuLoadOfSystemInStatPeriod);
}

double CpuCalculator::CalculateCpuUsage(uint64_t currCpuUsage, uint64_t lastCpuUsage, uint64_t statPeriod)
{
    if (lastCpuUsage > currCpuUsage || statPeriod == 0) {
        HIVIEW_LOGW("invalid params, currCpuUsage=%{public}" PRIu64 ", lastCpuUsage=%{public}" PRIu64
            ", statPeriod=%{public}" PRIu64, currCpuUsage, lastCpuUsage, statPeriod);
        return 0;
    }
    if (numOfCpuCores_ == 0) {
        HIVIEW_LOGW("invalid num of cpu cores");
        return 0;
    }
    uint64_t cpuUsageInStatPeriod = currCpuUsage - lastCpuUsage;
    uint64_t totalCpuUsageOfSystemInStatPeriod = statPeriod * numOfCpuCores_;
    return ((cpuUsageInStatPeriod * 1.0) / totalCpuUsageOfSystemInStatPeriod);
}

std::vector<uint32_t> CpuCalculator::GetCpuDmipses()
{
    return cpuDmipses_;
}
} // UCollectUtil
} // HiViewDFX
} // OHOS
