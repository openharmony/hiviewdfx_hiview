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
#include "sys_event_logger.h"

#include "app_usage_event_factory.h"
#include "event_cacher.h"
#include "hilog/log.h"
#include "plugin_fault_event_factory.h"
#include "plugin_load_event_factory.h"
#include "plugin_unload_event_factory.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, LABEL_DOMAIN, "HiView-SysEventLogger" };
}

void SysEventLogger::Init(const std::string &workPath)
{
    HiLog::Info(LABEL, "Init start");
    EventCacher::GetInstance().InitCache(workPath);
    HiLog::Info(LABEL, "Init succeeded");
}

void SysEventLogger::ReportPluginLoad(const std::string &name, uint32_t result)
{
    auto factory = std::make_unique<PluginLoadEventFactory>();
    auto event = factory->Create();
    event->Update(PluginEventSpace::KEY_OF_PLUGIN_NAME, name);
    event->Update(PluginEventSpace::KEY_OF_RESULT, result);
    event->Report();
    HiLog::Info(LABEL, "report plugin load event=%{public}s", event->ToJsonString().c_str());
}

void SysEventLogger::ReportPluginUnload(const std::string &name, uint32_t result)
{
    auto factory = std::make_unique<PluginUnloadEventFactory>();
    auto event = factory->Create();
    event->Update(PluginEventSpace::KEY_OF_PLUGIN_NAME, name);
    event->Update(PluginEventSpace::KEY_OF_RESULT, result);
    event->Report();
    HiLog::Info(LABEL, "report plugin unload event=%{public}s", event->ToJsonString().c_str());
}

void SysEventLogger::ReportPluginFault(const std::string &name, const std::string &reason)
{
    auto factory = std::make_unique<PluginFaultEventFactory>();
    auto event = factory->Create();
    event->Update(PluginFaultEventSpace::KEY_OF_PLUGIN_NAME, name);
    event->Update(PluginFaultEventSpace::KEY_OF_REASON, reason);
    event->Report();
    HiLog::Info(LABEL, "report plugin fault event=%{public}s", event->ToJsonString().c_str());
}

void SysEventLogger::ReportPluginStats()
{
    std::vector<std::shared_ptr<LoggerEvent>> events;
    EventCacher::GetInstance().GetPluginStatsEvents(events);
    for (auto event : events) {
        event->Report();
        HiLog::Info(LABEL, "report plugin stat event=%{public}s", event->ToJsonString().c_str());
    }
    EventCacher::GetInstance().ClearPluginStatsEvents();
}

void SysEventLogger::ReportAppUsage()
{
    auto factory = std::make_unique<AppUsageEventFactory>();
    std::vector<std::unique_ptr<LoggerEvent>> events;
    factory->Create(events);
    for (size_t i = 0; i < events.size(); ++i) {
        events[i]->Report();
        HiLog::Info(LABEL, "report app usage event=%{public}s", events[i]->ToJsonString().c_str());
    }
}

void SysEventLogger::ReportSysUsage()
{
    EventCacher::GetInstance().UpdateSysUsageEvent();
    auto reportEvent = EventCacher::GetInstance().GetSysUsageEvent();
    reportEvent->Report();
    HiLog::Info(LABEL, "report sys usage event=%{public}s", reportEvent->ToJsonString().c_str());
    EventCacher::GetInstance().ClearSysUsageEvent();
}

void SysEventLogger::UpdatePluginStats(const std::string &name, const std::string &procName, uint32_t procTime)
{
    HiLog::Debug(LABEL, "UpdatePluginStats pluginName=%{public}s, procName=%{public}s, time=%{public}d",
        name.c_str(), procName.c_str(), procTime);
    EventCacher::GetInstance().UpdatePluginStatsEvent(name, procName, procTime);
}
} // namespace HiviewDFX
} // namespace OHOS
