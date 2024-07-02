/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
    auto task = [this] { return memProfilerCollector_->Prepare(); };
    return Invoke(task, statInfoWrapper_, MEM_PROFILER_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

int MemProfilerDecorator::Start(ProfilerType type, int pid, int duration, int sampleInterval)
{
    auto task = [this, &type, &pid, &duration, &sampleInterval] {
        return memProfilerCollector_->Start(type, pid, duration, sampleInterval);
    };
    // has same func name, rename it with num "-1"
    return Invoke(task, statInfoWrapper_, MEM_PROFILER_COLLECTOR_NAME + UC_SEPARATOR + __func__ + "-1");
}

int MemProfilerDecorator::Stop(int pid)
{
    auto task = [this, &pid] { return memProfilerCollector_->Stop(pid); };
    // has same func name, rename it with num "-1"
    return Invoke(task, statInfoWrapper_, MEM_PROFILER_COLLECTOR_NAME + UC_SEPARATOR + __func__ + "-1");
}

int MemProfilerDecorator::Stop(const std::string& processName)
{
    auto task = [this, &processName] { return memProfilerCollector_->Stop(processName); };
    // has same func name, rename it with num "-1"
    return Invoke(task, statInfoWrapper_, MEM_PROFILER_COLLECTOR_NAME + UC_SEPARATOR + __func__ + "-2");
}

int MemProfilerDecorator::Start(int fd, ProfilerType type, int pid, int duration, int sampleInterval)
{
    auto task = [this, &fd, &type, &pid, &duration, &sampleInterval] {
        return memProfilerCollector_->Start(fd, type, pid, duration, sampleInterval);
    };
    // has same func name, rename it with num "-2"
    return Invoke(task, statInfoWrapper_, MEM_PROFILER_COLLECTOR_NAME + UC_SEPARATOR + __func__ + "-2");
}

int MemProfilerDecorator::StartPrintNmd(int fd, int pid, int type)
{
    auto task = [this, &fd, &pid, &type] { return memProfilerCollector_->StartPrintNmd(fd, pid, type); };
    return Invoke(task, statInfoWrapper_, MEM_PROFILER_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

int MemProfilerDecorator::Start(int fd, ProfilerType type, std::string processName, int duration,
                                int sampleInterval, bool startup)
{
    auto task = [this, &fd, &type, &processName, &duration, &sampleInterval, &startup] {
        return memProfilerCollector_->Start(fd, type, processName, duration, sampleInterval, startup);
    };
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
