/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "plugin_proxy.h"

#include "logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-PluginProxy");
bool PluginProxy::OnEvent(std::shared_ptr<Event>& event)
{
    LoadPluginIfNeed();
    if (plugin_ != nullptr) {
        return plugin_->OnEvent(event);
    }
    return false;
}

bool PluginProxy::CanProcessEvent(std::shared_ptr<Event> event)
{
    LoadPluginIfNeed();
    if (plugin_ != nullptr) {
        return plugin_->CanProcessEvent(event);
    }
    return false;
}

bool PluginProxy::CanProcessMoreEvents()
{
    LoadPluginIfNeed();
    if (plugin_ != nullptr) {
        return plugin_->CanProcessMoreEvents();
    }
    return false;
}

std::string PluginProxy::GetHandlerInfo()
{
    if (plugin_ != nullptr) {
        return plugin_->GetHandlerInfo();
    }
    return Plugin::GetHandlerInfo();
}

void PluginProxy::Dump(int fd, const std::vector<std::string>& cmds)
{
    if (plugin_ != nullptr) {
        return plugin_->Dump(fd, cmds);
    }
}

void PluginProxy::OnEventListeningCallback(const Event &msg)
{
    LoadPluginIfNeed();
    if (plugin_ == nullptr) {
        return;
    }
    plugin_->OnEventListeningCallback(msg);
}

void PluginProxy::LoadPluginIfNeed()
{
    if (plugin_ != nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(lock_);
    if (plugin_ == nullptr && GetHiviewContext() != nullptr) {
        plugin_ = GetHiviewContext()->InstancePluginByProxy(shared_from_this());
    }

    if (plugin_ == nullptr) {
        // log failure
        HIVIEW_LOGE("Failed to instaniate plugin with name :%{public}s", name_.c_str());
    } else {
        plugin_->UpdateActiveTime();
    }
}

void PluginProxy::DestroyInstanceIfNeed(time_t maxIdleTime)
{
    std::lock_guard<std::mutex> lock(lock_);
    time_t now = time(nullptr);
    if (plugin_ == nullptr) {
        return;
    }
    if (now - plugin_->GetLastActiveTime() >= maxIdleTime) {
        HIVIEW_LOGI("plugin(%{public}s) reach max idle time, unload.", name_.c_str());
        plugin_->OnUnload();
        plugin_ = nullptr;
    }
}
} // namespace HiviewDFX
} // namespace OHOS
