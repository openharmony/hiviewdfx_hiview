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

#ifndef HIVIEW_PLUGINS_USAGE_EVENT_REPORT_SREVICE_USAGE_EVENT_CACHER_H
#define HIVIEW_PLUGINS_USAGE_EVENT_REPORT_SREVICE_USAGE_EVENT_CACHER_H

#include <map>
#include <memory>
#include <string>

#include "event_db_helper.h"
#include "logger_event.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const std::string SYS_USAGE_COLL = "sys_usage";
}
class UsageEventCacher {
public:
    UsageEventCacher(const std::string& workPath);
    ~UsageEventCacher() {}

    void GetPluginStatsEvents(std::vector<std::shared_ptr<LoggerEvent>>& events);
    std::shared_ptr<LoggerEvent> GetSysUsageEvent(const std::string& coll = SYS_USAGE_COLL);
    void DeletePluginStatsEventsFromDb();
    void SavePluginStatsEventsToDb(const std::vector<std::shared_ptr<LoggerEvent>>& events);
    void DeleteSysUsageEventFromDb(const std::string& coll = SYS_USAGE_COLL);
    void SaveSysUsageEventToDb(const std::shared_ptr<LoggerEvent>& event, const std::string& coll = SYS_USAGE_COLL);

private:
    std::unique_ptr<EventDbHelper> dbHelper_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_USAGE_EVENT_REPORT_SREVICE_EVENT_CACHER_H
