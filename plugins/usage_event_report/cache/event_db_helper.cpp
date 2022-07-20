/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "event_db_helper.h"

#include "data_query.h"
#include "file_util.h"
#include "json_parser.h"
#include "logger.h"
#include "plugin_stats_event_factory.h"
#include "sys_usage_event.h"
#include "usage_event_common.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-EventDbHelper");
namespace {
const std::string DB_DIR = "sys_event_logger/";
const std::string DB_NAME = "event.db";
const std::string PLUGIN_STATS_COLL = "plugin_stats";
constexpr uint32_t DEFAULT_RECORD_NUM = 1;
constexpr int ERR_DEFAULT = -1;
}

EventDbHelper::EventDbHelper(const std::string workPath) : dbPath_(workPath), storeMgr_(nullptr)
{
    InitDbPath();
    InitDbStoreMgr();
}

EventDbHelper::~EventDbHelper()
{
    if (CloseDocStore() < 0) {
        HIVIEW_LOGE("fail to close db");
    }
}

void EventDbHelper::InitDbPath()
{
    std::string workPath = this->dbPath_;
    if (workPath.back() != '/') {
        this->dbPath_ +=  "/";
    }
    this->dbPath_ += DB_DIR;
    if (FileUtil::FileExists(this->dbPath_)) {
        this->dbPath_ += DB_NAME;
        return;
    }

    if (FileUtil::ForceCreateDirectory(this->dbPath_, FileUtil::FILE_PERM_770)) {
        HIVIEW_LOGI("create sys_event_logger path successfully");
    } else {
        this->dbPath_ = workPath;
        HIVIEW_LOGE("failed to create sys_event_logger path, use default path");
    }
    this->dbPath_ += DB_NAME;
}

void EventDbHelper::InitDbStoreMgr()
{
    this->storeMgr_ = std::make_unique<StoreManager>();
}

std::shared_ptr<DocStore> EventDbHelper::GetDocStore()
{
    Option option;
    option.db = this->dbPath_;
    option.flag = Option::NO_TRIM_ON_CLOSE;
    return this->storeMgr_->GetDocStore(option);
}

int EventDbHelper::CloseDocStore()
{
    Option option;
    option.db = this->dbPath_;
    return this->storeMgr_->CloseDocStore(option);
}

int EventDbHelper::InsertPluginStatsEvent(const std::shared_ptr<LoggerEvent>& event)
{
    if (event == nullptr) {
        return ERR_DEFAULT;
    }
    std::vector<std::shared_ptr<LoggerEvent>> queryEvents;
    std::string pluginName = event->GetValue(PluginStatsEventSpace::KEY_OF_PLUGIN_NAME).GetString();
    int queryRes = QueryPluginStatsEvent(queryEvents, pluginName);
    if (queryRes >= 0 && queryEvents.size() == DEFAULT_RECORD_NUM) {
        return UpdateDb(queryRes, event->ToJsonString(), PLUGIN_STATS_COLL);
    } else {
        return InsertDb(event->ToJsonString(), PLUGIN_STATS_COLL);
    }
}

int EventDbHelper::InsertSysUsageEvent(const std::shared_ptr<LoggerEvent>& event, const std::string& coll)
{
    if (event == nullptr) {
        return ERR_DEFAULT;
    }
    std::vector<std::shared_ptr<LoggerEvent>> queryEvents;
    int queryRes = QuerySysUsageEvent(queryEvents, coll);
    if (queryRes >= 0 && queryEvents.size() == DEFAULT_RECORD_NUM) {
        return UpdateDb(queryRes, event->ToJsonString(), coll);
    } else {
        return InsertDb(event->ToJsonString(), coll);
    }
}

