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

#include "io_decorator.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
const std::string IO_COLLECTOR_NAME = "IoCollector";
StatInfoWrapper IoDecorator::statInfoWrapper_;

CollectResult<ProcessIo> IoDecorator::CollectProcessIo(int32_t pid)
{
    auto task = std::bind(&IoCollector::CollectProcessIo, ioCollector_.get(), pid);
    return Invoke(task, statInfoWrapper_, IO_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::string> IoDecorator::CollectRawDiskStats()
{
    auto task = std::bind(&IoCollector::CollectRawDiskStats, ioCollector_.get());
    return Invoke(task, statInfoWrapper_, IO_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::vector<DiskStats>> IoDecorator::CollectDiskStats(
    DiskStatsFilter filter, bool isUpdate)
{
    auto task = std::bind(&IoCollector::CollectDiskStats, ioCollector_.get(), filter, isUpdate);
    return Invoke(task, statInfoWrapper_, IO_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::string> IoDecorator::ExportDiskStats(DiskStatsFilter filter)
{
    auto task = std::bind(&IoCollector::ExportDiskStats, ioCollector_.get(), filter);
    return Invoke(task, statInfoWrapper_, IO_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::vector<EMMCInfo>> IoDecorator::CollectEMMCInfo()
{
    auto task = std::bind(&IoCollector::CollectEMMCInfo, ioCollector_.get());
    return Invoke(task, statInfoWrapper_, IO_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::string> IoDecorator::ExportEMMCInfo()
{
    auto task = std::bind(&IoCollector::ExportEMMCInfo, ioCollector_.get());
    return Invoke(task, statInfoWrapper_, IO_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::vector<ProcessIoStats>> IoDecorator::CollectAllProcIoStats(bool isUpdate)
{
    auto task = std::bind(&IoCollector::CollectAllProcIoStats, ioCollector_.get(), isUpdate);
    return Invoke(task, statInfoWrapper_, IO_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::string> IoDecorator::ExportAllProcIoStats()
{
    auto task = std::bind(&IoCollector::ExportAllProcIoStats, ioCollector_.get());
    return Invoke(task, statInfoWrapper_, IO_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<SysIoStats> IoDecorator::CollectSysIoStats()
{
    auto task = std::bind(&IoCollector::CollectSysIoStats, ioCollector_.get());
    return Invoke(task, statInfoWrapper_, IO_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

CollectResult<std::string> IoDecorator::ExportSysIoStats()
{
    auto task = std::bind(&IoCollector::ExportSysIoStats, ioCollector_.get());
    return Invoke(task, statInfoWrapper_, IO_COLLECTOR_NAME + UC_SEPARATOR + __func__);
}

void IoDecorator::SaveStatCommonInfo()
{
    std::map<std::string, StatInfo> statInfo = statInfoWrapper_.GetStatInfo();
    std::vector<std::string> formattedStatInfo;
    for (const auto& record : statInfo) {
        formattedStatInfo.push_back(record.second.ToString());
    }
    WriteLinesToFile(formattedStatInfo, false);
}

void IoDecorator::ResetStatInfo()
{
    statInfoWrapper_.ResetStatInfo();
}
} // namespace UCollectUtil
} // namespace HiviewDFX
} // namespace OHOS
