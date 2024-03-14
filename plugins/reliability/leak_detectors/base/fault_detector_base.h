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
#ifndef FAULT_DETECTOR_BASE_H
#define FAULT_DETECTOR_BASE_H

#include <map>
#include <memory>
#include <unordered_map>

#include "fault_common_base.h"
#include "fault_info_base.h"
#include "fault_state_base.h"

namespace OHOS {
namespace HiviewDFX {
class FaultStateBase;

class FaultDetectorBase {
public:
    FaultDetectorBase() {};
    virtual ~FaultDetectorBase() {};
    virtual FaultStateBase* GetStateObj(FaultStateType stateType) = 0;
    virtual ErrCode MainProcess() = 0;
    virtual void OnChangeState(std::shared_ptr<FaultInfoBase> &monitorInfo, FaultStateType stateType) = 0;
    virtual ErrCode ExeNextStateProcess(std::shared_ptr<FaultInfoBase> monitorInfo, FaultStateType stateType) = 0;

protected:
    std::unordered_map<int64_t, std::string> monitoredPidList_;
    std::unordered_map<int64_t, std::shared_ptr<FaultInfoBase>> monitoredPidsInfo_;
    std::map<std::string, int64_t> processedPids_;
};
} // HiviewDFX
} // OHOS

#endif // FAULT_DETECTOR_BASE_H
