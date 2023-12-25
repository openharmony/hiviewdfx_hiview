/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include "unified_collector.h"

#include "ffrt.h"
#include "file_util.h"
#include "io_collector.h"
#include "logger.h"
#include "plugin_factory.h"
#include "process_status.h"
#include "sys_event.h"
#include "unified_collection_stat.h"

namespace OHOS {
namespace HiviewDFX {
REGISTER(UnifiedCollector);
DEFINE_LOG_TAG("HiView-UnifiedCollector");
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace std::literals::chrono_literals;
namespace {
const std::unordered_map<std::string, ProcessState> APP_STATES = {
    {"APP_FOREGROUND", FOREGROUND},
    {"APP_BACKGROUND", BACKGROUND},
};

ProcessState GetProcessStateByEvent(const SysEvent& sysEvent)
{
    std::string eventName = sysEvent.GetEventName();
    if (APP_STATES.find(eventName) != APP_STATES.end()) {
        return APP_STATES.at(eventName);
    }
    HIVIEW_LOGW("invalid event name=%{public}s", eventName.c_str());
    return INVALID;
}
}

void UnifiedCollector::OnLoad()
{
    HIVIEW_LOGI("start to load UnifiedCollector plugin");
    Init();
}

void UnifiedCollector::OnUnload()
{
    HIVIEW_LOGI("start to unload UnifiedCollector plugin");
}

void UnifiedCollector::OnEventListeningCallback(const Event& event)
{
    SysEvent& sysEvent = static_cast<SysEvent&>(const_cast<Event&>(event));
    int32_t procId = sysEvent.GetEventIntValue("APP_PID");
    if (procId <= 0) {
        HIVIEW_LOGW("invalid process id=%{public}d", procId);
        return;
    }
    ProcessState procState = GetProcessStateByEvent(sysEvent);
    if (procState == INVALID) {
        HIVIEW_LOGW("invalid process state=%{public}d", procState);
        return;
    }
    ProcessStatus::GetInstance().NotifyProcessState(procId, procState);
}

void UnifiedCollector::Init()
{
    if (GetHiviewContext() == nullptr) {
        HIVIEW_LOGE("hiview context is null");
        return;
    }
    InitWorkLoop();
    InitWorkPath();
    RunCpuCollectionTask();
    RunIoCollectionTask();
    RunUCollectionStatTask();
}

void UnifiedCollector::InitWorkLoop()
{
    workLoop_ = GetHiviewContext()->GetSharedWorkLoop();
}

void UnifiedCollector::InitWorkPath()
{
    std::string hiviewWorkDir = GetHiviewContext()->GetHiViewDirectory(HiviewContext::DirectoryType::WORK_DIRECTORY);
    const std::string uCollectionDirName = "unified_collection";
    std::string tempWorkPath = FileUtil::IncludeTrailingPathDelimiter(hiviewWorkDir.append(uCollectionDirName));
    if (!FileUtil::IsDirectory(tempWorkPath) && !FileUtil::ForceCreateDirectory(tempWorkPath)) {
        HIVIEW_LOGE("failed to create dir=%{public}s", tempWorkPath.c_str());
        return;
    }
    workPath_ = tempWorkPath;
}

void UnifiedCollector::RunCpuCollectionTask()
{
    if (workPath_.empty()) {
        HIVIEW_LOGE("workPath is null");
        return;
    }
    auto task = std::bind(&UnifiedCollector::CpuCollectionFfrtTask, this);
    ffrt::submit(task, {}, {}, ffrt::task_attr().name("UC_CPU").qos(ffrt::qos_default));
}

void UnifiedCollector::CpuCollectionFfrtTask()
{
    cpuCollectionTask_ = std::make_shared<CpuCollectionTask>(workPath_);
    while (true) {
        ffrt::this_task::sleep_for(10s); // 10s: collect period
        cpuCollectionTask_->Collect();
    }
}

void UnifiedCollector::RunIoCollectionTask()
{
    if (workLoop_ == nullptr) {
        HIVIEW_LOGE("workLoop is null");
        return;
    }
    auto ioCollectionTask = std::bind(&UnifiedCollector::IoCollectionTask, this);
    const uint64_t taskInterval = 30; // 30s
    workLoop_->AddTimerEvent(nullptr, nullptr, ioCollectionTask, taskInterval, true);
}

void UnifiedCollector::IoCollectionTask()
{
    auto ioCollector = UCollectUtil::IoCollector::Create();
    (void)ioCollector->CollectDiskStats([](const DiskStats &stats) { return false; }, true);
    (void)ioCollector->CollectAllProcIoStats(true);
}

void UnifiedCollector::RunUCollectionStatTask()
{
    if (workLoop_ == nullptr) {
        HIVIEW_LOGE("workLoop is null");
        return;
    }
    auto statTask = std::bind(&UnifiedCollector::UCollectionStatTask, this);
    const uint64_t taskInterval = 600; // 600s
    workLoop_->AddTimerEvent(nullptr, nullptr, statTask, taskInterval, true);
}

void UnifiedCollector::UCollectionStatTask()
{
    UnifiedCollectionStat stat;
    stat.Report();
}
} // namespace HiviewDFX
} // namespace OHOS
