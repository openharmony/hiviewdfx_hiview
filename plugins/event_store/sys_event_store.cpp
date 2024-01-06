/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "sys_event_store.h"

#include <cstdio>
#include <memory>

#include "event.h"
#include "hiview_global.h"
#include "logger.h"
#include "plugin_factory.h"
#include "sys_event.h"
#include "sys_event_db_mgr.h"
#include "hiview_platform.h"

namespace OHOS {
namespace HiviewDFX {
REGISTER(SysEventStore);
DEFINE_LOG_TAG("HiView-SysEventStore");
SysEventStore::SysEventStore() : hasLoaded_(false)
{
    sysEventDbMgr_ = std::make_unique<SysEventDbMgr>();
}

SysEventStore::~SysEventStore() {}

void SysEventStore::OnLoad()
{
    HIVIEW_LOGI("sys event service load");
    sysEventDbMgr_->StartCheckStoreTask(this->workLoop_);
    auto &platform = HiviewPlatform::GetInstance();
    sysEventParser_ = platform.GetEventJsonParser();
    if (sysEventParser_ == nullptr) {
        hasLoaded_ = false;
        return;
    }
    auto getTagFunc = std::bind(&EventJsonParser::GetTagByDomainAndName, *(sysEventParser_.get()),
        std::placeholders::_1, std::placeholders::_2);
    SysEventServiceAdapter::BindGetTagFunc(getTagFunc);
    auto getTypeFunc = std::bind(&EventJsonParser::GetTypeByDomainAndName, *(sysEventParser_.get()),
        std::placeholders::_1, std::placeholders::_2);
    SysEventServiceAdapter::BindGetTypeFunc(getTypeFunc);
    hasLoaded_ = true;
}

void SysEventStore::OnUnload()
{
    HIVIEW_LOGI("sys event service unload");
}

std::shared_ptr<SysEvent> SysEventStore::Convert2SysEvent(std::shared_ptr<Event>& event)
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

bool SysEventStore::OnEvent(std::shared_ptr<Event>& event)
{
    if (!hasLoaded_) {
        HIVIEW_LOGE("SysEventService not ready");
        return false;
    }
    std::shared_ptr<SysEvent> sysEvent = Convert2SysEvent(event);
    if (sysEvent->preserve_) {
        sysEventDbMgr_->SaveToStore(sysEvent);
    }
    return true;
}

} // namespace HiviewDFX
} // namespace OHOS
