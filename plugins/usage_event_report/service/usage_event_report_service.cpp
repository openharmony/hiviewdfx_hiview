/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include "usage_event_report_service.h"

#include <algorithm>
#include <getopt.h>

#include "app_usage_event_factory.h"
#include "logger.h"
#include "sys_usage_event_factory.h"
#include "time_util.h"
#include "usage_event_common.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-UsageEventReportService");
using namespace SysUsageEventSpace;
using namespace SysUsageDbSpace;
namespace {
constexpr char ARG_SELECTION[] = "p:t:T:sSA";
const std::string DEFAULT_WORK_PATH = "/data/log/hiview";
const std::string SYS_USAGE_KEYS[] = { KEY_OF_POWER, KEY_OF_RUNNING, KEY_OF_SCREEN };
}

UsageEventReportService::UsageEventReportService() : workPath_(DEFAULT_WORK_PATH), lastReportTime_(0),
    lastSysReportTime_(0)
{}

void UsageEventReportService::InitWorkPath(const char* path)
{
    if (path == nullptr) {
        HIVIEW_LOGE("invalid path, path is nullptr");
        return;
    }
    char realPath[PATH_MAX] = { 0x00 };
    if (strlen(path) >= PATH_MAX || realpath(path, realPath) == nullptr) {
        HIVIEW_LOGE("invalid path, path does not exist");
        return;
    }
    if (std::string realPathStr = realPath; realPathStr.rfind(DEFAULT_WORK_PATH, 0) != 0) {
        HIVIEW_LOGE("invalid path, path should be in the hiview dir");
        return;
    }
    workPath_ = realPath;
}

void UsageEventReportService::ReportAppUsage()
{
    HIVIEW_LOGI("start to report app usage event");
    auto factory = std::make_unique<AppUsageEventFactory>();
    std::vector<std::unique_ptr<LoggerEvent>> events;
    factory->Create(events);
    for (size_t i = 0; i < events.size(); ++i) {
        events[i]->Report();
        HIVIEW_LOGI("report app usage event=%{public}s", events[i]->ToJsonString().c_str());
    }
}

void UsageEventReportService::ReportSysUsage()
{
    HIVIEW_LOGI("start to report sys usage event");
    UsageEventCacher cacher(workPath_);
    auto lastUsage = cacher.GetSysUsageEvent(LAST_SYS_USAGE_COLL);
    if (lastUsage != nullptr) {
        lastSysReportTime_ = lastUsage->GetValue(KEY_OF_END).GetUint64();
    }

    // update last reported time
    std::shared_ptr<LoggerEvent> nowUsage = std::make_unique<SysUsageEventFactory>()->Create();
    nowUsage->Update(KEY_OF_START, lastSysReportTime_);
    cacher.SaveSysUsageEventToDb(nowUsage, LAST_SYS_USAGE_COLL);

    // calc the current usage of this report
    MinusLastSysUsage(nowUsage, lastUsage);

    // add cache usage time if any
    AddCacheSysUsage(nowUsage, cacher.GetSysUsageEvent());
    nowUsage->Report();

    // clear cache event
    cacher.DeleteSysUsageEventFromDb();
}

void UsageEventReportService::AddCacheSysUsage(std::shared_ptr<LoggerEvent>& nowUsage,
    const std::shared_ptr<LoggerEvent>& cacheUsage)
{
    if (nowUsage == nullptr || cacheUsage == nullptr) {
        return;
    }
    for (auto key : SYS_USAGE_KEYS) {
        uint64_t sumUsage = nowUsage->GetValue(key).GetUint64() + cacheUsage->GetValue(key).GetUint64();
        nowUsage->Update(key, sumUsage);
    }
}

void UsageEventReportService::MinusLastSysUsage(std::shared_ptr<LoggerEvent>& nowUsage,
    const std::shared_ptr<LoggerEvent>& lastUsage)
{
    if (nowUsage == nullptr || lastUsage == nullptr) {
        return;
    }
    for (auto key : SYS_USAGE_KEYS) {
        uint64_t nowUsageTime = nowUsage->GetValue(key).GetUint64();
        uint64_t lastUsageTime = lastUsage->GetValue(key).GetUint64();
        uint64_t curUsageTime = nowUsageTime > lastUsageTime ? (nowUsageTime - lastUsageTime) : nowUsageTime;
        nowUsage->Update(key, curUsageTime);
    }
}

void UsageEventReportService::SaveSysUsage()
{
    HIVIEW_LOGI("start to save sys usage event to db");
    std::shared_ptr<LoggerEvent> nowUsage = std::make_unique<SysUsageEventFactory>()->Create();

    // minus the usage time up to the last reported usage if any
    UsageEventCacher cacher(workPath_);
    MinusLastSysUsage(nowUsage, cacher.GetSysUsageEvent(LAST_SYS_USAGE_COLL));

    // add cache usage time if any
    AddCacheSysUsage(nowUsage, cacher.GetSysUsageEvent());

    // save the last reported time
    nowUsage->Update(KEY_OF_START, lastReportTime_);

    // save current usage
    cacher.SaveSysUsageEventToDb(nowUsage);
}

bool UsageEventReportService::ProcessArgsRequest(int argc, char* argv[])
{
    if (argv == nullptr) {
        return false;
    }
    int opt = 0;
    while ((opt = getopt(argc, argv, ARG_SELECTION)) != -1) {
        switch (opt) {
            case 'p':
                InitWorkPath(optarg);
                break;
            case 's':
                SaveSysUsage();
                break;
            case 'A':
                ReportAppUsage();
                break;
            case 'S':
                ReportSysUsage();
                break;
            case 't':
                lastReportTime_ = strtoull(optarg, nullptr, 0);
                break;
            case 'T':
                lastSysReportTime_ = strtoull(optarg, nullptr, 0);
                break;
            default:
                break;
        }
    }
    return true;
}
}  // namespace HiviewDFX
}  // namespace OHOS
