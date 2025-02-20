/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_HIVIEWDFX_EVENT_PUBLISH_H
#define OHOS_HIVIEWDFX_EVENT_PUBLISH_H

#include <mutex>
#include <string>
#include <thread>

#include "json/json.h"
#include "hisysevent.h"
#include "singleton.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
const std::string DOMAIN_OS = "OS";
const std::string EVENT_APP_CRASH = "APP_CRASH";
const std::string EVENT_APP_FREEZE = "APP_FREEZE";
const std::string EVENT_APP_LAUNCH = "APP_LAUNCH";
const std::string EVENT_SCROLL_JANK = "SCROLL_JANK";
const std::string EVENT_CPU_USAGE_HIGH = "CPU_USAGE_HIGH";
const std::string EVENT_BATTERY_USAGE = "BATTERY_USAGE";
const std::string EVENT_RESOURCE_OVERLIMIT = "RESOURCE_OVERLIMIT";
const std::string EVENT_ADDRESS_SANITIZER = "ADDRESS_SANITIZER";
const std::string EVENT_MAIN_THREAD_JANK = "MAIN_THREAD_JANK";
const std::string EVENT_APP_START = "APP_START";
} // namespace HiAppEvent
class EventPublish : public OHOS::DelayedRefSingleton<EventPublish> {
public:
    EventPublish() {};
    ~EventPublish() {};

public:
    void PushEvent(int32_t uid, const std::string& eventName, HiSysEvent::EventType eventType,
        const std::string& paramJson);

private:
    void StartSendingThread();
    void SendEventToSandBox();
    void StartOverLimitThread(int32_t uid, const std::string& eventName,
                              const std::string& pathHolder, Json::Value& eventJson);
    void SendOverLimitEventToSandBox(int32_t uid, const std::string& eventName,
                                     const std::string& pathHolder, Json::Value eventJson);

private:
    std::mutex mutex_;
    std::unique_ptr<std::thread> sendingThread_ = nullptr;
    std::unique_ptr<std::thread> sendingOverlimitThread_ = nullptr;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_EVENT_PUBLISH_H