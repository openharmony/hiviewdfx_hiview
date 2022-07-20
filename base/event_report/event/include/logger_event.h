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

#ifndef HIVIEW_BASE_EVENT_REPORT_REPORT_EVENT_H
#define HIVIEW_BASE_EVENT_REPORT_REPORT_EVENT_H

#include <map>
#include <string>

#include "param_value.h"
#include "hisysevent.h"

namespace OHOS {
namespace HiviewDFX {
class LoggerEvent {
public:
    LoggerEvent(const std::string &domain, const std::string &name, HiSysEvent::EventType type)
        : eventDomain_(domain), eventName_(name), eventType_(type) {}

    virtual ~LoggerEvent() {}

    template<typename T>
    void Update(const std::string &name, const T& value)
    {
        InnerUpdate(name, ParamValue(value));
    }

    ParamValue GetValue(const std::string& name);

    std::string ToJsonString();

    virtual void Report() = 0;

protected:
    virtual void InnerUpdate(const std::string &name, const ParamValue& value);

protected:
    std::string eventDomain_;
    std::string eventName_;
    HiSysEvent::EventType eventType_;
    std::map<std::string, ParamValue> paramMap_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_BASE_EVENT_REPORT_REPORT_EVENT_H
