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
#include "app_usage_event.h"

#include "usage_event_common.h"

namespace OHOS {
namespace HiviewDFX {
using namespace AppUsageEventSpace;

AppUsageEvent::AppUsageEvent(const std::string &tag, const std::string &name, HiSysEvent::EventType type)
    : LoggerEvent(tag, name, type)
{
    this->paramMap_ = {
        {KEY_OF_PACKAGE, DEFAULT_STRING}, {KEY_OF_VERSION, DEFAULT_STRING},
        {KEY_OF_USAGE, DEFAULT_UINT64}, {KEY_OF_DATE, DEFAULT_STRING}
    };
}

void AppUsageEvent::Report()
{
    HiSysEvent::Write(this->eventDomain_, this->eventName_, this->eventType_,
        KEY_OF_PACKAGE, this->paramMap_[KEY_OF_PACKAGE].GetString(),
        KEY_OF_VERSION, this->paramMap_[KEY_OF_VERSION].GetString(),
        KEY_OF_USAGE, this->paramMap_[KEY_OF_USAGE].GetUint64(),
        KEY_OF_DATE, this->paramMap_[KEY_OF_DATE].GetString());
}
} // namespace HiviewDFX
} // namespace OHOS
