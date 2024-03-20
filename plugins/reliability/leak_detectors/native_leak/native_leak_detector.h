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
#ifndef NATIVE_LEAK_DETECTOR_H
#define NATIVE_LEAK_DETECTOR_H

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "fault_common_base.h"
#include "fault_detector_base.h"
#include "fault_info_base.h"
#include "fault_state_base.h"
#include "ffrt.h"
#include "singleton.h"

namespace OHOS {
namespace HiviewDFX {
class NativeLeakDetector : public FaultDetectorBase, public Singleton<NativeLeakDetector> {
    DECLARE_SINGLETON(NativeLeakDetector);

public:
    void PrepareNativeLeakEnv();
    void ProcessUserEvent(const std::string &name, const std::string &msg, uint32_t pid);
    ErrCode MainProcess() override;
    void OnChangeState(std::shared_ptr<FaultInfoBase> &monitorInfo, FaultStateType stateType) override;
    ErrCode ExeNextStateProcess(std::shared_ptr<FaultInfoBase> monitorInfo, FaultStateType stateType) override;
    FaultStateBase* GetStateObj(FaultStateType stateType) override;

private:
    void NativeLeakConfigParse();
    void InitMonitorInfo();
    void UpdateUserMonitorInfo();
    void RecordNativeInfo();
    void RemoveInvalidLeakedPid();
    void RemoveInvalidUserInfo();
    void UpdateProcessedPidsList();
    void RemoveFinishedInfo(int64_t pid);
    void DoProcessNativeLeak();
    bool JudgeNativeLeak(std::shared_ptr<FaultInfoBase> &monitorInfo);
    void AddMonitorToList(std::shared_ptr<FaultInfoBase> &monitorInfo);

private:
    uint32_t loopCnt_ { 0 };
    uint32_t funcLoopCnt_ { 0 };
    uint32_t sampleInterval_ { 0 };
    uint32_t updateInterval_ { 0 };
    uint64_t defauleThreshold_ { 0 };

    ffrt::mutex nativeDetectorMtx_;
    std::unordered_map<std::string, uint64_t> thresholdLists_;
    std::map<pid_t, std::shared_ptr<FaultInfoBase>> grayList_;
    std::map<pid_t, std::string> monitoredPidsList_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // NATIVE_LEAK_DETECTOR_H
