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

#include "trace_collector_impl.h"

#include <memory>
#include <mutex>
#include <string>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

#include "hiview_logger.h"
#include "trace_decorator.h"
#include "trace_strategy.h"
#include "trace_state_machine.h"
#include "trace_utils.h"
#include "trace_strategy_factory.h"
#include "hiview_zip_util.h"
#include "trace_worker.h"
#include "hiview_event_report.h"

using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace OHOS::HiviewDFX::UCollect;

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
namespace {
DEFINE_LOG_TAG("UCollectUtil-TraceCollector");
constexpr uid_t HIVIEW_UID = 1201;
const int64_t MS_TO_US = 1000;

void ZipShareTempTraceFile(const std::string &srcFile)
{
    std::string tempZipFile = UNIFIED_SHARE_PATH + StringUtil::ReplaceStr(FileUtil::ExtractFileName(srcFile),
        ".sys", ".zip");
    std::string zipTraceFile = AddVersionInfoToZipName(tempZipFile);
    if (FileUtil::FileExists(zipTraceFile)) {
        HIVIEW_LOGI("dst: %{public}s already exist", zipTraceFile.c_str());
        return;
    }
    CheckCurrentCpuLoad();
    HiviewEventReport::ReportCpuScene("5");
    HiviewZipUnit zipUnit(zipTraceFile);
    if (int32_t ret = zipUnit.AddFileInZip(srcFile, ZipFileLevel::KEEP_NONE_PARENT_PATH); ret != 0) {
        HIVIEW_LOGE("zip trace failed, ret: %{public}d.", ret);
        return;
    }
    HIVIEW_LOGI("ZipTraceFile success: %{public}s", FileUtil::ExtractFileName(zipTraceFile).c_str());
}
}
std::shared_ptr<TraceCollector> TraceCollector::Create()
{
    static std::shared_ptr<TraceCollector> instance_ =
        std::make_shared<TraceDecorator>(std::make_shared<TraceCollectorImpl>());
    return instance_;
}

CollectResult<std::vector<std::string>> TraceCollectorImpl::DumpTraceWithDuration(
    TraceCaller caller, uint32_t maxDuration, uint64_t happenTime)
{
    if (maxDuration > INT32_MAX) {
        return StartDumpTrace(caller, INT32_MAX, happenTime);
    }
    return StartDumpTrace(caller, maxDuration, happenTime);
}

CollectResult<std::vector<std::string>> TraceCollectorImpl::DumpTraceWithFilter(TeleModule module,
    uint32_t maxDuration, uint64_t happenTime)
{
    if (auto uid = getuid(); uid != HIVIEW_UID) {
        HIVIEW_LOGE("Do not allow uid:%{public}d to dump trace except in hiview process", uid);
        return {UcError::PERMISSION_CHECK_FAILED};
    }
    CollectResult<std::vector<std::string>> result;
    auto strategy = std::make_shared<TelemetryStrategy>(
        StrategyParam {maxDuration, happenTime, ModuleToString(module)},
            std::make_shared<TraceZipHandler>(UNIFIED_TELEMETRY_PATH, BusinessName::TELEMETRY));
    TraceRet ret = strategy->DoDump(result.data);
    result.retCode = GetUcError(ret);
    HIVIEW_LOGI("caller:%{public}s retCode = %{public}d, file number = %{public}zu.", ModuleToString(module).c_str(),
        result.retCode, result.data.size());
    return result;
}

CollectResult<int32_t> TraceCollectorImpl::FilterTraceOn(TeleModule module, uint64_t postTime)
{
    if (auto uid = getuid(); uid != HIVIEW_UID) {
        HIVIEW_LOGE("Do not allow uid:%{public}d to invoke in hiview process", uid);
        return {UcError::PERMISSION_CHECK_FAILED};
    }
    if (postTime == 0) {
        HIVIEW_LOGI("trace on module:%{public}d", static_cast<int32_t>(module));
        return GetUcError(TraceStateMachine::GetInstance().TraceTelemetryOn());
    }
    auto ret = TraceStateMachine::GetInstance().PostTelemetryOn(postTime);
    if (ret.GetStateError() != TraceStateCode::UPDATE_TIME) {
        HIVIEW_LOGI("module:%{public}d post on not update timer", static_cast<int32_t>(module));
        return GetUcError(ret);
    }
    std::lock_guard<std::mutex> lock(postMutex_);
    if (ffrtQueue_ == nullptr) {
        ffrtQueue_ = std::make_unique<ffrt::queue>("telemetry_post_on");
    }
    if (ffrtQueue_->cancel(handle_) < 0) {
        HIVIEW_LOGW("no task to cancel");
    }
    HIVIEW_LOGI("post a timeout task to ffrt delay:%{public} " PRId64 ", module:%{public}d", postTime,
        static_cast<int32_t>(module));
    handle_ = ffrtQueue_->submit_h([]() {
        TraceStateMachine::GetInstance().PostTelemetryTimeOut();
    }, ffrt::task_attr().name("post_trace_on").delay(postTime * MS_TO_US));
    return GetUcError(ret);
}

