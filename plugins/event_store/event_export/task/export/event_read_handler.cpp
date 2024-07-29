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
#include "file_util.h"
#include "hiview_logger.h"
#include "sys_event_dao.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-EventReadHandler");
namespace {
constexpr int EACH_QUERY_MAX_LIMIT = 1000;
}

bool EventReadHandler::HandleRequest(RequestPtr req)
{
    RegistExportFilePackagedListener();
    auto readReq = BaseRequest::DownCastTo<EventReadRequest>(req);
    int64_t exportBeginSeq = readReq->beginSeq;
    int64_t exportEndSeq = readReq->endSeq;
    // split range
    std::map<int64_t, int64_t> queryRanges;
    while (exportBeginSeq + EACH_QUERY_MAX_LIMIT < exportEndSeq) {
        queryRanges.emplace(exportBeginSeq, exportBeginSeq + EACH_QUERY_MAX_LIMIT);
        exportBeginSeq += EACH_QUERY_MAX_LIMIT;
    };
    // query by range in order
    queryRanges.emplace(exportBeginSeq, exportEndSeq);
    for (const auto& queryRange : queryRanges) {
        bool queryRet = QuerySysEvent(queryRange.first, queryRange.second, readReq->eventList,
            [this, &readReq] (bool isQueryCompleted) {
                auto writeReq = std::make_shared<EventWriteRequest>(readReq->moduleName, cachedSysEvents_,
                    readReq->exportDir, isQueryCompleted, readReq->maxSize);
                auto ret = nextHandler_->HandleRequest(writeReq);
                cachedSysEvents_.clear();
                return ret;
            });
        if (eventExportedListener_ != nullptr) {
            eventExportedListener_(queryRange.first, queryRange.second);
        }
        if (!queryRet) {
            HIVIEW_LOGE("failed to export from %{public}" PRId64 " to %{public}" PRId64 "", queryRange.first,
                queryRange.second);
            Rollback();
            return false;
        }
        CopyTmpZipFilesToDest();
    }
    return true;
}

bool EventReadHandler::QuerySysEvent(const int64_t beginSeq, const int64_t endSeq, const ExportEventList& eventList,
    QueryCallback queryCallback)
{
    int64_t queryCnt = endSeq - beginSeq;
    EventStore::Cond whereCond;
    whereCond.And(EventStore::EventCol::SEQ, EventStore::Op::GE, beginSeq)
        .And(EventStore::EventCol::SEQ, EventStore::Op::LT, endSeq);
    std::shared_ptr<EventStore::SysEventQuery> query = nullptr;
    int32_t queryRet = static_cast<int32_t>(EventStore::DbQueryStatus::SUCCEED);
    bool isFirstPartialQuery = true;
    auto iter = eventList.begin();
    while (queryCnt > 0 && iter != eventList.end()) {
        int64_t queryLimit = queryCnt < EACH_QUERY_MAX_LIMIT ? queryCnt : EACH_QUERY_MAX_LIMIT;
        query = EventStore::SysEventDao::BuildQuery(iter->first, iter->second, 0, endSeq, beginSeq);
        query->Where(whereCond);
        query->Order(EventStore::EventCol::SEQ, true);
        auto resultSet = query->Execute(queryLimit, { true, isFirstPartialQuery },
            std::make_pair(EventStore::INNER_PROCESS_ID, ""),
            [&queryRet] (EventStore::DbQueryStatus status) {
                std::unordered_map<EventStore::DbQueryStatus, int32_t> statusToCode {
                    { EventStore::DbQueryStatus::CONCURRENT,
                        static_cast<int32_t>(EventStore::DbQueryStatus::CONCURRENT)},
                    { EventStore::DbQueryStatus::OVER_TIME,
                        static_cast<int32_t>(EventStore::DbQueryStatus::OVER_TIME)},
                    { EventStore::DbQueryStatus::OVER_LIMIT,
                        static_cast<int32_t>(EventStore::DbQueryStatus::OVER_LIMIT)},
                    { EventStore::DbQueryStatus::TOO_FREQENTLY,
                        static_cast<int32_t>(EventStore::DbQueryStatus::TOO_FREQENTLY)},
                };
                queryRet = statusToCode[status];
            });
        if (queryRet != static_cast<int32_t>(EventStore::DbQueryStatus::SUCCEED)) {
            HIVIEW_LOGW("query control works when query with domain %{public}s, query ret is %{public}d",
                iter->first.c_str(), queryRet);
        }
        if (!HandleQueryResult(resultSet, queryCallback, queryLimit, queryCnt)) {
            HIVIEW_LOGE("failed to export events with domain: %{public}s in range [%{public}"
                PRId64 ",%{publiuc}" PRId64 ")", iter->first.c_str(), beginSeq, endSeq);
            return false;
        }
        iter++;
        isFirstPartialQuery = false;
    }
    return queryCallback(true);
}

bool EventReadHandler::HandleQueryResult(EventStore::ResultSet& resultSet, QueryCallback queryCallback,
    const int64_t queryLimit, int64_t& totalQueryCnt)
{
    EventStore::ResultSet::RecordIter iter;
    while (resultSet.HasNext() && totalQueryCnt > 0) {
        iter = resultSet.Next();
        auto currentEventStr = iter->AsJsonStr();
        if (currentEventStr.empty()) {
            continue;
        }
        if (cachedSysEvents_.size() >= static_cast<size_t>(queryLimit) && !queryCallback(false)) {
            return false;
        }
        auto item = std::make_shared<CachedEventItem>(iter->GetSysVersion(), iter->domain_, iter->eventName_,
            currentEventStr);
        cachedSysEvents_.emplace_back(item);
        totalQueryCnt--;
    }
    return true;
}

void EventReadHandler::CopyTmpZipFilesToDest()
{
    // move all tmp zipped event export file to dest dir
    std::for_each(zippedEventFileMap_.begin(), zippedEventFileMap_.end(), [] (const auto& item) {
        if (!FileUtil::RenameFile(item.first, item.second)) {
            HIVIEW_LOGE("failed to move %{private}s to %{private}s", item.first.c_str(), item.second.c_str());
        }
    });
    // clear cache
    zippedEventFileMap_.clear();
}

void EventReadHandler::Rollback()
{
    // delete all tmp zipped event export file
    std::for_each(zippedEventFileMap_.begin(), zippedEventFileMap_.end(), [] (const auto& item) {
        if (!FileUtil::RemoveFile(item.first)) {
            HIVIEW_LOGE("failed to delete %{private}s", item.first.c_str());
        }
    });
    // clear cache
    zippedEventFileMap_.clear();
}

void EventReadHandler::RegistExportFilePackagedListener()
{
    auto writerHandler = reinterpret_cast<EventWriteHandler*>(nextHandler_.get());
    writerHandler->SetExportFilePackgedListener([this] (const std::string& srcFilePath,
        const std::string& destFilePath) {
        zippedEventFileMap_[srcFilePath] = destFilePath;
    });
}

void EventReadHandler::SetEventExportedListener(EventExportedListener listener)
{
    eventExportedListener_ = listener;
}
} // HiviewDFX
} // OHOS