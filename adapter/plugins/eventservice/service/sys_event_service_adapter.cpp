/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "sys_event_service_adapter.h"

#include "event_loop.h"
#include "hiview_logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-SysEventServiceAdapter");
void SysEventServiceAdapter::StartService(SysEventServiceBase* service, OHOS::HiviewDFX::NotifySysEvent notify)
{
    if (service == nullptr) {
        HIVIEW_LOGE("failed to start SysEventService.");
        return;
    }
    OHOS::HiviewDFX::SysEventServiceOhos::StartService(service, notify);
}

void SysEventServiceAdapter::OnSysEvent(std::shared_ptr<SysEvent> &event)
{
    if (event == nullptr) {
        HIVIEW_LOGE("sys event is nullptr.");
        return;
    }
    auto service = OHOS::HiviewDFX::SysEventServiceOhos::GetInstance();
    if (service == nullptr) {
        HIVIEW_LOGE("SysEventServiceOhos service is null.");
        return;
    }
    service->OnSysEvent(event);
}

void SysEventServiceAdapter::BindGetTagFunc(const GetTagByDomainNameFunc& getTagFunc)
{
    auto service = OHOS::HiviewDFX::SysEventServiceOhos::GetInstance();
    if (service == nullptr) {
        HIVIEW_LOGE("SysEventServiceOhos service is null.");
        return;
    }
    service->BindGetTagFunc(getTagFunc);
}

void SysEventServiceAdapter::BindGetTypeFunc(const GetTypeByDomainNameFunc& getTypeFunc)
{
    auto service = OHOS::HiviewDFX::SysEventServiceOhos::GetInstance();
    if (service == nullptr) {
        HIVIEW_LOGE("SysEventServiceOhos service is null.");
        return;
    }
    service->BindGetTypeFunc(getTypeFunc);
}

void SysEventServiceAdapter::SetWorkLoop(std::shared_ptr<EventLoop> looper)
{
    auto service = OHOS::HiviewDFX::SysEventServiceOhos::GetInstance();
    if (service == nullptr) {
        HIVIEW_LOGE("SetWorkLoop SysEventServiceOhos service is null.");
        return;
    }
    service->SetWorkLoop(looper);
}
}  // namespace HiviewDFX
}  // namespace OHOS