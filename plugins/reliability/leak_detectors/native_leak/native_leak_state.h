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
#ifndef NATIVE_LEAK_STATE_H
#define NATIVE_LEAK_STATE_H

#include <fstream>
#include <memory>
#include <string>

#include "plugin.h"
#include "app_event_handler.h"
#include "app_event_publisher.h"
#include "fault_common_base.h"
#include "fault_detector_base.h"
#include "fault_info_base.h"
#include "fault_state_base.h"
#include "ffrt.h"
#include "mem_profiler_collector.h"
#include "native_leak_info.h"
#include "native_leak_util.h"

namespace OHOS {
namespace HiviewDFX {

class NativeLeakSampleState : public FaultStateBase {
public:
    void CollectBaseInfo(std::shared_ptr<NativeLeakInfo> &userMonitorInfo) const;
    ErrCode ChangeNextState(std::shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj) override;
    ErrCode StateProcess(std::shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj) override;
private:
    bool CollectUserBaseInfo(std::shared_ptr<NativeLeakInfo> &userMonitorInfo) const;
    void RemoveData(std::shared_ptr<NativeLeakInfo> &userMonitorInfo) const;
};

class NativeLeakJudgeState : public FaultStateBase {
public:
    ErrCode ChangeNextState(std::shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj) override;
    ErrCode StateProcess(std::shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj) override;

private:
    bool IsMemoryLeak(std::shared_ptr<NativeLeakInfo> &userMonitorInfo);
    std::string JudgeMemoryLeakGrade(std::shared_ptr<NativeLeakInfo> &userMonitorInfo);
    std::string JudgeSmallMemoryLeakGrade(std::shared_ptr<NativeLeakInfo> &userMonitorInfo);
    std::string JudgeMemoryLeakGradeByRatio(std::shared_ptr<NativeLeakInfo> &userMonitorInfo);
    std::string JudgeOtherMemoryLeakGrade(std::shared_ptr<NativeLeakInfo> &userMonitorInfo);
};

class NativeLeakDumpState : public FaultStateBase {
public:
    NativeLeakDumpState();
    ErrCode ChangeNextState(std::shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj) override;
    ErrCode StateProcess(std::shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj) override;
    void DumpUserMemInfo(std::shared_ptr<NativeLeakInfo> &userMonitorInfo);

private:
    void GetMemoryLeakLog(std::shared_ptr<NativeLeakInfo> &userMonitorInfo, uint32_t flag);
    void DumpGeneralInfo(std::ofstream &fout, std::shared_ptr<NativeLeakInfo> &userMonitorInfo) const;
    void DumpDetailInfo(std::ofstream &fout, std::shared_ptr<NativeLeakInfo> &userMonitorInfo);
    void DumpStackInfo(std::shared_ptr<NativeLeakInfo> &userMonitorInfo);
    void DumpExtraInfo(std::shared_ptr<NativeLeakInfo> &userMonitorInfo, uint32_t type) const;
    bool ForkProcessToDumpExtraInfo(
        const std::string &path, std::shared_ptr<NativeLeakInfo> &userMonitorInfo, uint32_t type) const;
    bool DumpUserMemInfoToSmapsFile(int writeFd, std::shared_ptr<NativeLeakInfo> &userMonitorInfo) const;
    std::string GetExtraInfo(uint32_t type) const;
    void ExecuteChildProcessGetSmapsInfo(int pid) const;
    void ExecuteChildProcessGetRsInfo() const;
    void ExecuteChildProcessGetGpuInfo() const;
    void LaunchMemoryDebug(std::shared_ptr<NativeLeakInfo> &userMonitorInfo);
    void GetProfiler(std::shared_ptr<NativeLeakInfo> &userMonitorInfo);
    bool SuccessToSendCmd(std::shared_ptr<NativeLeakInfo> &userMonitorInfo, MemCmd cmdType);
    int32_t SendCmd(std::shared_ptr<NativeLeakInfo> &userMonitorInfo, MemCmd cmdType) const;

private:
    ffrt::mutex dumpStateMtx_;
#ifdef HAS_HIPROFILER
    std::shared_ptr<UCollectUtil::MemProfilerCollector> memProfilerCollector_;
#endif
};

class NativeLeakReportState : public FaultStateBase {
public:
    ErrCode ChangeNextState(std::shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj) override;
    ErrCode StateProcess(std::shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj) override;
    void setEventHandler(std::shared_ptr<AppEventHandler> handler);
private:
    void PostEvent(std::shared_ptr<FaultInfoBase> &monitorInfo);
    std::shared_ptr<AppEventHandler> appEventHandler_ { nullptr };
};

class NativeLeakRemovalState : public FaultStateBase {
public:
    ErrCode ChangeNextState(std::shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj) override;
    ErrCode StateProcess(std::shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj) override;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // NATIVE_LEAK_STATE_H
