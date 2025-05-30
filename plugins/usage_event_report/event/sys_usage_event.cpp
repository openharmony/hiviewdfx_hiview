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
#include "sys_usage_event.h"

#include "hiview_logger.h"
#include "usage_event_common.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("SysUsageEvent");
using namespace SysUsageEventSpace;

SysUsageEvent::SysUsageEvent(const std::string &name, HiSysEvent::EventType type)
    : LoggerEvent(name, type)
{
    this->paramMap_ = {
        {KEY_OF_START, DEFAULT_UINT64}, {KEY_OF_END, DEFAULT_UINT64},
        {KEY_OF_POWER, DEFAULT_UINT64}, {KEY_OF_RUNNING, DEFAULT_UINT64}
    };
}

void SysUsageEvent::Report()
{
    ReportDFX();
    ReportDFXUE();
}

void SysUsageEvent::ReportDFX()
{
    auto ret = HiSysEventWrite(HiSysEvent::Domain::HIVIEWDFX, this->eventName_, this->eventType_,
        KEY_OF_START, this->paramMap_[KEY_OF_START].GetUint64(),
        KEY_OF_END, this->paramMap_[KEY_OF_END].GetUint64(),
        KEY_OF_POWER, this->paramMap_[KEY_OF_POWER].GetUint64(),
        KEY_OF_RUNNING, this->paramMap_[KEY_OF_RUNNING].GetUint64());
    if (ret != 0) {
        HIVIEW_LOGW("failed to report sys usage event, ret=%{public}d", ret);
    }
}

void SysUsageEvent::ReportDFXUE()
{
    auto ret = HiSysEventWrite(DomainSpace::HIVIEWDFX_UE_DOMAIN, this->eventName_, this->eventType_,
        KEY_OF_START, this->paramMap_[KEY_OF_START].GetUint64(),
        KEY_OF_END, this->paramMap_[KEY_OF_END].GetUint64(),
        KEY_OF_POWER, this->paramMap_[KEY_OF_POWER].GetUint64(),
        KEY_OF_RUNNING, this->paramMap_[KEY_OF_RUNNING].GetUint64());
    if (ret != 0) {
        HIVIEW_LOGW("failed to report sys usage event, ret=%{public}d", ret);
    }
}
} // namespace HiviewDFX
} // namespace OHOS
