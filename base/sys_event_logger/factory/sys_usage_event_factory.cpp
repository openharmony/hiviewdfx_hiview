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
#include "sys_usage_event_factory.h"

#include <cinttypes>

#include "battery_stats_client.h"
#include "hilog/log.h"
#include "sys_usage_event.h"
#include "event_cacher.h"
#include "sys_event_common.h"
#include "time_service_client.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, LABEL_DOMAIN, "SysUsageEventFactory" };
constexpr uint64_t TIME_ERROR = 10;
constexpr uint64_t DEFAULT_UINT64_VALUE = 0;
constexpr uint64_t SEC_TO_MILLISEC = 1000;
}
using namespace SysUsageEventSpace;

std::unique_ptr<LoggerEvent> SysUsageEventFactory::Create()
{
    std::unique_ptr<LoggerEvent> event = std::make_unique<SysUsageEvent>(EVENT_DOMAIN, EVENT_NAME, EVENT_TYPE);
    auto nowTime = TimeUtil::GetMilliseconds();
    event->Update(KEY_OF_START, nowTime);
    event->Update(KEY_OF_END, nowTime);

    // correct the time error caused by interface call
    auto monotonicTime = GetMonotonicTime();
    auto bootTime = GetBootTime();
    if (bootTime < (monotonicTime + TIME_ERROR)) {
        bootTime = monotonicTime;
    }

    event->Update(KEY_OF_POWER, bootTime);
    event->Update(KEY_OF_RUNNING, monotonicTime);
    event->Update(KEY_OF_SCREEN, GetScreenTime());
    return event;
}

uint64_t SysUsageEventFactory::GetBootTime()
{
    auto powerTime = MiscServices::TimeServiceClient::GetInstance()->GetBootTimeMs();
    HiLog::Info(LABEL, "get boot time=%{public}" PRId64, powerTime);
    if (powerTime < 0) {
        HiLog::Error(LABEL, "failed to get boot time");
        return DEFAULT_UINT64_VALUE;
    }
    return static_cast<uint64_t>(powerTime);
}

uint64_t SysUsageEventFactory::GetMonotonicTime()
{
    auto runningTime = MiscServices::TimeServiceClient::GetInstance()->GetMonotonicTimeMs();
    HiLog::Info(LABEL, "get runningTime=%{public}" PRId64, runningTime);
    if (runningTime < 0) {
        HiLog::Error(LABEL, "failed to get Monotonic time");
        return DEFAULT_UINT64_VALUE;
    }
    return static_cast<uint64_t>(runningTime);
}

uint64_t SysUsageEventFactory::GetScreenTime()
{
    auto screenTime = PowerMgr::BatteryStatsClient::GetInstance().GetTotalTimeSecond(
        PowerMgr::StatsUtils::STATS_TYPE_SCREEN_ON);
    HiLog::Info(LABEL, "get screenTime=%{public}" PRId64, screenTime);
    return screenTime * SEC_TO_MILLISEC;
}
} // namespace HiviewDFX
} // namespace OHOS
