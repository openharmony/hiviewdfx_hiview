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
#ifndef FAULT_COMMON_BASE_H
#define FAULT_COMMON_BASE_H

#include <string>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
#define MAX_MONITOR_PROCESS_CNT 10
typedef enum FaultLevel {
    Normal = 0,
    Minor,
    Serious,
    Critical,
    CriticalDfx,
    AllFaultLevel
} FaultLevel;

typedef enum ErrCode {
    SUCCESSED = 0,
    FAILURE
} ErrorCode;

enum DetectorType {
    NATIVE_LEAK_DETECTOR,
    DETECTOR_SIZE
};

const std::vector<std::string> DetectorTypeName = {
    "native_leak_detector"
};

enum FaultStateType {
    PROC_IDLE_STATE = 0,
    PROC_SAMPLE_STATE,
    PROC_JUDGE_STATE,
    PROC_DUMP_STATE,
    PROC_REPORT_STATE,
    PROC_REMOVAL_STATE,
    PROC_FINISHED_STATE
};

const std::string FaultStateName[] = {
    "[ProcIdleState]",
    "[ProcSampleState]",
    "[ProcJudgeState]",
    "[ProcDumpState]",
    "[ProcReportState]",
    "[ProcRemovalState]",
    "[ProcFinishedState]"
};

} // namespace HiviewDFX
} // namespace OHOS
#endif // FAULT_COMMON_BASE_H
