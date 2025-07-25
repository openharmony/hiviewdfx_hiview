/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include <charconv>
#include <sys/stat.h>
#include <set>

#include "app_caller_event.h"
#include "app_event_task_storage.h"
#include "file_util.h"
#include "hiview_logger.h"
#include "time_util.h"
#include "trace_common.h"
#include "trace_flow_controller.h"
#include "trace_db_callback.h"

using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace HiviewDFX {
namespace {
DEFINE_LOG_TAG("TraceFlowController");
constexpr int32_t DB_VERSION = 5;
const std::string DB_NAME = "trace_flow_control.db";
constexpr int32_t HITRACE_CACHE_DURATION_LIMIT_DAILY_TOTAL = 10 * 60; // 10 minutes
const std::set<std::string> DB_CALLER {
    CallerName::XPERF, CallerName::XPOWER, CallerName::RELIABILITY, CallerName::HIVIEW
};
}

void TraceFlowController::InitTraceDb(const std::string& dbPath)
{
    NativeRdb::RdbStoreConfig config(dbPath + DB_NAME);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    TraceDbStoreCallback callback;
    auto ret = NativeRdb::E_OK;
    dbStore_ = NativeRdb::RdbHelper::GetRdbStore(config, DB_VERSION, callback, ret);
    if (ret != NativeRdb::E_OK) {
        HIVIEW_LOGE("failed to init db store, db store path=%{public}s", dbPath.c_str());
        dbStore_ = nullptr;
        return;
    }
}

void TraceFlowController::InitTraceStorage(const std::string& caller)
{
    if (dbStore_ == nullptr) {
        HIVIEW_LOGE("dbStore fail init");
        return;
    }
    if (DB_CALLER.find(caller) != DB_CALLER.end()) {
        HIVIEW_LOGD("is db caller, init TraceStorage");
        traceStorage_ = std::make_shared<TraceStorage>(dbStore_, caller);
    }
    if (caller == ClientName::APP) {
        appTaskStore_ = std::make_shared<AppEventTaskStorage>(dbStore_);
    }
    if (caller == BusinessName::BEHAVIOR) {
        behaviorTaskStore_ = std::make_shared<TraceBehaviorStorage>(dbStore_);
    }
    if (caller == BusinessName::TELEMETRY) {
        teleMetryStorage_ = std::make_shared<TeleMetryStorage>(dbStore_);
    }
}

TraceFlowController::TraceFlowController(const std::string& caller, const std::string& dbPath)
{
    InitTraceDb(dbPath);
    InitTraceStorage(caller);
}

int64_t TraceFlowController::GetRemainingTraceSize()
{
    if (traceStorage_ == nullptr) {
        return 0;
    }
    return traceStorage_->GetRemainingTraceSize();
}

void TraceFlowController::StoreDb(int64_t traceSize)
{
    if (traceStorage_ == nullptr) {
        return;
    }
    traceStorage_->StoreDb(traceSize);
}

bool TraceFlowController::HasCallOnceToday(int32_t uid, uint64_t happenTime)
{
    if (appTaskStore_ == nullptr) {
        return true;
    }
    uint64_t happenTimeInSecond = happenTime / TimeUtil::SEC_TO_MILLISEC;
    std::string date = TimeUtil::TimestampFormatToDate(happenTimeInSecond, "%Y%m%d");

    AppEventTask appEventTask;
    appEventTask.id_ = 0;
    int32_t dateNum = 0;
    auto result = std::from_chars(date.c_str(), date.c_str() + date.size(), dateNum);
    if (result.ec != std::errc()) {
        HIVIEW_LOGW("convert error, dateStr: %{public}s", date.c_str());
        return true;
    }
    HIVIEW_LOGD("Query appEventTask where uid:%{public}d, dateNum:%{public}d", uid, dateNum);
    appTaskStore_->GetAppEventTask(uid, dateNum, appEventTask);
    return appEventTask.id_ > 0;
}

bool TraceFlowController::RecordCaller(std::shared_ptr<AppCallerEvent> appEvent)
{
    if (appTaskStore_ == nullptr) {
        return false;
    }
    uint64_t happenTimeInSecond = appEvent->happenTime_ / TimeUtil::SEC_TO_MILLISEC;
    std::string date = TimeUtil::TimestampFormatToDate(happenTimeInSecond, "%Y%m%d");
    int64_t dateNum = 0;
    auto result = std::from_chars(date.c_str(), date.c_str() + date.size(), dateNum);
    if (result.ec != std::errc()) {
        HIVIEW_LOGW("convert error, dateStr: %{public}s", date.c_str());
        return false;
    }
    AppEventTask appEventTask;
    appEventTask.taskDate_ = dateNum;
    appEventTask.taskType_ = APP_EVENT_TASK_TYPE_JANK_EVENT;
    appEventTask.uid_ = appEvent->uid_;
    appEventTask.pid_ = appEvent->pid_;
    appEventTask.bundleName_ = appEvent->bundleName_;
    appEventTask.bundleVersion_ = appEvent->bundleVersion_;
    appEventTask.startTime_ = appEvent->taskBeginTime_;
    appEventTask.finishTime_ = appEvent->taskEndTime_;
    appEventTask.resourePath_ = appEvent->externalLog_;
    appEventTask.resourceSize_ = static_cast<int32_t>(FileUtil::GetFileSize(appEventTask.resourePath_));
    appEventTask.state_ = APP_EVENT_TASK_STATE_FINISH;
    return appTaskStore_->InsertAppEventTask(appEventTask);
}

void TraceFlowController::CleanOldAppTrace(int32_t dateNum)
{
    if (appTaskStore_ == nullptr) {
        return;
    }
    appTaskStore_->RemoveAppEventTask(dateNum);
}

CacheFlow TraceFlowController::UseCacheTimeQuota(int32_t interval)
{
    if (behaviorTaskStore_ == nullptr) {
        return CacheFlow::EXIT;
    }
    BehaviorRecord record;
    record.behaviorId = CACHE_LOW_MEM;
    record.dateNum = TimeUtil::TimestampFormatToDate(TimeUtil::GetSeconds(), "%Y%m%d");
    if (!behaviorTaskStore_->GetRecord(record) && !behaviorTaskStore_->InsertRecord(record)) {
        HIVIEW_LOGE("failed to get and insert record, close task");
        return CacheFlow::EXIT;
    }
    HIVIEW_LOGD("get used quota:%{public}d", record.usedQuota);
    if (record.usedQuota >= HITRACE_CACHE_DURATION_LIMIT_DAILY_TOTAL) {
        HIVIEW_LOGW("usedQuota exceeds daily limit. usedQuota:%{public}d", record.usedQuota);
        return CacheFlow::OVER_FLOW;
    }
    record.usedQuota += interval;
    behaviorTaskStore_->UpdateRecord(record);
    return CacheFlow::SUCCESS;
}

TelemetryRet TraceFlowController::InitTelemetryData(const std::string &telemetryId, int64_t &runningTime,
    const std::map<std::string, int64_t>& flowControlQuotas)
{
    if (teleMetryStorage_ == nullptr) {
        HIVIEW_LOGE("failed to init teleMetryStorage");
        return TelemetryRet::EXIT;
    }
    return teleMetryStorage_->InitTelemetryControl(telemetryId, runningTime, flowControlQuotas);
}

TelemetryRet TraceFlowController::NeedTelemetryDump(const std::string &module, int64_t traceSize)
{
    if (teleMetryStorage_ == nullptr) {
        HIVIEW_LOGE("failed to init teleMetryStorage, close task");
        return TelemetryRet::EXIT;
    }
    return teleMetryStorage_->NeedTelemetryDump(module, traceSize);
}

void TraceFlowController::ClearTelemetryData()
{
    if (teleMetryStorage_ == nullptr) {
        HIVIEW_LOGE("failed to init teleMetryStorage, return");
        return;
    }
    return teleMetryStorage_->ClearTelemetryData();
}

bool TraceFlowController::QueryRunningTime(int64_t &runningTime)
{
    if (teleMetryStorage_ == nullptr) {
        HIVIEW_LOGE("failed to QueryTraceOnTime, return");
        return false;
    }
    return teleMetryStorage_->QueryRunningTime(runningTime);
}

bool TraceFlowController::UpdateRunningTime(int64_t runningTime)
{
    if (teleMetryStorage_ == nullptr) {
        HIVIEW_LOGE("failed to UpdateTraceOnTime, return");
        return false;
    }
    return teleMetryStorage_->UpdateRunningTime(runningTime);
}
} // HiViewDFX
} // OHOS
