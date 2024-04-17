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

#ifndef HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_CPU_DECORATOR_H
#define HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_CPU_DECORATOR_H

#include "cpu_collector.h"
#include "decorator.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
class CpuDecorator : public CpuCollector, public ThreadCpuCollector, public UCDecorator {
public:
    CpuDecorator(std::shared_ptr<CpuCollector> collector,
        std::shared_ptr<ThreadCpuCollector> threadCpuCollector = nullptr) : cpuCollector_(collector),
            threadCpuCollector_(threadCpuCollector) {};
    virtual ~CpuDecorator() = default;

public:
    virtual CollectResult<SysCpuLoad> CollectSysCpuLoad() override;
    virtual CollectResult<SysCpuUsage> CollectSysCpuUsage(bool isNeedUpdate = false) override;
    virtual CollectResult<double> GetSysCpuUsage() override;
    virtual CollectResult<ProcessCpuStatInfo> CollectProcessCpuStatInfo(int32_t pid,
        bool isNeedUpdate = false) override;
    virtual CollectResult<std::vector<CpuFreq>> CollectCpuFrequency() override;
    virtual CollectResult<std::vector<ProcessCpuStatInfo>> CollectProcessCpuStatInfos(
        bool isNeedUpdate = false) override;
    virtual std::shared_ptr<ThreadCpuCollector> CreateThreadCollector(int pid) override;
    virtual CollectResult<std::vector<ThreadCpuStatInfo>> CollectThreadStatInfos(bool isNeedUpdate = false) override;
    virtual int GetCollectPid() override;
    static void SaveStatCommonInfo();
    static void ResetStatInfo();

private:
    std::shared_ptr<CpuCollector> cpuCollector_;
    std::shared_ptr<ThreadCpuCollector> threadCpuCollector_;
    static StatInfoWrapper statInfoWrapper_;
};
} // namespace UCollectUtil
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_CPU_DECORATOR_H
