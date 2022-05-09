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
#include "hiview_timer_info.h"

#include "hilog/log.h"
#include "sys_event_common.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, LABEL_DOMAIN, "HiViewTimerInfo" };
}
void HiViewTimerInfo::OnTrigger()
{
    HiLog::Info(LABEL, "hiview OnTrigger start");
}

void HiViewTimerInfo::SetType(const int& type)
{
    this->type = type;
}

void HiViewTimerInfo::SetRepeat(bool repeat)
{
    this->repeat = repeat;
}
void HiViewTimerInfo::SetInterval(const uint64_t& interval)
{
    this->interval = interval;
}
void HiViewTimerInfo::SetWantAgent(std::shared_ptr<OHOS::AbilityRuntime::WantAgent::WantAgent> wantAgent)
{
    this->wantAgent = wantAgent;
}
} // namespace HiviewDFX
} // namespace OHOS
