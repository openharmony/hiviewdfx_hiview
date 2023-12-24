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

#include "io_collector_impl.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
const std::string IO_COLLECTOR_NAME = "IoCollector";
StatInfoWrapper IoDecorator::statInfoWrapper_;

std::shared_ptr<IoCollector> IoCollector::Create()
{
    static std::shared_ptr<IoDecorator> instance_ = std::make_shared<IoDecorator>();
    return instance_;
}

IoDecorator::IoDecorator()
{
    ioCollector_ = std::make_shared<IoCollectorImpl>();
}

CollectResult<ProcessIo> IoDecorator::CollectProcessIo(int32_t pid)
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<ProcessIo> result = ioCollector_->CollectProcessIo(pid);
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = IO_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::string> IoDecorator::CollectRawDiskStats()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::string> result = ioCollector_->CollectRawDiskStats();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = IO_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::vector<DiskStats>> IoDecorator::CollectDiskStats(
    DiskStatsFilter filter, bool isUpdate)
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::vector<DiskStats>> result = ioCollector_->CollectDiskStats(filter, isUpdate);
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = IO_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::string> IoDecorator::ExportDiskStats(DiskStatsFilter filter)
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::string> result = ioCollector_->ExportDiskStats(filter);
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = IO_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::vector<EMMCInfo>> IoDecorator::CollectEMMCInfo()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::vector<EMMCInfo>> result = ioCollector_->CollectEMMCInfo();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = IO_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::string> IoDecorator::ExportEMMCInfo()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::string> result = ioCollector_->ExportEMMCInfo();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = IO_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::vector<ProcessIoStats>> IoDecorator::CollectAllProcIoStats(bool isUpdate)
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::vector<ProcessIoStats>> result = ioCollector_->CollectAllProcIoStats(isUpdate);
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = IO_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::string> IoDecorator::ExportAllProcIoStats()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::string> result = ioCollector_->ExportAllProcIoStats();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = IO_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<SysIoStats> IoDecorator::CollectSysIoStats()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<SysIoStats> result = ioCollector_->CollectSysIoStats();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = IO_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::string> IoDecorator::ExportSysIoStats()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::string> result = ioCollector_->ExportSysIoStats();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = IO_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
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
