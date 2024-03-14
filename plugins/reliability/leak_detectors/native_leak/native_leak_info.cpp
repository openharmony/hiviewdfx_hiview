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
#include "native_leak_info.h"

#include <string>

namespace OHOS {
namespace HiviewDFX {
using std::string;
using std::vector;
using std::to_string;

NativeLeakInfo::~NativeLeakInfo() {}

time_t NativeLeakInfo::GetPidStartTime() const
{
    return pidStartTime_;
}

void NativeLeakInfo::SetPidStartTime(time_t startTime)
{
    pidStartTime_ = startTime;
}

uint64_t NativeLeakInfo::GetActualRssThreshold() const
{
    return actualRssThreshold_;
}

void NativeLeakInfo::SetActualRssThreshold(uint64_t value)
{
    actualRssThreshold_ = value;
}

time_t NativeLeakInfo::GetDebugStartTime() const
{
    return debugStartTime_;
}

void NativeLeakInfo::SetDebugStartTime(time_t value)
{
    debugStartTime_ = value;
}

string NativeLeakInfo::GetLeakGrade() const
{
    return leakGrade_;
}

void NativeLeakInfo::SetLeakGrade(string grade)
{
    leakGrade_ = grade;
}

bool NativeLeakInfo::GetJavaState() const
{
    return javaState_;
}

void NativeLeakInfo::SetJavaState(bool flag)
{
    javaState_ = flag;
}

bool NativeLeakInfo::GetInThresholdList() const
{
    return isInThresholdList_;
}

void NativeLeakInfo::SetInThresholdList(bool flag)
{
    isInThresholdList_ = flag;
}

bool NativeLeakInfo::GetIsProcessDied() const
{
    return isProcessDied_;
}

void NativeLeakInfo::SetIsProcessDied(bool flag)
{
    isProcessDied_ = flag;
}

bool NativeLeakInfo::GetIsAppendSmapsFile() const
{
    return isAppendSmapsFile_;
}

void NativeLeakInfo::SetIsAppendSmapsFile(bool flag)
{
    isAppendSmapsFile_ = flag;
}

bool NativeLeakInfo::GetIsDumpHiprofilerDrop() const
{
    return isDumpHiprofilerDrop_;
}

void NativeLeakInfo::SetIsDumpHiprofilerDrop(bool flag)
{
    isDumpHiprofilerDrop_ = flag;
}

string NativeLeakInfo::GetSampleFilePath()
{
    string path = MEMORY_LEAK_PATH + "/memleak-native-" + processName_ + "-" + to_string(pid_) + "-sample.txt";
    return path;
}

string NativeLeakInfo::GetLogFilePath()
{
    string path = MEMORY_LEAK_PATH + "/memleak-temp-" + processName_ + "-" + to_string(pid_) + "-" +
        leakedTime_ + ".txt";
    return path;
}

string NativeLeakInfo::GetRsMemPath()
{
    string path = MEMORY_LEAK_PATH + "/memleak-temp-" + processName_ + "-" + to_string(pid_) + "-rsMem.txt";
    return path;
}

string NativeLeakInfo::GetRsGpuPath()
{
    string path = MEMORY_LEAK_PATH + "/memleak-temp-" + processName_ + "-" + to_string(pid_) + "-rsGpu.txt";
    return path;
}

string NativeLeakInfo::GetSmapsPath()
{
    string path = MEMORY_LEAK_PATH + "/memleak-native-" + processName_ + "-" + to_string(pid_) + "-smaps.txt";
    return path;
}

} // namespace HiviewDFX
} // namespace OHOS
