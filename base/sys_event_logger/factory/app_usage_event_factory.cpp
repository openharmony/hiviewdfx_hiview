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
#include "app_usage_event_factory.h"

#include <cinttypes>
#include <vector>

#include "app_usage_event.h"
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
#include "bundle_active_client.h"
#endif
#include "hilog/log.h"
#include "os_account_manager.h"
#include "sys_event_common.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, LABEL_DOMAIN, "AppUsageEventFactory" };
constexpr int32_t DEFAULT_USER_ID = 100;
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
constexpr int32_t INTERVAL_TYPE = 1;
constexpr int64_t MILLISEC_TO_SEC = 1000;
#endif
const std::string DATE_FORMAT = "%Y-%m-%d";
}
using namespace AppUsageEventSpace;
using namespace OHOS::AccountSA;
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
using namespace OHOS::DeviceUsageStats;
#endif

std::unique_ptr<LoggerEvent> AppUsageEventFactory::Create()
{
    return std::make_unique<AppUsageEvent>(EVENT_DOMAIN, EVENT_NAME, EVENT_TYPE);
}

void AppUsageEventFactory::Create(std::vector<std::unique_ptr<LoggerEvent>>& events)
{
    // get user ids
    std::vector<int32_t> userIds;
    GetAllCreatedOsAccountIds(userIds);
    if (userIds.empty()) {
        HiLog::Error(LABEL, "the accounts obtained are empty");
        return;
    }

    // get app usage info
    std::vector<AppUsageInfo> appUsageInfos;
    for (auto userId : userIds) {
        GetAppUsageInfosByUserId(appUsageInfos, userId);
    }

    // create events
    for (auto info : appUsageInfos) {
        std::unique_ptr<LoggerEvent> event = Create();
        event->Update(KEY_OF_PACKAGE, info.package_);
        event->Update(KEY_OF_USAGE, info.usage_);
        event->Update(KEY_OF_DATE, info.date_);
        events.push_back(std::move(event));
    }
}

void AppUsageEventFactory::GetAllCreatedOsAccountIds(std::vector<int32_t>& ids)
{
    std::vector<OsAccountInfo> osAccountInfos;
    auto res = OsAccountManager::QueryAllCreatedOsAccounts(osAccountInfos);
    if (res != ERR_OK) {
        HiLog::Error(LABEL, "failed to get userId, err=%{public}d", res);
        ids.push_back(DEFAULT_USER_ID);
        return;
    }
    for (auto info : osAccountInfos) {
        ids.push_back(info.GetLocalId());
    }
}

void AppUsageEventFactory::GetAppUsageInfosByUserId(std::vector<AppUsageInfo>& appUsageInfos, int32_t userId)
{
#ifdef DEVICE_USAGE_STATISTICS_ENABLE
    HiLog::Info(LABEL, "get app usage info by userId=%{public}d", userId);
    int64_t today0Time = TimeUtil::Get0ClockStampMs();
    int64_t gapTime = static_cast<int64_t>(TimeUtil::MILLISECS_PER_DAY);
    int64_t startTime = today0Time > gapTime ? (today0Time - gapTime) : 0;
    int64_t endTime = today0Time > MILLISEC_TO_SEC ? (today0Time - MILLISEC_TO_SEC) : 0;
    int32_t errCode = ERR_OK;
    auto pkgStats = BundleActiveClient::GetInstance().QueryPackageStats(INTERVAL_TYPE, startTime, endTime,
        errCode, userId);
    if (errCode != ERR_OK) {
        HiLog::Error(LABEL, "failed to get package stats, errCode=%{public}d", errCode);
        return;
    }

    HiLog::Info(LABEL, "startTime=%{public}" PRId64 ", endTime=%{public}" PRId64 ", size=%{public}zu",
        startTime, endTime, pkgStats.size());
    std::string dateStr = TimeUtil::TimestampFormatToDate(startTime / MILLISEC_TO_SEC, DATE_FORMAT);
    for (auto stat : pkgStats) {
        std::string package = stat.bundleName_;
        auto it = std::find_if(appUsageInfos.begin(), appUsageInfos.end(), [package](auto info) {
            return info.package_ == package;
        });
        uint64_t usage = stat.totalInFrontTime_ > 0 ? static_cast<uint64_t>(stat.totalInFrontTime_) : 0;
        if (it != appUsageInfos.end()) {
            it->usage_ += usage;
        } else {
            appUsageInfos.push_back(AppUsageInfo(stat.bundleName_, usage, dateStr));
        }
    }
#endif
}
} // namespace HiviewDFX
} // namespace OHOS
