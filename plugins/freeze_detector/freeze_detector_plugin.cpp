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

#include "freeze_detector_plugin.h"

#include "logger.h"
#include "plugin_factory.h"
#include "string_util.h"
#include "sys_event_dao.h"

namespace OHOS {
namespace HiviewDFX {
REGISTER_PROXY(FreezeDetectorPlugin);
DEFINE_LOG_TAG("FreezeDetector");
FreezeDetectorPlugin::FreezeDetectorPlugin()
{
}

FreezeDetectorPlugin::~FreezeDetectorPlugin()
{
}

bool FreezeDetectorPlugin::ReadyToLoad()
{
    freezeCommon_ = std::make_shared<FreezeCommon>();
    bool ret1 = freezeCommon_->Init();
    freezeResolver_ = std::make_unique<FreezeResolver>(freezeCommon_);
    bool ret2 = freezeResolver_->Init();
    return ret1 && ret2;
}

void FreezeDetectorPlugin::OnLoad()
{
    HIVIEW_LOGD("OnLoad.");
    SetName(FREEZE_DETECTOR_PLUGIN_NAME);
    SetVersion(FREEZE_DETECTOR_PLUGIN_VERSION);

    threadLoop_ = GetWorkLoop();
    if (threadLoop_ == nullptr) {
        HIVIEW_LOGW("thread loop is null.");
        return;
    }
    threadLoop_->StartLoop(false);
    freezeCommon_ = std::make_shared<FreezeCommon>();
    bool ret = freezeCommon_->Init();
    if (!ret) {
        HIVIEW_LOGW("freezeCommon_->Init false.");
        freezeCommon_ = nullptr;
        return;
    }
    freezeResolver_ = std::make_unique<FreezeResolver>(freezeCommon_);
    ret = freezeResolver_->Init();
    if (!ret) {
        HIVIEW_LOGW("freezeResolver_->Init false.");
        freezeCommon_ = nullptr;
        freezeResolver_ = nullptr;
    }
}

void FreezeDetectorPlugin::OnUnload()
{
    HIVIEW_LOGD("OnUnload.");
}

bool FreezeDetectorPlugin::OnEvent(std::shared_ptr<Event> &event)
{
    return false;
}

bool FreezeDetectorPlugin::CanProcessEvent(std::shared_ptr<Event> event)
{
    return false;
}

std::string FreezeDetectorPlugin::RemoveRedundantNewline(const std::string& content) const
{
    std::vector<std::string> lines;
    StringUtil::SplitStr(content, "\\n", lines, false, false);

    std::string outContent;
    for (const auto& line : lines) {
        outContent.append(line).append("\n");
    }
    return outContent;
}

WatchPoint FreezeDetectorPlugin::MakeWatchPoint(const Event& event)
{
    Event& eventRef = const_cast<Event&>(event);
    SysEvent& sysEvent = static_cast<SysEvent&>(eventRef);

    long seq = sysEvent.GetSeq();
    long tid = sysEvent.GetTid();
    long pid = sysEvent.GetEventIntValue(FreezeCommon::EVENT_PID);
    pid = pid ? pid : sysEvent.GetPid();
    long uid = sysEvent.GetEventIntValue(FreezeCommon::EVENT_UID);
    uid = uid ? uid : sysEvent.GetUid();
    std::string packageName = sysEvent.GetEventValue(FreezeCommon::EVENT_PACKAGE_NAME);
    std::string processName = sysEvent.GetEventValue(FreezeCommon::EVENT_PROCESS_NAME);
    std::string msg = RemoveRedundantNewline(sysEvent.GetEventValue(FreezeCommon::EVENT_MSG));
    std::string info = sysEvent.GetEventValue(EventStore::EventCol::INFO);
    std::regex reg("logPath:([^,]+)");
    std::smatch result;
    std::string logPath = "";
    if (std::regex_search(info, result, reg)) {
        logPath = result[1].str();
    }
    WatchPoint watchPoint = OHOS::HiviewDFX::WatchPoint::Builder()
        .InitSeq(seq)
        .InitDomain(event.domain_)
        .InitStringId(event.eventName_)
        .InitTimestamp(event.happenTime_)
        .InitPid(pid)
        .InitTid(tid)
        .InitUid(uid)
        .InitPackageName(packageName)
        .InitProcessName(processName)
        .InitMsg(msg)
        .InitLogPath(logPath)
        .Build();
    HIVIEW_LOGI("watchpoint domain=%{public}s, stringid=%{public}s, pid=%{public}ld, uid=%{public}ld, "
        "seq=%{public}ld, packageName=%{public}s, processName=%{public}s, msg=%{public}s logPath=%{public}s.",
        event.domain_.c_str(), event.eventName_.c_str(), pid, uid,
        seq, packageName.c_str(), processName.c_str(), msg.c_str(), logPath.c_str());

    return watchPoint;
}

void FreezeDetectorPlugin::OnEventListeningCallback(const Event& event)
{
    HIVIEW_LOGD("received event id=%{public}u, domain=%{public}s, stringid=%{public}s, extraInfo=%{public}s.",
        event.eventId_, event.domain_.c_str(), event.eventName_.c_str(), event.jsonExtraInfo_.c_str());
    if (freezeCommon_ == nullptr) {
        return;
    }

    if (freezeCommon_->IsFreezeEvent(event.domain_, event.eventName_) == false) {
        HIVIEW_LOGE("not freeze event.");
        return;
    }

    HIVIEW_LOGD("received event domain=%{public}s, stringid=%{public}s",
        event.domain_.c_str(), event.eventName_.c_str());
    HIVIEW_LOGD("threadLoop_->IsRunning() = %{public}d", threadLoop_->IsRunning());
    this->AddUseCount();
    // dispatcher context, send task to our thread
    WatchPoint watchPoint = MakeWatchPoint(event);
    if (watchPoint.GetLogPath().empty()) {
        return;
    }

    std::shared_ptr<FreezeRuleCluster> freezeRuleCluster = freezeCommon_->GetFreezeRuleCluster();
    std::vector<FreezeResult> freezeResultList;
    bool ruleRet = freezeRuleCluster->GetResult(watchPoint, freezeResultList);
    if (!ruleRet) {
        return;
    }
    long delayTime = 0;
    if (freezeResultList.size() > 1) {
        for (auto& i : freezeResultList) {
            long window = i.GetWindow();
            delayTime = std::max(delayTime, window);
        }
        if (delayTime == 0) {
            delayTime = 10; // delay: 10s
        }
    }
    auto task = std::bind(&FreezeDetectorPlugin::ProcessEvent, this, watchPoint);
    threadLoop_->AddTimerEvent(nullptr, nullptr, task, delayTime, false);
}

void FreezeDetectorPlugin::ProcessEvent(WatchPoint watchPoint)
{
    HIVIEW_LOGD("received event domain=%{public}s, stringid=%{public}s",
        watchPoint.GetDomain().c_str(), watchPoint.GetStringId().c_str());
    if (freezeResolver_ == nullptr) {
        this->SubUseCount();
        return;
    }

    auto ret = freezeResolver_->ProcessEvent(watchPoint);
    if (ret < 0) {
        HIVIEW_LOGE("FreezeResolver ProcessEvent filled.");
    }
    this->SubUseCount();
}
} // namespace HiviewDFX
} // namespace OHOS
