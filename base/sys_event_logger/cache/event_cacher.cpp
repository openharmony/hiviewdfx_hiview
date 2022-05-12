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
#include "event_cacher.h"

#include "hilog/log.h"
#include "plugin_stats_event_factory.h"
#include "sys_event_common.h"
#include "sys_usage_event_factory.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, LABEL_DOMAIN, "SysEventLogger-EventCacher" };
constexpr uint64_t DEFAULT_TIME = 0;
}
using namespace PluginStatsEventSpace;
using namespace SysUsageEventSpace;

EventCacher::EventCacher() : dbHelper_(nullptr), sysUsageEvent_(nullptr)
{}

EventCacher::~EventCacher()
{
    pluginStatsEvents_.clear();
    dbHelper_ = nullptr;
    sysUsageEvent_ = nullptr;
}

void EventCacher::InitCache(const std::string& workPath)
{
    dbHelper_ = std::make_unique<EventDbHelper>();
    dbHelper_->Init(workPath);
    InitSysUsageEvent();
    InitPluginStatsEvents();
}

void EventCacher::GetPluginStatsEvents(std::vector<std::shared_ptr<LoggerEvent>>& events)
{
    for (auto entry : pluginStatsEvents_) {
        events.push_back(entry.second);
    }
}

void EventCacher::UpdatePluginStatsEvent(const std::string &name, const std::string &procName, uint32_t procTime)
{
    if (pluginStatsEvents_.find(name) != pluginStatsEvents_.end()) {
        auto event = pluginStatsEvents_.find(name)->second;
        event->Update(KEY_OF_PROC_NAME, procName);
        event->Update(KEY_OF_PROC_TIME, procTime);
    } else {
        auto event = std::make_unique<PluginStatsEventFactory>()->Create();
        event->Update(KEY_OF_PLUGIN_NAME, name);
        event->Update(KEY_OF_PROC_NAME, procName);
        event->Update(KEY_OF_PROC_TIME, procTime);
        pluginStatsEvents_[name] = std::move(event);
    }
}

void EventCacher::ClearPluginStatsEvents()
{
    pluginStatsEvents_.clear();
    DeletePluginStatsEventsFromDb();
}

std::shared_ptr<LoggerEvent> EventCacher::GetSysUsageEvent()
{
    return sysUsageEvent_;
}

void EventCacher::AddUint64ToEvent(std::shared_ptr<LoggerEvent>& event, const std::string &name, uint64_t value)
{
    uint64_t newValue = event->GetValue(name).GetUint64() + value;
    newValue = newValue > TimeUtil::MILLISECS_PER_DAY ? TimeUtil::MILLISECS_PER_DAY : newValue;
    event->Update(name, newValue);
}

void EventCacher::UpdateSysUsageEvent()
{
    auto event = std::make_unique<SysUsageEventFactory>()->Create();
    sysUsageEvent_->Update(KEY_OF_END, event->GetValue(KEY_OF_END).GetUint64());
    AddUint64ToEvent(sysUsageEvent_, KEY_OF_POWER, event->GetValue(KEY_OF_POWER).GetUint64());
    AddUint64ToEvent(sysUsageEvent_, KEY_OF_RUNNING, event->GetValue(KEY_OF_RUNNING).GetUint64());
    AddUint64ToEvent(sysUsageEvent_, KEY_OF_SCREEN, event->GetValue(KEY_OF_SCREEN).GetUint64());
}

void EventCacher::ClearSysUsageEvent()
{
    sysUsageEvent_->Update(KEY_OF_START, sysUsageEvent_->GetValue(KEY_OF_END).GetUint64());
    sysUsageEvent_->Update(KEY_OF_POWER, DEFAULT_TIME);
    sysUsageEvent_->Update(KEY_OF_RUNNING, DEFAULT_TIME);
    sysUsageEvent_->Update(KEY_OF_SCREEN, DEFAULT_TIME);
    SaveSysUsageEventToDb();
}

void EventCacher::DeletePluginStatsEventsFromDb()
{
    std::vector<std::shared_ptr<LoggerEvent>> events;
    if (dbHelper_->QueryPluginStatsEvent(events) >= 0) {
        dbHelper_->DeletePluginStatsEvent();
    }
}

void EventCacher::DeleteSysUsageEventFromDb()
{
    std::vector<std::shared_ptr<LoggerEvent>> events;
    if (dbHelper_->QuerySysUsageEvent(events) >= 0) {
        dbHelper_->DeleteSysUsageEvent();
    }
}

void EventCacher::InitSysUsageEvent()
{
    std::vector<std::shared_ptr<LoggerEvent>> events;
    if (dbHelper_->QuerySysUsageEvent(events) <= 0) {
        HiLog::Warn(LABEL, "failed to query sys usage event from db");
        sysUsageEvent_ = std::make_unique<SysUsageEventFactory>()->Create();
        sysUsageEvent_->Update(KEY_OF_POWER, DEFAULT_TIME);
        sysUsageEvent_->Update(KEY_OF_RUNNING, DEFAULT_TIME);
        sysUsageEvent_->Update(KEY_OF_SCREEN, DEFAULT_TIME);
    } else {
        sysUsageEvent_ = events[0];
    }
}

void EventCacher::InitPluginStatsEvents()
{
    std::vector<std::shared_ptr<LoggerEvent>> events;
    if (dbHelper_->QueryPluginStatsEvent(events) <= 0) {
        HiLog::Warn(LABEL, "failed to query plugin stats events from db");
        return;
    }
    for (auto event : events) {
        std::string pluginName = event->GetValue(KEY_OF_PLUGIN_NAME).GetString();
        if (!pluginName.empty()) {
            pluginStatsEvents_[pluginName] = event;
        }
    }
}

void EventCacher::SavePluginStatsEventsToDb()
{
    std::vector<std::shared_ptr<LoggerEvent>> events;
    GetPluginStatsEvents(events);
    for (auto event : events) {
        if (dbHelper_->InsertPluginStatsEvent(event) < 0) {
            HiLog::Error(LABEL, "failed to save event to db");
        }
    }
}

void EventCacher::SaveSysUsageEventToDb()
{
    if (dbHelper_->InsertSysUsageEvent(sysUsageEvent_) < 0) {
        HiLog::Error(LABEL, "failed to save sys usage event to db");
    }
}
} // namespace HiviewDFX
} // namespace OHOS
