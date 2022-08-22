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

#ifndef HIVIEW_PLUGINS_USAGE_EVENT_REPORT_INCLUDE_USAGE_EVENT_REPORT_H
#define HIVIEW_PLUGINS_USAGE_EVENT_REPORT_INCLUDE_USAGE_EVENT_REPORT_H

#include <memory>
#include <string>

#include "event_loop.h"
#include "ishutdown_callback.h"
#include "plugin.h"

namespace OHOS {
namespace HiviewDFX {
class UsageEventReport : public Plugin {
public:
    UsageEventReport();
    ~UsageEventReport() {}
    void OnLoad() override;
    void OnUnload() override;
    void TimeOut();
    static void SaveEventToDb();

private:
    void Init();
    void InitCallback();
    void Start();
    void ReportDailyEvent();
    void ReportTimeOutEvent();
    void ReportSysUsageEvent();
    void DeletePluginStatsEvents();
    static void StartServiceByOption(const std::string& opt);
    static void SavePluginStatsEvents();
    static void SaveSysUsageEvent();

private:
    sptr<PowerMgr::IShutdownCallback> callback_;
    uint64_t timeOutCnt_;
    static uint64_t lastSysReportTime_;
    static uint64_t lastReportTime_;
    static uint64_t nextReportTime_;
    static std::string workPath_;
}; // UsageEventReport
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_USAGE_EVENT_REPORT_INCLUDE_USAGE_EVENT_REPORT_H
