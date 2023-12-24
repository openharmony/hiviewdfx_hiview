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

#include "mem_profiler_collector_impl.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
const std::string MEM_PROFILER_COLLECTOR_NAME = "MemProfilerCollector";
StatInfoWrapper MemProfilerDecorator::statInfoWrapper_;

std::shared_ptr<MemProfilerCollector> MemProfilerCollector::Create()
{
    static std::shared_ptr<MemProfilerDecorator> instance_ = std::make_shared<MemProfilerDecorator>();
    return instance_;
}

MemProfilerDecorator::MemProfilerDecorator()
{
    memProfilerCollector_ = std::make_shared<MemProfilerCollectorImpl>();
}

int MemProfilerDecorator::Start(ProfilerType type, int pid, int duration, int sampleInterval)
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    int result = memProfilerCollector_->Start(type, pid, duration, sampleInterval);
    // has same func name, rename it with num "-1"
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = MEM_PROFILER_COLLECTOR_NAME + UC_SEPARATOR + __func__ + "-1";
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result == 0);
    return result;
}

int MemProfilerDecorator::Stop(int pid)
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    int result = memProfilerCollector_->Stop(pid);
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = MEM_PROFILER_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result == 0);
    return result;
}

int MemProfilerDecorator::Start(int fd, ProfilerType type, int pid, int duration, int sampleInterval)
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    int result = memProfilerCollector_->Start(fd, type, pid, duration, sampleInterval);
    // has same func name, rename it with num "-2"
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = MEM_PROFILER_COLLECTOR_NAME + UC_SEPARATOR + __func__ + "-2";
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result == 0);
    return result;
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
