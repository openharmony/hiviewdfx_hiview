/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "event_read_handler.h"

#include "event_write_handler.h"
#include "export_db_storage.h"
#include "hiview_logger.h"
#include "ret_code.h"
#include "sys_event_service_adapter.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-EventReadHandler");
namespace {
constexpr int64_t EVENT_QUERY_STEP = 10000; // export 10000 events in one cycle
constexpr int EACH_QUERY_MAX_LIMIT = 1000;
constexpr int QUERY_SUCCESS = 0;
}

bool EventReadHandler::HandleRequest(RequestPtr req)
{
    auto readReq = BaseRequest::DownCastTo<EventReadRequest>(req);
    int64_t exportBeginSeq = readReq->beginSeq;
    if (readReq->eventList.empty()) {
        HIVIEW_LOGE("no need to export because of empty event export config list");
        return false;
    }
    int64_t curEventSeq = SysEventServiceAdapter::GetCurrentEventSeq();
    if (exportBeginSeq == INVALID_SEQ_VAL || exportBeginSeq > curEventSeq) {
        HIVIEW_LOGE("invalid export: begin sequence:%{public}" PRId64 ", current event seq: %{public}" PRId64 "",
            exportBeginSeq, curEventSeq);
        return false;
    }
    std::map<int64_t, int64_t> queryRanges;
    while (exportBeginSeq + EVENT_QUERY_STEP < curEventSeq) {
        queryRanges.emplace(exportBeginSeq, exportBeginSeq + EVENT_QUERY_STEP);
        exportBeginSeq += EVENT_QUERY_STEP;
    };
    queryRanges.emplace(exportBeginSeq, curEventSeq);
    bool queryRet = true;
    for (const auto& queryRange : queryRanges) {
        HIVIEW_LOGD("export sysevent from %{public}" PRId64 " to %{public}" PRId64 "", queryRange.first,
            queryRange.second);
        queryRet = QuerySysEvent(queryRange.first, queryRange.second, readReq->eventList,
            [this, &readReq] (bool isQueryCompleted) {
                auto writeReq = std::make_shared<EventWriteRequest>();
                writeReq->moduleName = readReq->moduleName;
                writeReq->maxSingleFileSize = readReq->maxSize;
                writeReq->sysEvents = this->cachedSysEvents_;
                writeReq->exportDir = readReq->exportDir;
                writeReq->isQueryCompleted = isQueryCompleted;
                if (nextHandler_ != nullptr) {
                    nextHandler_->HandleRequest(writeReq);
                }
                this->cachedSysEvents_.clear();
            });
        if (!queryRet) {
            return false;
        }
    }
    return true;
}

bool EventReadHandler::NeedSwitchToNextQuery(EventStore::ResultSet& resultSet,
    QueryCallback callback, const int64_t queryLimit, int64_t& totalQueryCnt)
{
    int64_t currentQueryCnt = 0;
    EventStore::ResultSet::RecordIter iter;
    while (resultSet.HasNext() && totalQueryCnt > 0) {
        iter = resultSet.Next();
        auto currentEventStr = iter->AsJsonStr();
        if (currentEventStr.empty()) {
            continue;
        }
        if (cachedSysEvents_.size() >= static_cast<size_t>(queryLimit)) {
            callback(false);
            currentQueryCnt = 0;
        }
        CachedEventItem item {
            .version = iter->GetSysVersion(),
            .domain = iter->domain_,
            .seq = iter->GetSeq(),
            .name = iter->eventName_,
            .eventStr = currentEventStr,
        };
        cachedSysEvents_.emplace_back(item);
        currentQueryCnt++;
        totalQueryCnt--;
    }
    return currentQueryCnt < queryLimit;
}

bool EventReadHandler::QuerySysEvent(const int64_t beginSeq, const int64_t endSeq, const ExportEventList& eventList,
    QueryCallback callback)
{
    HIVIEW_LOGD("export sysevent from %{public}" PRId64 " to %{public}" PRId64 "", beginSeq, endSeq);
    int64_t queryCnt = (endSeq > beginSeq) ? (endSeq - beginSeq) : 0;
    if (queryCnt == 0) {
        HIVIEW_LOGE("no need to query because of query cnt is 0");
        return false;
    }
    EventStore::Cond whereCond;
    whereCond.And(EventStore::EventCol::SEQ, EventStore::Op::GE, beginSeq)
            .And(EventStore::EventCol::SEQ, EventStore::Op::LT, endSeq);
    std::shared_ptr<EventStore::SysEventQuery> query = nullptr;
    int32_t queryResult = QUERY_SUCCESS;
    bool isFirstPartialQuery = true;
    int64_t queryLimit = 0;
    auto iter = eventList.begin();
    while (queryCnt > 0 && iter != eventList.end()) {
        queryLimit = queryCnt < EACH_QUERY_MAX_LIMIT ? queryCnt : EACH_QUERY_MAX_LIMIT;
        query = EventStore::SysEventDao::BuildQuery(iter->first, iter->second, 0, endSeq, beginSeq);
        query->Where(whereCond);
        query->Order(EventStore::EventCol::SEQ, true);
        auto resultSet = query->Execute(queryLimit, { false, isFirstPartialQuery },
            std::make_pair(EventStore::INNER_PROCESS_ID, ""),
            [&queryResult] (EventStore::DbQueryStatus status) {
                std::unordered_map<EventStore::DbQueryStatus, int32_t> statusToCode {
                    { EventStore::DbQueryStatus::CONCURRENT, ERR_TOO_MANY_CONCURRENT_QUERIES },
                    { EventStore::DbQueryStatus::OVER_TIME, ERR_QUERY_OVER_TIME },
                    { EventStore::DbQueryStatus::OVER_LIMIT, ERR_QUERY_OVER_LIMIT },
                    { EventStore::DbQueryStatus::TOO_FREQENTLY, ERR_QUERY_TOO_FREQUENTLY },
                };
                queryResult = statusToCode[status];
            });
        if (queryResult != QUERY_SUCCESS) {
            callback(true);
            return false;
        }
        if (NeedSwitchToNextQuery(resultSet, callback, queryLimit, queryCnt)) {
            ++iter;
        }
        isFirstPartialQuery = false;
    }
    callback(true);
    return true;
}
} // HiviewDFX
} // OHOS