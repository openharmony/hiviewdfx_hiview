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

#ifndef HIVIEW_PLUGINS_SYS_EVENT_LOGGER_EVENT_CACHER_H
#define HIVIEW_PLUGINS_SYS_EVENT_LOGGER_EVENT_CACHER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "logger_event.h"
#include "event_db_helper.h"
#include "singleton.h"

namespace OHOS {
namespace HiviewDFX {
class EventCacher : public DelayedRefSingleton<EventCacher> {
public:
    EventCacher();
    ~EventCacher();

    void InitCache(const std::string& workPath);

    void GetPluginStatsEvents(std::vector<std::shared_ptr<LoggerEvent>>& events);
    void UpdatePluginStatsEvent(const std::string &name, const std::string &procName, uint32_t procTime);
    void ClearPluginStatsEvents();

    std::shared_ptr<LoggerEvent> GetSysUsageEvent();
    void UpdateSysUsageEvent(const std::shared_ptr<LoggerEvent>& event);
    void ClearSysUsageEvent();

    void DeletePluginStatsEventsFromDb();
    void SavePluginStatsEventsToDb();
    void DeleteSysUsageEventFromDb();
    void SaveSysUsageEventToDb();

private:
    void InitSysUsageEvent();
    void InitPluginStatsEvents();
    void AddUint64ToEvent(std::shared_ptr<LoggerEvent>& event, const std::string &name, uint64_t value);

private:
    std::unique_ptr<EventDbHelper> dbHelper_;
    std::map<std::string, std::shared_ptr<LoggerEvent>> pluginStatsEvents_;
    std::shared_ptr<LoggerEvent> sysUsageEvent_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_SYS_EVENT_LOGGER_EVENT_CACHER_H
