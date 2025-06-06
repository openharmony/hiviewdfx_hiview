/*
 * Copyright (C) 2023-2025 Huawei Device Co., Ltd.
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
#include "trace_storage.h"

#include <algorithm>
#include <cinttypes>

#include "hiview_logger.h"
#include "parameter_ex.h"
#include "time_util.h"
#include "trace_common.h"
#include "trace_quota_config.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
DEFINE_LOG_TAG("TraceStorage");
const std::string TABLE_NAME = "trace_flow_control";
const std::string COLUMN_SYSTEM_TIME = "system_time";
const std::string COLUMN_CALLER_NAME = "caller_name";
const std::string COLUMN_USED_SIZE = "used_size";
const float TEN_PERCENT_LIMIT = 0.1;

NativeRdb::ValuesBucket GetBucket(const TraceFlowRecord& traceFlowRecord)
{
    NativeRdb::ValuesBucket bucket;
    bucket.PutString(COLUMN_SYSTEM_TIME, traceFlowRecord.systemTime);
    bucket.PutString(COLUMN_CALLER_NAME, traceFlowRecord.callerName);
    bucket.PutLong(COLUMN_USED_SIZE, traceFlowRecord.usedSize);
    return bucket;
}

const std::map<std::string, int64_t> TRACE_QUOTA = {
    {CallerName::XPERF, TraceQuotaConfig::GetTraceQuotaByCaller(CallerName::XPERF)},
    {CallerName::XPOWER, TraceQuotaConfig::GetTraceQuotaByCaller(CallerName::XPOWER)},
    {CallerName::RELIABILITY, TraceQuotaConfig::GetTraceQuotaByCaller(CallerName::RELIABILITY)},
    {CallerName::HIVIEW, TraceQuotaConfig::GetTraceQuotaByCaller(CallerName::HIVIEW)},
};
}

TraceStorage::TraceStorage(std::shared_ptr<NativeRdb::RdbStore> dbStore, const std::string& caller)
    : caller_(caller), dbStore_(dbStore)
{
    InitTableRecord();
}

void TraceStorage::InitTableRecord()
{
    if (TRACE_QUOTA.find(caller_) == TRACE_QUOTA.end()) {
        HIVIEW_LOGE("caller_ is invalid");
        return;
    }
    traceFlowRecord_.callerName = caller_;
    Query(traceFlowRecord_);
    HIVIEW_LOGI("systemTime:%{public}s, callerName:%{public}s, usedSize:%{public}" PRId64,
    traceFlowRecord_.systemTime.c_str(), traceFlowRecord_.callerName.c_str(),
        traceFlowRecord_.usedSize);
}

void TraceStorage::Store(const TraceFlowRecord& traceFlowRecord)
{
    if (dbStore_ == nullptr) {
        HIVIEW_LOGE("db store is null,");
        return;
    }
    TraceFlowRecord tmpTraceFlowRecord = {.callerName = traceFlowRecord.callerName};
    Query(tmpTraceFlowRecord);
    if (!tmpTraceFlowRecord.systemTime.empty()) { // time not empty means record exist
        UpdateTable(traceFlowRecord);
    } else {
        InsertTable(traceFlowRecord);
    }
}

void TraceStorage::UpdateTable(const TraceFlowRecord& traceFlowRecord)
{
    NativeRdb::ValuesBucket bucket = GetBucket(traceFlowRecord);
    NativeRdb::AbsRdbPredicates predicates(TABLE_NAME);
    predicates.EqualTo(COLUMN_CALLER_NAME, traceFlowRecord.callerName);
    int changeRows = 0;
    if (dbStore_->Update(changeRows, bucket, predicates) != NativeRdb::E_OK) {
        HIVIEW_LOGW("failed to update table");
    }
}

void TraceStorage::InsertTable(const TraceFlowRecord& traceFlowRecord)
{
    NativeRdb::ValuesBucket bucket = GetBucket(traceFlowRecord);
    int64_t seq = 0;
    if (dbStore_->Insert(seq, TABLE_NAME, bucket) != NativeRdb::E_OK) {
        HIVIEW_LOGW("failed to insert table");
    }
}

void TraceStorage::Query(TraceFlowRecord& traceFlowRecord)
{
    if (dbStore_ == nullptr) {
        HIVIEW_LOGE("db store is null");
        return;
    }
    QueryTable(traceFlowRecord);
}

void TraceStorage::QueryTable(TraceFlowRecord& traceFlowRecord)
{
    NativeRdb::AbsRdbPredicates predicates(TABLE_NAME);
    predicates.EqualTo(COLUMN_CALLER_NAME, traceFlowRecord.callerName);
    auto resultSet = dbStore_->Query(predicates, {COLUMN_SYSTEM_TIME, COLUMN_USED_SIZE});
    if (resultSet == nullptr) {
        HIVIEW_LOGE("failed to query from table %{public}s, db is null", TABLE_NAME.c_str());
        return;
    }

    if (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        resultSet->GetString(0, traceFlowRecord.systemTime); // 0 means system_time field
        resultSet->GetLong(1, traceFlowRecord.usedSize); // 1 means used_size field
    }
    resultSet->Close();
}

std::string TraceStorage::GetDate()
{
    std::string dateStr = TimeUtil::TimestampFormatToDate(std::time(nullptr), "%Y-%m-%d");
    return dateStr;
}

// remaining trace size contains 10% fluctuation of the quota when quota not completely used up
int64_t TraceStorage::GetRemainingTraceSize()
{
    if (TRACE_QUOTA.find(caller_) == TRACE_QUOTA.end()) {
        return 0;
    }
    auto quota = TRACE_QUOTA.at(caller_);
    std::string nowDays = GetDate();
    HIVIEW_LOGI("start to dump, nowDays = %{public}s, systemTime = %{public}s.",
                nowDays.c_str(), traceFlowRecord_.systemTime.c_str());
    if (nowDays != traceFlowRecord_.systemTime) {
        HIVIEW_LOGD("date changes");
        traceFlowRecord_.systemTime = nowDays;
        traceFlowRecord_.usedSize = 0;
        return quota + quota * TEN_PERCENT_LIMIT;
    }
    if (quota <= traceFlowRecord_.usedSize) {
        return 0;
    }
    return (quota - traceFlowRecord_.usedSize) + quota * TEN_PERCENT_LIMIT;
}

void TraceStorage::StoreDb(int64_t traceSize)
{
    if (TRACE_QUOTA.find(caller_) == TRACE_QUOTA.end()) {
        HIVIEW_LOGI("caller %{public}s not need store", caller_.c_str());
        return;
    }
    traceFlowRecord_.usedSize += traceSize;
    HIVIEW_LOGI("systemTime:%{public}s, callerName:%{public}s, usedSize:%{public}" PRId64,
                traceFlowRecord_.systemTime.c_str(), traceFlowRecord_.callerName.c_str(),
                traceFlowRecord_.usedSize);
    Store(traceFlowRecord_);
}
}  // namespace HiviewDFX
}  // namespace OHOS
