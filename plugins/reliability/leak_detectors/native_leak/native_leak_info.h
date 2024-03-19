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
#ifndef NATIVE_LEAK_INFO_H
#define NATIVE_LEAK_INFO_H

#include <string>
#include "fault_info_base.h"

namespace OHOS {
namespace HiviewDFX {

enum {
    GENERAL_STATISTICS = 1,
    ACCURATE_STATISTICS = 1 << 1,
    STACK_STATISTICS = 1 << 2
};

class NativeLeakInfo : public MemoryLeakInfoBase {
public:
    ~NativeLeakInfo();

    std::string GetSampleFilePath() override;
    std::string GetLogFilePath() override;
    std::string GetRsMemPath() override;
    time_t GetPidStartTime() const;
    void SetPidStartTime(time_t startTime);
    time_t GetDebugStartTime() const;
    void SetDebugStartTime(time_t value);
    uint64_t GetActualRssThreshold() const;
    void SetActualRssThreshold(uint64_t value);
    std::string GetLeakGrade() const;
    void SetLeakGrade(const std::string &grade);
    bool GetInThresholdList() const;
    void SetInThresholdList(bool flag);
    bool GetJavaState() const;
    void SetJavaState(bool flag);
    bool GetIsProcessDied() const;
    void SetIsProcessDied(bool flag);
    bool GetIsAppendSmapsFile() const;
    void SetIsAppendSmapsFile(bool flag);
    bool GetIsDumpHiprofilerDrop() const;
    void SetIsDumpHiprofilerDrop(bool flag);
    std::string GetRsGpuPath();
    std::string GetSmapsPath();

private:
    time_t pidStartTime_ { 0 };
    time_t debugStartTime_ { 0 };
    uint64_t actualRssThreshold_ { 0 };
    bool javaState_ { false };
    bool isInThresholdList_ { false };
    bool isProcessDied_ { false };
    bool isDumpHiprofilerDrop_ { false };
    bool isAppendSmapsFile_ { false };
    std::string leakGrade_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // NATIVE_LEAK_INFO_H
