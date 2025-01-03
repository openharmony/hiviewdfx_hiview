/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "hiview_logger.h"
#include "sys_usage_event.h"
#include "time_service_client.h"
#include "time_util.h"
#include "usage_event_common.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-SysUsageEventFactory");
namespace {
constexpr uint64_t TIME_ERROR = 10;
}
using namespace SysUsageEventSpace;

std::unique_ptr<LoggerEvent> SysUsageEventFactory::Create()
{
    std::unique_ptr<LoggerEvent> event =
        std::make_unique<SysUsageEvent>(EVENT_NAME, HiSysEvent::STATISTIC);
    event->Update(KEY_OF_START, DEFAULT_UINT64);
    event->Update(KEY_OF_END, TimeUtil::GetMilliseconds());

    // correct the time error caused by interface call
    auto monotonicTime = GetMonotonicTime();
    auto bootTime = GetBootTime();
    if (bootTime < (monotonicTime + TIME_ERROR)) {
        bootTime = monotonicTime;
    }

    event->Update(KEY_OF_POWER, bootTime);
    event->Update(KEY_OF_RUNNING, monotonicTime);
    return event;
}

uint64_t SysUsageEventFactory::GetBootTime()
{
    auto powerTime = MiscServices::TimeServiceClient::GetInstance()->GetBootTimeMs();
    HIVIEW_LOGI("get boot time=%{public}" PRId64, powerTime);
    if (powerTime < 0) {
        HIVIEW_LOGE("failed to get boot time");
        return DEFAULT_UINT64;
    }
    return static_cast<uint64_t>(powerTime);
}

uint64_t SysUsageEventFactory::GetMonotonicTime()
{
    auto runningTime = MiscServices::TimeServiceClient::GetInstance()->GetMonotonicTimeMs();
    HIVIEW_LOGI("get runningTime=%{public}" PRId64, runningTime);
    if (runningTime < 0) {
        HIVIEW_LOGE("failed to get Monotonic time");
        return DEFAULT_UINT64;
    }
    return static_cast<uint64_t>(runningTime);
}
} // namespace HiviewDFX
} // namespace OHOS