int EventDbHelper::InsertDb(const std::string& jsonStr, const std::string& coll)
{
    HIVIEW_LOGD("insert db file %{public}s with %{public}s", this->dbPath_.c_str(), jsonStr.c_str());
    auto docStore = GetDocStore();
    Entry entry;
    entry.id = 0;
    entry.value = jsonStr;
    if (docStore->Put(entry, coll.c_str()) != 0) {
        HIVIEW_LOGE("failed to insert collection %{public}s for %{public}s", coll.c_str(), jsonStr.c_str());
        return ERR_DEFAULT;
    }
    return entry.id;
}

void EventDbHelper::BuildQuery(DataQuery& query, const std::map<std::string, std::string>& condMap)
{
    for (auto& cond : condMap) {
        query.EqualTo(cond.first, cond.second);
    }
}

int EventDbHelper::QueryPluginStatsEvent(std::vector<std::shared_ptr<LoggerEvent>>& events,
    const std::string& pluginName)
{
    std::map<std::string, std::string> condMap;
    if (!pluginName.empty()) {
        condMap = { {PluginStatsEventSpace::KEY_OF_PLUGIN_NAME, pluginName} };
    }
    DataQuery query;
    BuildQuery(query, condMap);
    std::vector<Entry> entries;
    int res = QueryDb(query, entries, PLUGIN_STATS_COLL);
    if (res < 0) {
        return res;
    }
    auto factory = std::make_unique<PluginStatsEventFactory>();
    for (auto entry : entries) {
        res = entry.id;
        std::shared_ptr<LoggerEvent> event = factory->Create();
        if (!JsonParser::ParsePluginStatsEvent(event, entry.value)) {
            HIVIEW_LOGE("failed to parse the database records=%{public}s", entry.value.c_str());
            continue;
        }
        events.push_back(event);
    }
    HIVIEW_LOGI("query plugin_stats result=%{public}d, events size=%{public}zu", res, entries.size());
    return events.size() == DEFAULT_RECORD_NUM ? res : static_cast<int>(events.size());
}

int EventDbHelper::QuerySysUsageEvent(std::vector<std::shared_ptr<LoggerEvent>>& events, const std::string& coll)
{
    using namespace SysUsageEventSpace;

    std::vector<Entry> entries;
    int res = QueryDb(DataQuery(), entries, coll);
    if (res < 0) {
        return res;
    }
    for (auto entry : entries) {
        res = entry.id;
        std::shared_ptr<LoggerEvent> event =
            std::make_shared<SysUsageEvent>(EVENT_DOMAIN, EVENT_NAME, HiSysEvent::STATISTIC);
        if (!JsonParser::ParseSysUsageEvent(event, entries[0].value)) {
            HIVIEW_LOGE("failed to parse the database records=%{public}s", entries[0].value.c_str());
            continue;
        }
        events.push_back(event);
    }
    HIVIEW_LOGI("query sys_usage result=%{public}d, events size=%{public}zu", res, entries.size());
    return  events.size() == DEFAULT_RECORD_NUM ? res : static_cast<int>(events.size());
}

int EventDbHelper::QueryDb(const DataQuery& query, std::vector<Entry>& entries, const std::string& coll)
{
    return GetDocStore()->GetEntriesWithQuery(query, entries, coll.c_str());
}

int EventDbHelper::UpdateDb(int id, const std::string& value, const std::string& coll)
{
    HIVIEW_LOGD("update db coll %{public}s with %{public}s", coll.c_str(), value.c_str());
    Entry entry;
    entry.id = id;
    entry.value = value;
    return GetDocStore()->Merge(entry, coll.c_str());
}

int EventDbHelper::DeletePluginStatsEvent()
{
    return DeleteDb(DataQuery(), PLUGIN_STATS_COLL);
}

int EventDbHelper::DeleteSysUsageEvent(const std::string& coll)
{
    return DeleteDb(DataQuery(), coll);
}

int EventDbHelper::DeleteDb(const DataQuery& query, const std::string& coll)
{
    HIVIEW_LOGD("delete db coll %{public}s", coll.c_str());
    return GetDocStore()->Delete(query, coll.c_str());
}
} // namespace HiviewDFX
} // namespace OHOS
