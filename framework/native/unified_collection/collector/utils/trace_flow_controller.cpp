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
#include <chrono>
#include <cinttypes>
#include <ctime>
#include <sys/stat.h>
#include <vector>

#include "time_util.h"
#include "trace_flow_controller.h"
#include "trace_utils.h"
#include "file_util.h"
#include "logger.h"
#include "string_util.h"
#include "app_event_task_storage.h"
#include "app_caller_event.h"

using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace HiviewDFX {
namespace {
DEFINE_LOG_TAG("UCollectUtil-TraceCollector");
const std::string UNIFIED_SHARE_PATH = "/data/log/hiview/unified_collection/trace/share/";
const std::string UNIFIED_SPECIAL_PATH = "/data/log/hiview/unified_collection/trace/special/";
const int64_t XPERF_SIZE = 1750 * 1024 * 1024;
const int64_t XPOWER_SIZE = 700 * 1024 * 1024;
const int64_t RELIABILITY_SIZE = 350 * 1024 * 1024;
const float TEN_PERCENT_LIMIT = 0.1;
}

void CreateTracePath(const std::string &filePath)
{
    if (FileUtil::FileExists(filePath)) {
        return;
    }
    if (!CreateMultiDirectory(filePath)) {
        HIVIEW_LOGE("failed to create multidirectory %{public}s.", filePath.c_str());
        return;
    }
}

void TraceFlowController::InitTraceDb()
{
    ucollectionTraceStorage_ = QueryDb();
    HIVIEW_LOGI("systemTime:%{public}s, xperfSize:%{public}" PRId64 ", xpowerSize:%{public}" PRId64
        ", reliabilitySize:%{public}" PRId64, ucollectionTraceStorage_.systemTime.c_str(),
        ucollectionTraceStorage_.xperfSize, ucollectionTraceStorage_.xpowerSize,
        ucollectionTraceStorage_.reliabilitySize);
}

void TraceFlowController::InitTraceStorage()
{
    CreateTracePath(UNIFIED_SHARE_PATH);
    CreateTracePath(UNIFIED_SPECIAL_PATH);

    traceStorage_ = std::make_shared<TraceStorage>();
}

TraceFlowController::TraceFlowController()
{
    InitTraceStorage();
    InitTraceDb();
}

bool TraceFlowController::NeedDump(TraceCollector::Caller &caller)
{
    std::string nowDays = GetDate();
    HIVIEW_LOGI("start to dump, nowDays = %{public}s, systemTime = %{public}s.",
        nowDays.c_str(), ucollectionTraceStorage_.systemTime.c_str());
    if (nowDays != ucollectionTraceStorage_.systemTime) {
        // date changes
        ucollectionTraceStorage_.systemTime = nowDays;
        ucollectionTraceStorage_.xperfSize = 0;
        ucollectionTraceStorage_.xpowerSize = 0;
        ucollectionTraceStorage_.reliabilitySize = 0;
        return true;
    }

    switch (caller) {
        case TraceCollector::Caller::RELIABILITY:
            return ucollectionTraceStorage_.reliabilitySize < RELIABILITY_SIZE;
        case TraceCollector::Caller::XPERF:
            return ucollectionTraceStorage_.xperfSize < XPERF_SIZE;
        case TraceCollector::Caller::XPOWER:
            return ucollectionTraceStorage_.xpowerSize < XPOWER_SIZE;
        default:
            return true;
    }
}

bool TraceFlowController::NeedUpload(TraceCollector::Caller &caller, TraceRetInfo ret)
{
    int64_t traceSize = GetTraceSize(ret);
    HIVIEW_LOGI("start to upload , systemTime = %{public}s, traceSize = %{public}" PRId64 ".",
        ucollectionTraceStorage_.systemTime.c_str(), traceSize);
    switch (caller) {
        case TraceCollector::Caller::RELIABILITY:
            if (IsLowerLimit(ucollectionTraceStorage_.reliabilitySize, traceSize, RELIABILITY_SIZE)) {
                ucollectionTraceStorage_.reliabilitySize += traceSize;
                return true;
            } else {
                return false;
            }
        case TraceCollector::Caller::XPERF:
            if (IsLowerLimit(ucollectionTraceStorage_.xperfSize, traceSize, XPERF_SIZE)) {
                ucollectionTraceStorage_.xperfSize += traceSize;
                return true;
            } else {
                return false;
            }
        case TraceCollector::Caller::XPOWER:
            if (IsLowerLimit(ucollectionTraceStorage_.xpowerSize, traceSize, XPOWER_SIZE)) {
                ucollectionTraceStorage_.xpowerSize += traceSize;
                return true;
            } else {
                return false;
            }
        default:
            return true;
    }
}

bool TraceFlowController::IsLowerLimit(int64_t nowSize, int64_t traceSize, int64_t limitSize)
{
    if (limitSize == 0) {
        HIVIEW_LOGE("error, limit size is zero.");
        return false;
    }

    int64_t totalSize = nowSize + traceSize;
    if (totalSize < limitSize) {
        return true;
    }

    float limit = static_cast<float>(totalSize - limitSize) / limitSize;
    if (limit > TEN_PERCENT_LIMIT) {
        return false;
    }
    return true;
}

void TraceFlowController::StoreDb()
{
    HIVIEW_LOGI("systemTime:%{public}s, xperfSize:%{public}" PRId64 ", xpowerSize:%{public}" PRId64
        ", reliabilitySize:%{public}" PRId64, ucollectionTraceStorage_.systemTime.c_str(),
        ucollectionTraceStorage_.xperfSize, ucollectionTraceStorage_.xpowerSize,
        ucollectionTraceStorage_.reliabilitySize);
    traceStorage_->Store(ucollectionTraceStorage_);
}

int64_t TraceFlowController::GetTraceSize(TraceRetInfo ret)
{
    struct stat fileInfo;
    int64_t traceSize = 0;
    for (const auto &tracePath : ret.outputFiles) {
        int ret = stat(tracePath.c_str(), &fileInfo);
        if (ret != 0) {
            HIVIEW_LOGE("%{public}s is not exists, ret = %{public}d.", tracePath.c_str(), ret);
            continue;
        }
        traceSize += fileInfo.st_size;
    }
    return traceSize;
}

std::string TraceFlowController::GetDate()
{
    std::string dateStr = TimeUtil::TimestampFormatToDate(std::time(nullptr), "%Y-%m-%d");
    return dateStr;
}

UcollectionTraceStorage TraceFlowController::QueryDb()
{
    struct UcollectionTraceStorage ucollectionTraceStorage;
    traceStorage_->Query(ucollectionTraceStorage);
    return ucollectionTraceStorage;
}

bool TraceFlowController::HasCallOnceToday(int32_t uid, uint64_t happenTime)
{
    uint64_t happenTimeInSecond = happenTime / TimeUtil::SEC_TO_MILLISEC;
    std::string date = TimeUtil::TimestampFormatToDate(happenTimeInSecond, "%Y%m%d");

    AppEventTask appEventTask;
    appEventTask.id_ = 0;
    traceStorage_->QueryAppEventTask(uid, std::stoll(date, nullptr, 0), appEventTask);
    return appEventTask.id_ > 0;
}

bool TraceFlowController::AddNewFinishTask(std::shared_ptr<AppCallerEvent> appEvent)
{
    AppEventTask appEventTask;

    uint64_t happenTimeInSecond = appEvent->happenTime_ / TimeUtil::SEC_TO_MILLISEC;
    std::string date = TimeUtil::TimestampFormatToDate(happenTimeInSecond, "%Y%m%d");
    appEventTask.taskDate_ = std::stoll(date, nullptr, 0);
    appEventTask.taskType_ = APP_EVENT_TASK_TYPE_JANK_EVENT;
    appEventTask.uid_ = appEvent->uid_;
    appEventTask.pid_ = appEvent->pid_;
    appEventTask.bundleName_ = appEvent->bundleName_;
    appEventTask.bundleVersion_ = appEvent->bundleVersion_;
    appEventTask.startTime_ = appEvent->taskBeginTime_;
    appEventTask.finishTime_ = appEvent->taskEndTime_;
    appEventTask.resourePath_ = appEvent->externalLog_;
    appEventTask.resourceSize_ = FileUtil::GetFileSize(appEventTask.resourePath_);
    appEventTask.state_ = APP_EVENT_TASK_STATE_FINISH;
    return traceStorage_->StoreAppEventTask(appEventTask);
}

void TraceFlowController::CleanAppTrace()
{
    TraceCollector::Caller caller = TraceCollector::Caller::APP;
    FileRemove(caller);
}
} // HiViewDFX
} // OHOS
