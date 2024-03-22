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
#ifndef FAULT_INFO_BASE_H
#define FAULT_INFO_BASE_H

#include <string>
#include <vector>

#include "fault_common_base.h"
#include "fault_detector_util.h"

namespace OHOS {
namespace HiviewDFX {

class FaultInfoBase {
public:
    virtual ~FaultInfoBase() {};
    uint32_t GetState() const
    {
        return state_;
    }

    void SetState(FaultStateType state)
    {
        state_ = state;
    }

    std::string GetProcessName() const
    {
        return processName_;
    }

    void SetProcessName(std::string name)
    {
        processName_ = name;
    }

    uint32_t GetPid() const
    {
        return pid_;
    }

    void SetPid(uint32_t pid)
    {
        pid_ = pid;
    }

    uint32_t IsMonitoredStat() const
    {
        return isMonitored_;
    }

    void SetIsMonitoredStat(bool isMonitored)
    {
        isMonitored_ = isMonitored;
    }

    std::string GetLeakedTime() const
    {
        return leakedTime_;
    }

    void RecordLeakedTime()
    {
        leakedTime_ = FaultDetectorUtil::GetRealTimeStampStr();
    }

    std::string GetHapVersion() const
    {
        return hapVersion_;
    }

    void SetHapVersion(std::string hapVersion)
    {
        hapVersion_ = hapVersion;
    }

protected:
    FaultStateType state_;
    std::string processName_;
    std::string leakedTime_;
    std::string hapVersion_;
    uint32_t pid_;
    bool isMonitored_ { false };
};

class MemoryLeakInfoBase : public FaultInfoBase {
public:
    virtual ~MemoryLeakInfoBase() {};
    virtual std::string GetSampleFilePath() = 0;
    virtual std::string GetRsMemPath() = 0;
    virtual std::string GetLogFilePath() = 0;

    uint64_t GetMemoryLimit() const
    {
        return memoryLimit_;
    }

    void SetMemoryLimit(uint64_t value)
    {
        memoryLimit_ = value;
    }

    const std::vector<time_t> &GetCpuTime() const
    {
        return cpuTime_;
    }

    void AddMemory(uint64_t memory)
    {
        // rename to memorySamples_
        sampleMemory_.push_back(memory);
    }

    void RemoveMemory(uint32_t removeCnt)
    {
        if (removeCnt > sampleMemory_.size()) {
            removeCnt = sampleMemory_.size();
        }
        sampleMemory_.erase(sampleMemory_.begin(), sampleMemory_.begin() + removeCnt);
    }

    const std::vector<uint64_t> &GetMemory() const
    {
        return sampleMemory_;
    }

    void AddCpuTime(time_t value)
    {
        cpuTime_.push_back(value);
    }

    const std::vector<std::string> &GetRealTime() const
    {
        return realTime_;
    }

    void AddRealTime(const std::string &value)
    {
        realTime_.push_back(value);
    }

    void RemoveTime(uint32_t removeCnt)
    {
        uint32_t cpuRemoveCnt = removeCnt;
        if (cpuRemoveCnt > cpuTime_.size()) {
            cpuRemoveCnt = cpuTime_.size();
        }
        cpuTime_.erase(cpuTime_.begin(), cpuTime_.begin() + cpuRemoveCnt);

        uint32_t realRemoveCnt = removeCnt;
        if (realRemoveCnt > realTime_.size()) {
            realRemoveCnt = realTime_.size();
        }
        realTime_.erase(realTime_.begin(), realTime_.begin() + realRemoveCnt);
    }

    uint64_t GetTopMemory() const
    {
        return topMemory_;
    }

    void SetTopMemory(uint64_t value)
    {
        topMemory_ = value;
    }

    const std::string &GetEventMsg() const
    {
        return eventMsg_;
    }

    void SetEventMsg(const std::string &msg)
    {
        eventMsg_ = msg;
    }

private:
    uint64_t memoryLimit_ {0};
    uint64_t topMemory_ {0};
    std::string eventMsg_;
    std::vector<uint64_t> sampleMemory_;
    std::vector<time_t> cpuTime_;
    std::vector<std::string> realTime_;
};
} // HiviewDFX
} // OHOS
#endif // FAULT_INFO_BASE_H
