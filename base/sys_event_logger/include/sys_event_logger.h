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

#ifndef HIVIEW_PLUGINS_SYS_EVENT_LOGGER_SYS_EVENT_LOGGER_H
#define HIVIEW_PLUGINS_SYS_EVENT_LOGGER_SYS_EVENT_LOGGER_H

#include <string>

#include "sys_event_common.h"

namespace OHOS {
namespace HiviewDFX {
using SysEventLoggerTask = void (*)();

class SysEventLogger {
public:
    static void Init(const std::string &workPath);
    void Timeout();
    static void ReportPluginLoad(const std::string &name, uint32_t result);
    static void ReportPluginUnload(const std::string &name, uint32_t result);
    static void ReportPluginFault(const std::string &name, const std::string &reason);
    static void ReportPluginStats();
    static void ReportAppUsage();
    static void ReportSysUsage();
    static void UpdatePluginStats(const std::string &name, const std::string &procName, uint32_t procTime);

private:
    void Start();
    void AddScheduledTask(uint32_t interval, SysEventLoggerTask task);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_SYS_EVENT_LOGGER_SYS_EVENT_LOGGER_H