CollectResult<int32_t> TraceCollectorImpl::FilterTraceOff(TeleModule module)
{
    if (auto uid = getuid(); uid != HIVIEW_UID) {
        HIVIEW_LOGE("Do not allow uid:%{public}d to dump trace except in hiview process", uid);
        return {UcError::PERMISSION_CHECK_FAILED};
    }
    HIVIEW_LOGI("module:%{public}d", static_cast<int32_t>(module));
    return GetUcError(TraceStateMachine::GetInstance().TraceTelemetryOff());
}

CollectResult<std::vector<std::string>> TraceCollectorImpl::DumpTrace(TraceCaller caller)
{
    return StartDumpTrace(caller, 0, static_cast<uint64_t>(0));
}

CollectResult<std::vector<std::string>> TraceCollectorImpl::StartDumpTrace(TraceCaller &caller, uint32_t timeLimit,
    uint64_t happenTime)
{
    if (auto uid = getuid(); uid != HIVIEW_UID) {
        HIVIEW_LOGE("Do not allow uid:%{public}d to dump trace except in hiview process", uid);
        return {UcError::PERMISSION_CHECK_FAILED};
    }
    CollectResult<std::vector<std::string>> result;
    auto strategy = TraceStrategyFactory::CreateTraceStrategy(caller, timeLimit, happenTime);
    if (strategy == nullptr) {
        HIVIEW_LOGE("Create traceStrategy error caller:%{public}d", caller);
        result.retCode = UcError::UNSUPPORT;
        return result;
    }
    TraceRet ret = strategy->DoDump(result.data);
    result.retCode = GetUcError(ret);
    HIVIEW_LOGI("caller:%{public}s, retCode = %{public}d, data.size = %{public}zu.", EnumToString(caller).c_str(),
        result.retCode, result.data.size());
    return result;
}

void TraceCollectorImpl::RecoverTmpTrace()
{
    if (auto uid = getuid(); uid != HIVIEW_UID) {
        HIVIEW_LOGE("Do not allow uid:%{public}d to RecoverTmpTrace trace except in hiview process", uid);
        return;
    }
    std::vector<std::string> traceFiles;
    FileUtil::GetDirFiles(UNIFIED_SHARE_TEMP_PATH, traceFiles, false);
    HIVIEW_LOGI("traceFiles need recover: %{public}zu", traceFiles.size());
    for (auto &filePath : traceFiles) {
        std::string fileName = FileUtil::ExtractFileName(filePath);
        HIVIEW_LOGI("unfinished trace file: %{public}s", fileName.c_str());
        std::string originTraceFile = StringUtil::ReplaceStr("/data/log/hitrace/" + fileName, ".zip", ".sys");
        if (!FileUtil::FileExists(originTraceFile)) {
            HIVIEW_LOGI("source file not exist: %{public}s", originTraceFile.c_str());
            FileUtil::RemoveFile(UNIFIED_SHARE_TEMP_PATH + fileName);
            continue;
        }
        int fd = open(originTraceFile.c_str(), O_RDONLY | O_NONBLOCK);
        if (fd == -1) {
            HIVIEW_LOGI("open source file failed: %{public}s", originTraceFile.c_str());
            continue;
        }
        fdsan_exchange_owner_tag(fd, 0, logLabelDomain);
        // add lock before zip trace file, in case hitrace delete origin trace file.
        if (flock(fd, LOCK_EX | LOCK_NB) < 0) {
            HIVIEW_LOGI("get source file lock failed: %{public}s", originTraceFile.c_str());
            fdsan_close_with_tag(fd, logLabelDomain);
            continue;
        }
        HIVIEW_LOGI("originTraceFile path: %{public}s", originTraceFile.c_str());
        UcollectionTask traceTask = [=]() {
            ZipShareTempTraceFile(originTraceFile);
            flock(fd, LOCK_UN);
            fdsan_close_with_tag(fd, logLabelDomain);
        };
        TraceWorker::GetInstance().HandleUcollectionTask(traceTask);
    }
}
} // UCollectUtil
} // HiViewDFX
} // OHOS
