/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "sys_dispatcher.h"
#include <cstdio>
#include <memory>

#include "event.h"
#include "hiview_logger.h"
#include "plugin_factory.h"
#include "sys_event_service_adapter.h"
#include "sys_event.h"

namespace OHOS {
namespace HiviewDFX {
REGISTER(SysEventDispatcher);
DEFINE_LOG_TAG("SysEventDispatcher");

void SysEventDispatcher::OnLoad()
{
    HIVIEW_LOGI("OnLoad.");
}

void SysEventDispatcher::OnUnload()
{
    HIVIEW_LOGI("OnUnload.");
}

void SysEventDispatcher::DispatchEvent(std::shared_ptr<SysEvent>& sysEvent)
{
    auto dispatchList = GetHiviewContext()->GetDisPatcherInfo(sysEvent->eventType_, sysEvent->eventName_,
        sysEvent->GetTag(), sysEvent->domain_);
    for (auto& dispatcher : dispatchList) {
        auto ptr = dispatcher.lock();
        if (ptr != nullptr) {
            ptr->OnEventListeningCallback(*sysEvent);
        }
    }
}

std::shared_ptr<SysEvent> SysEventDispatcher::Convert2SysEvent(std::shared_ptr<Event>& event)
{
    if (event == nullptr) {
        HIVIEW_LOGE("event is null");
        return nullptr;
    }
    if (event->messageType_ != Event::MessageType::SYS_EVENT) {
        HIVIEW_LOGE("receive out of sys event type");
        return nullptr;
    }
    std::shared_ptr<SysEvent> sysEvent = Event::DownCastTo<SysEvent>(event);
    if (sysEvent == nullptr) {
        HIVIEW_LOGE("sysevent is null");
    }
    return sysEvent;
}

bool SysEventDispatcher::OnEvent(std::shared_ptr<Event> &event)
{
    auto sysEvent = Convert2SysEvent(event);
    if (sysEvent == nullptr) {
        return false;
    }
    DispatchEvent(sysEvent);
    SysEventServiceAdapter::OnSysEvent(sysEvent);
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
