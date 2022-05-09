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

#ifndef HIVIEW_PLUGINS_SYS_EVENT_LOGGER_HIVIEW_TIMER_INFO_H
#define HIVIEW_PLUGINS_SYS_EVENT_LOGGER_HIVIEW_TIMER_INFO_H

#include <memory>
#include <string>
#include <vector>

#include "time_service_client.h"

namespace OHOS {
namespace HiviewDFX {
class HiViewTimerInfo : public MiscServices::ITimerInfo {
public:
    HiViewTimerInfo() {}
    virtual ~HiViewTimerInfo() {}
    virtual void OnTrigger() override;
    virtual void SetType(const int &type) override;
    virtual void SetRepeat(bool repeat) override;
    virtual void SetInterval(const uint64_t &interval) override;
    virtual void SetWantAgent(std::shared_ptr<OHOS::AbilityRuntime::WantAgent::WantAgent> wantAgent) override;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_SYS_EVENT_LOGGER_HIVIEW_TIMER_INFO_H
