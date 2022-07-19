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

#ifndef HIVIEW_PLUGINS_USAGE_EVENT_REPORT_SREVICE_EVENT_DB_HELPER_H
#define HIVIEW_PLUGINS_USAGE_EVENT_REPORT_SREVICE_EVENT_DB_HELPER_H

#include <map>
#include <memory>
#include <string>

#include "logger_event.h"
#include "store_manager.h"

namespace OHOS {
namespace HiviewDFX {
class EventDbHelper {
public:
    EventDbHelper(const std::string workPath);
    ~EventDbHelper();
    int InsertPluginStatsEvent(const std::shared_ptr<LoggerEvent>& event);
    int InsertSysUsageEvent(const std::shared_ptr<LoggerEvent>& event, const std::string& coll);
    int QueryPluginStatsEvent(std::vector<std::shared_ptr<LoggerEvent>>& events, const std::string& pluginName = "");
    int QuerySysUsageEvent(std::vector<std::shared_ptr<LoggerEvent>>& events, const std::string& coll);
    int DeletePluginStatsEvent();
    int DeleteSysUsageEvent(const std::string& coll);

private:
    void InitDbPath();
    void InitDbStoreMgr();
    std::shared_ptr<DocStore> GetDocStore();
    int CloseDocStore();
    int InsertDb(const std::string& jsonStr, const std::string& coll);
    void BuildQuery(DataQuery& query, const std::map<std::string, std::string>& condMap = {});
    int QueryDb(const DataQuery& query, std::vector<Entry>& entries, const std::string& coll);
    int UpdateDb(int id, const std::string& value, const std::string& coll);
    int DeleteDb(const DataQuery& query, const std::string& coll);

private:
    std::string dbPath_;
    std::unique_ptr<StoreManager> storeMgr_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_USAGE_EVENT_REPORT_SREVICE_EVENT_DB_HELPER_H
