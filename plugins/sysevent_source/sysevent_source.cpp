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

#include "sysevent_source.h"

#include <functional>
#include <memory>

#include "decoded/decoded_event.h"
#include "defines.h"
#include "event_export_engine.h"
#include "hiview_logger.h"
#include "plugin_factory.h"
#include "time_util.h"
#include "sys_event.h"
#include "hiview_platform.h"
#include "dispatch_config.h"
#include "sys_event_dao.h"

namespace OHOS {
namespace HiviewDFX {
REGISTER(SysEventSource);
DEFINE_LOG_TAG("HiView-SysEventSource");

void SysEventReceiver::HandlerEvent(std::shared_ptr<EventRaw::RawData> rawData)
{
    if (rawData == nullptr || rawData->GetData() == nullptr) {
        HIVIEW_LOGW("raw data of sys event is null");
        return;
    }
    std::shared_ptr<PipelineEvent> event = std::make_shared<SysEvent>("SysEventSource",
        static_cast<PipelineEventProducer*>(&eventSource), rawData);
    if (eventSource.CheckEvent(event)) {
        eventSource.PublishPipelineEvent(event);
    }
}

void SysEventSource::OnLoad()
{
    HIVIEW_LOGI("SysEventSource load ");
    std::shared_ptr<EventLoop> looper = GetHiviewContext()->GetSharedWorkLoop();
    platformMonitor_.StartMonitor(looper);
    sysEventParser_ = HiviewPlatform::GetInstance().GetEventJsonParser();
    sysEventStat_ = std::make_unique<SysEventStat>();
    EventExportEngine::GetInstance().Start();
}

void SysEventSource::OnUnload()
{
    eventServer_.Stop();
    HIVIEW_LOGI("SysEventSource unload");
    EventExportEngine::GetInstance().Stop();
}

void SysEventSource::StartEventSource()
{
    HIVIEW_LOGI("SysEventSource start");
    std::shared_ptr<EventReceiver> sysEventReceiver = std::make_shared<SysEventReceiver>(*this);
    eventServer_.AddReceiver(sysEventReceiver);
    eventServer_.Start();
}

void SysEventSource::Recycle(PipelineEvent *event)
{
    platformMonitor_.CollectCostTime(event);
}

void SysEventSource::PauseDispatch(std::weak_ptr<Plugin> plugin)
{
    auto requester = plugin.lock();
    if (requester != nullptr) {
        HIVIEW_LOGI("process pause dispatch event from plugin:%s.\n", requester->GetName().c_str());
    }
}

bool SysEventSource::PublishPipelineEvent(std::shared_ptr<PipelineEvent> event)
{
    platformMonitor_.CollectEvent(event);
    platformMonitor_.Breaking();
    auto &platform = HiviewPlatform::GetInstance();
    auto const &pipelineRules = platform.GetPipelineConfigMap();
    auto const &pipelineMap = platform.GetPipelineMap();
    for (auto it = pipelineRules.begin(); it != pipelineRules.end(); it++) {
        std::string pipelineName = it->first;
        auto dispathRule = it->second;
        if (dispathRule->FindEvent(event->domain_, event->eventName_)) {
            pipelineMap.at(pipelineName)->ProcessEvent(event);
            return true;
        }
    }
    pipelineMap.at("SysEventPipeline")->ProcessEvent(event);
    return true;
}

bool SysEventSource::CheckEvent(std::shared_ptr<Event> event)
{
    std::shared_ptr<SysEvent> sysEvent = Convert2SysEvent(event);
    if (sysEvent == nullptr || sysEventParser_ == nullptr) {
        HIVIEW_LOGE("event or event parser is null.");
        sysEventStat_->AccumulateEvent(false);
        return false;
    }
    EventStore::SysEventDao::CheckRepeat(sysEvent);
    if (!sysEventParser_->HandleEventJson(sysEvent)) {
        sysEventStat_->AccumulateEvent(sysEvent->domain_, sysEvent->eventName_, false);
        return false;
    }
    HIVIEW_LOGD("event[%{public}s|%{public}s|%{public}" PRId64 "] is valid.",
        sysEvent->domain_.c_str(), sysEvent->eventName_.c_str(), sysEvent->GetEventSeq());
    sysEventStat_->AccumulateEvent();
    return true;
}

std::shared_ptr<SysEvent> SysEventSource::Convert2SysEvent(std::shared_ptr<Event>& event)
{
    if (event == nullptr) {
        HIVIEW_LOGE("event is null");
        return nullptr;
    }
    if (event->messageType_ != Event::MessageType::SYS_EVENT) {
        HIVIEW_LOGE("receive out of sys event type");
        return nullptr;
    }
    return Event::DownCastTo<SysEvent>(event);
}

static void ShowUsage(int fd, const std::vector<std::string>& cmds)
{
    dprintf(fd, "invalid cmd:");
    for (auto it = cmds.begin(); it != cmds.end(); it++) {
        dprintf(fd, "%s ", it->c_str());
    }
    dprintf(fd, "\n");
    dprintf(fd, "usage: SysEventService [sum|detail|invalid|clear]\n");
}

void SysEventSource::Dump(int fd, const std::vector<std::string>& cmds)
{
    if (cmds.size() >= 2) { // 2：args from the second item
        std::string arg1 = cmds[1];
        if (arg1 == "sum") {
            sysEventStat_->StatSummary(fd);
        } else if (arg1 == "detail") {
            sysEventStat_->StatDetail(fd);
        } else if (arg1 == "invalid") {
            sysEventStat_->StatInvalidDetail(fd);
        } else if (arg1 == "clear") {
            sysEventStat_->Clear(fd);
        } else {
            ShowUsage(fd, cmds);
        }
    } else {
        sysEventStat_->StatSummary(fd);
    }
}
} // namespace HiviewDFX
} // namespace OHOS
