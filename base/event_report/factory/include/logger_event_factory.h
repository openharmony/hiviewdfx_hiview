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

#ifndef HIVIEW_BASE_EVENT_REPORT_LOGGER_EVENT_FACTORY_H
#define HIVIEW_BASE_EVENT_REPORT_LOGGER_EVENT_FACTORY_H

#include <memory>
#include <string>
#include <vector>

#include "logger_event.h"

namespace OHOS {
namespace HiviewDFX {
class LoggerEventFactory {
public:
    virtual std::unique_ptr<LoggerEvent> Create() = 0;
    virtual void Create(std::vector<std::unique_ptr<LoggerEvent>>& events) {}
    virtual ~LoggerEventFactory() {}
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_BASE_EVENT_REPORT_LOGGER_EVENT_FACTORY_H
