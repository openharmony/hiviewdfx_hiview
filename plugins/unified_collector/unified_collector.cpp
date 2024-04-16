/*
 * Copyright (C) 2023-2024 Huawei Device Co., Ltd.
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
#include "hitrace_dump.h"
#include "io_collector.h"
#include "logger.h"
#include "parameter_ex.h"
#include "plugin_factory.h"
#include "process_status.h"
#include "sys_event.h"
#include "unified_collection_stat.h"
#include "utility/trace_collector.h"

namespace OHOS {
namespace HiviewDFX {
REGISTER(UnifiedCollector);
DEFINE_LOG_TAG("HiView-UnifiedCollector");
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace std::literals::chrono_literals;
using namespace OHOS::HiviewDFX::Hitrace;
namespace {
std::mutex g_traceLock;
const std::string HIPERF_LOG_PATH = "/data/log/hiperf";
const std::string COLLECTION_IO_PATH = "/data/log/hiview/unified_collection/io/";
const std::string UNIFIED_SPECIAL_PATH = "/data/log/hiview/unified_collection/trace/special/";
const std::string OTHER = "Other";
#if PC_APP_STATE_COLLECT_ENABLE
const std::string RSS_APP_STATE_EVENT = "APP_CGROUP_CHANGE";
const int NAP_BACKGROUND_GROUP = 11;

ProcessState GetProcessStateByGroup(SysEvent& sysEvent)
{
    if (sysEvent.GetEventName() != RSS_APP_STATE_EVENT) {
        return INVALID;
    }
    int32_t procGroup = sysEvent.GetEventIntValue("PROCESS_NEWGROUP");
    if (procGroup == NAP_BACKGROUND_GROUP) {
        return BACKGROUND;
    }
    // else - The app is in the foreground group
    return FOREGROUND;
}
#endif
}

void UnifiedCollector::OnLoad()
{
    HIVIEW_LOGI("start to load UnifiedCollector plugin");
    UCollectUtil::TraceCollector::RecoverTmpTrace();
    Init();
}

void UnifiedCollector::OnUnload()
{
    HIVIEW_LOGI("start to unload UnifiedCollector plugin");
    observerMgr_ = nullptr;
}

void UnifiedCollector::OnEventListeningCallback(const Event& event)
{
#if PC_APP_STATE_COLLECT_ENABLE
    SysEvent& sysEvent = static_cast<SysEvent&>(const_cast<Event&>(event));
    int32_t procId = sysEvent.GetEventIntValue("APP_PID");
    if (procId <= 0) {
        HIVIEW_LOGW("invalid process id=%{public}d", procId);
        return;
    }
    ProcessState procState = GetProcessStateByGroup(sysEvent);
    if (procState == INVALID) {
        HIVIEW_LOGD("invalid process state=%{public}d", procState);
        return;
    }
    ProcessStatus::GetInstance().NotifyProcessState(procId, procState);
#endif
}

void UnifiedCollector::Init()
{
    if (GetHiviewContext() == nullptr) {
        HIVIEW_LOGE("hiview context is null");
        return;
    }
    InitWorkLoop();
    InitWorkPath();
    bool isAllowCollect = Parameter::IsBetaVersion() || Parameter::IsUCollectionSwitchOn();
    if (isAllowCollect) {
        RunIoCollectionTask();
        RunUCollectionStatTask();
        LoadHitraceService();
    }
    if (isAllowCollect || Parameter::IsDeveloperMode()) {
        RunCpuCollectionTask();
    }
    if (!Parameter::IsBetaVersion()) {
        int ret = Parameter::WatchParamChange(HIVIEW_UCOLLECTION_STATE, OnSwitchStateChanged, this);
        HIVIEW_LOGI("add ucollection switch param watcher ret: %{public}d", ret);
    }
    observerMgr_ = std::make_shared<UcObserverManager>();
}

void UnifiedCollector::CleanDataFiles()
{
    std::vector<std::string> files;
    FileUtil::GetDirFiles(UNIFIED_SPECIAL_PATH, files);
    for (const auto& file : files) {
        if (file.find(OTHER) != std::string::npos) {
            FileUtil::RemoveFile(file);
        }
    }
    FileUtil::ForceRemoveDirectory(COLLECTION_IO_PATH, false);
    FileUtil::ForceRemoveDirectory(HIPERF_LOG_PATH, false);
}

void UnifiedCollector::OnSwitchStateChanged(const char* key, const char* value, void* context)
{
    if (context == nullptr || key == nullptr || value == nullptr) {
        HIVIEW_LOGE("input ptr null");
        return;
    }
    if (strncmp(key, HIVIEW_UCOLLECTION_STATE, strlen(HIVIEW_UCOLLECTION_STATE)) != 0) {
        HIVIEW_LOGE("param key error");
        return;
    }
    HIVIEW_LOGI("ucollection switch state changed, ret: %{public}s", value);
    UnifiedCollector* unifiedCollectorPtr = static_cast<UnifiedCollector*>(context);
    if (unifiedCollectorPtr == nullptr) {
        HIVIEW_LOGE("unifiedCollectorPtr is null");
        return;
    }
    if (strncmp(value, "true", strlen("true")) == 0) {
        unifiedCollectorPtr->RunCpuCollectionTask();
        unifiedCollectorPtr->RunIoCollectionTask();
        unifiedCollectorPtr->RunUCollectionStatTask();
        unifiedCollectorPtr->LoadHitraceService();
    } else {
        if (!Parameter::IsDeveloperMode()) {
            unifiedCollectorPtr->isCpuTaskRunning_ = false;
        }
        for (const auto &it : unifiedCollectorPtr->taskList_) {
            unifiedCollectorPtr->workLoop_->RemoveEvent(it);
        }
        unifiedCollectorPtr->taskList_.clear();
        unifiedCollectorPtr->ExitHitraceService();
        unifiedCollectorPtr->CleanDataFiles();
    }
}

void UnifiedCollector::LoadHitraceService()
{
    std::lock_guard<std::mutex> lock(g_traceLock);
    HIVIEW_LOGI("start to load hitrace service.");
    uint8_t mode = GetTraceMode();
    if (mode != TraceMode::CLOSE) {
        HIVIEW_LOGE("service is running, mode=%{public}u.", mode);
        return;
    }
    const std::vector<std::string> tagGroups = {"scene_performance"};
    TraceErrorCode ret = OpenTrace(tagGroups);
    if (ret != TraceErrorCode::SUCCESS) {
        HIVIEW_LOGE("OpenTrace fail.");
    }
}

void UnifiedCollector::ExitHitraceService()
{
    std::lock_guard<std::mutex> lock(g_traceLock);
    HIVIEW_LOGI("exit hitrace service.");
    uint8_t mode = GetTraceMode();
    if (mode == TraceMode::CLOSE) {
        HIVIEW_LOGE("service is close mode.");
        return;
    }
    CloseTrace();
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
    if (workPath_.empty() || isCpuTaskRunning_) {
        HIVIEW_LOGE("workPath is null or task is running");
        return;
    }
    isCpuTaskRunning_ = true;
    auto task = std::bind(&UnifiedCollector::CpuCollectionFfrtTask, this);
    ffrt::submit(task, {}, {}, ffrt::task_attr().name("UC_CPU").qos(ffrt::qos_default));
}

void UnifiedCollector::CpuCollectionFfrtTask()
{
    cpuCollectionTask_ = std::make_shared<CpuCollectionTask>(workPath_);
    while (true) {
        if (!isCpuTaskRunning_) {
            HIVIEW_LOGE("exit cpucollection task");
            break;
        }
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
    auto ioSeqId = workLoop_->AddTimerEvent(nullptr, nullptr, ioCollectionTask, taskInterval, true);
    taskList_.push_back(ioSeqId);
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
    auto statSeqId = workLoop_->AddTimerEvent(nullptr, nullptr, statTask, taskInterval, true);
    taskList_.push_back(statSeqId);
}

void UnifiedCollector::UCollectionStatTask()
{
    UnifiedCollectionStat stat;
    stat.Report();
}
} // namespace HiviewDFX
} // namespace OHOS
