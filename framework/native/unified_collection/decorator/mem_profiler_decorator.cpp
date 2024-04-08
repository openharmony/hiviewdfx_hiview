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

#include "mem_profiler_decorator.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
const std::string MEM_PROFILER_COLLECTOR_NAME = "MemProfilerCollector";
StatInfoWrapper MemProfilerDecorator::statInfoWrapper_;

int MemProfilerDecorator::Prepare()
{
    auto task = std::bind(&MemProfilerCollector::Prepare, memProfilerCollector_.get());
    return Invoke(task, statInfoWrapper_, MEM_PROFILER_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

int MemProfilerDecorator::Start(ProfilerType type, int pid, int duration, int sampleInterval)
{
    auto task = std::bind(
        static_cast<int(MemProfilerCollector::*)(ProfilerType, int, int, int)>(&MemProfilerCollector::Start),
        memProfilerCollector_.get(), type, pid, duration, sampleInterval);
    // has same func name, rename it with num "-1"
    return Invoke(task, statInfoWrapper_, MEM_PROFILER_COLLECTOR_NAME + UC_SEPARATOR + __func__ + "-1");
}

int MemProfilerDecorator::Stop(int pid)
{
    auto task = std::bind(&MemProfilerCollector::Stop, memProfilerCollector_.get(), pid);
    return Invoke(task, statInfoWrapper_, MEM_PROFILER_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

int MemProfilerDecorator::Start(int fd, ProfilerType type, int pid, int duration, int sampleInterval)
{
    auto task = std::bind(
        static_cast<int(MemProfilerCollector::*)(int, ProfilerType, int, int, int)>(&MemProfilerCollector::Start),
        memProfilerCollector_.get(), fd, type, pid, duration, sampleInterval);
    // has same func name, rename it with num "-2"
    return Invoke(task, statInfoWrapper_, MEM_PROFILER_COLLECTOR_NAME + UC_SEPARATOR + __func__ + "-2");
}

int MemProfilerDecorator::StartPrintNmd(int fd, int pid, int type)
{
    auto task = std::bind(&MemProfilerCollector::StartPrintNmd, memProfilerCollector_.get(), fd, pid, type);
    return Invoke(task, statInfoWrapper_, MEM_PROFILER_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

int MemProfilerDecorator::Start(int fd, ProfilerType type, std::string processName, int duration,
                                int sampleInterval, bool startup)
{
    auto task = std::bind(
        static_cast<int(MemProfilerCollector::*)(int, ProfilerType,
                                                 std::string, int, int, bool)>(&MemProfilerCollector::Start),
        memProfilerCollector_.get(), fd, type, processName, duration, sampleInterval, startup);
    // has same func name, rename it with num "-2"
    return Invoke(task, statInfoWrapper_, MEM_PROFILER_COLLECTOR_NAME + UC_SEPARATOR + __func__ + "-3");
}

void MemProfilerDecorator::SaveStatCommonInfo()
{
    std::map<std::string, StatInfo> statInfo = statInfoWrapper_.GetStatInfo();
    std::vector<std::string> formattedStatInfo;
    for (const auto& record : statInfo) {
        formattedStatInfo.push_back(record.second.ToString());
    }
    WriteLinesToFile(formattedStatInfo, false);
}

void MemProfilerDecorator::ResetStatInfo()
{
    statInfoWrapper_.ResetStatInfo();
}
} // namespace UCollectUtil
} // namespace HiviewDFX
} // namespace OHOS
