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

#include <memory>

#include "app_caller_event.h"
#include "collect_event.h"
#include "event_publish.h"
#include "ffrt.h"
#include "file_util.h"
#include "hitrace_dump.h"
#include "hiview_logger.h"
#include "io_collector.h"
#include "json/json.h"
#include "parameter_ex.h"
#include "plugin_factory.h"
#include "process_status.h"
#include "sys_event.h"
#include "time_util.h"
#include "trace_flow_controller.h"
#include "trace_manager.h"
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
const std::string UNIFIED_SHARE_PATH = "/data/log/hiview/unified_collection/trace/share/";
const std::string DEVELOPER_MODE_TRACE_ARGS = "tags:memory clockType:boot1 bufferSize:1024 overwrite:0";
const std::string OTHER = "Other";
const std::string HIVIEW_UCOLLECTION_STATE_TRUE = "true";
const std::string HIVIEW_UCOLLECTION_STATE_FALSE = "false";
const std::string DEVELOP_TRACE_RECORDER_TRUE = "true";
const std::string DEVELOP_TRACE_RECORDER_FALSE = "false";

const int8_t STATE_COUNT = 2;
const int8_t COML_STATE = 0;
// [coml:0][is remote log][is trace recorder]
// [beta:1][is remote log][is trace recorder]
const bool DYNAMIC_TRACE_FSM[STATE_COUNT][STATE_COUNT][STATE_COUNT] = {
    {{true,  false}, {false, false}},
    {{false, false}, {false, false}},
};

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
#endif // PC_APP_STATE_COLLECT_ENABLE

bool StartCatpureAppTrace(std::shared_ptr<AppCallerEvent> appJankEvent)
{
    TraceManager manager;
    std::string appArgs = "tags:graphic,ace,app clockType:boot bufferSize:1024 overwrite:1";
    int32_t retCode = manager.OpenRecordingTrace(appArgs);
    if (retCode != UCollect::UcError::SUCCESS) {
        manager.CloseTrace();
        HIVIEW_LOGE("failed to open trace in recording mode, error code %{public}d", retCode);
        appJankEvent->resultCode_ = UCollect::UcError(retCode);
        return false;
    }

    std::shared_ptr<UCollectUtil::TraceCollector> traceCollector = UCollectUtil::TraceCollector::Create();
    CollectResult<int32_t> result = traceCollector->TraceOn();
    if (result.retCode != UCollect::UcError::SUCCESS) {
        manager.CloseTrace();
        HIVIEW_LOGE("failed to trace on, error code %{public}d", result.retCode);
        appJankEvent->resultCode_ = result.retCode;
        return false;
    }
    appJankEvent->resultCode_ = 0;
    return true;
}

std::string MakeTraceFileName(const std::string &bundleName, int32_t pid,
    int64_t beginTime, int64_t endTime, int32_t costTime)
{
    std::string d1 = TimeUtil::TimestampFormatToDate(beginTime/ TimeUtil::SEC_TO_MILLISEC, "%Y%m%d%H%M%S");
    std::string d2 = TimeUtil::TimestampFormatToDate(endTime/ TimeUtil::SEC_TO_MILLISEC, "%Y%m%d%H%M%S");

    std::string name;
    name.append(UNIFIED_SHARE_PATH).append("APP_").append(bundleName).append("_").append(std::to_string(pid));
    name.append("_").append(d1).append("_").append(d2).append("_").append(std::to_string(costTime)).append(".sys");
    return name;
}

void StopRecordAppTrace(std::shared_ptr<AppCallerEvent> appJankEvent)
{
    std::shared_ptr<UCollectUtil::TraceCollector> traceCollector = UCollectUtil::TraceCollector::Create();
    CollectResult<std::vector<std::string>> result = traceCollector->TraceOff();
    if (result.retCode != UCollect::UcError::SUCCESS) {
        HIVIEW_LOGE("trace off for uid=%{public}d pid=%{public}d error code=%{public}d",
            appJankEvent->uid_, appJankEvent->pid_, result.retCode);
    }

    TraceManager manager;
    int32_t retCode = manager.CloseTrace();
    if (retCode != UCollect::UcError::SUCCESS) {
        HIVIEW_LOGE("close trace for uid=%{public}d pid=%{public}d error code=%{public}d",
            appJankEvent->uid_, appJankEvent->pid_, result.retCode);
    }
    appJankEvent->taskEndTime_ = static_cast<int64_t>(TimeUtil::GetMilliseconds());

    if (result.data.empty()) {
        HIVIEW_LOGE("failed to collect app trace for uid=%{public}d pid=%{public}d",
            appJankEvent->uid_, appJankEvent->pid_);
    } else {
        std::string traceFileName = MakeTraceFileName(appJankEvent->bundleName_,
            appJankEvent->pid_, appJankEvent->taskBeginTime_, appJankEvent->taskEndTime_,
            (appJankEvent->taskEndTime_ - appJankEvent->taskBeginTime_));
        FileUtil::RenameFile(result.data[0], traceFileName);
        appJankEvent->externalLog_ = traceFileName;
    }
}

void ShareAppEvent(std::shared_ptr<AppCallerEvent> appJankEvent)
{
    Json::Value eventJson;
    eventJson[UCollectUtil::APP_EVENT_PARAM_UID] = appJankEvent->uid_;
    eventJson[UCollectUtil::APP_EVENT_PARAM_PID] = appJankEvent->pid_;
    eventJson[UCollectUtil::APP_EVENT_PARAM_TIME] = appJankEvent->happenTime_;
    eventJson[UCollectUtil::APP_EVENT_PARAM_BUNDLE_NAME] = appJankEvent->bundleName_;
    eventJson[UCollectUtil::APP_EVENT_PARAM_BUNDLE_VERSION] = appJankEvent->bundleVersion_;
    eventJson[UCollectUtil::APP_EVENT_PARAM_BEGIN_TIME] = appJankEvent->beginTime_;
    eventJson[UCollectUtil::APP_EVENT_PARAM_END_TIME] = appJankEvent->endTime_;
    eventJson[UCollectUtil::APP_EVENT_PARAM_EXTERNAL_LOG] = appJankEvent->externalLog_;
    std::string param = Json::FastWriter().write(eventJson);

    HIVIEW_LOGI("send for uid=%{public}d pid=%{public}d", appJankEvent->uid_, appJankEvent->pid_);
    EventPublish::GetInstance().PushEvent(appJankEvent->uid_, UCollectUtil::MAIN_THREAD_JANK,
        HiSysEvent::EventType::FAULT, param);
}

void ReportMainThreadJankForTrace(std::shared_ptr<AppCallerEvent> appJankEvent)
{
    HiSysEventWrite(HiSysEvent::Domain::FRAMEWORK, UCollectUtil::MAIN_THREAD_JANK, HiSysEvent::EventType::FAULT,
        UCollectUtil::SYS_EVENT_PARAM_BUNDLE_NAME, appJankEvent->bundleName_,
        UCollectUtil::SYS_EVENT_PARAM_BUNDLE_VERSION, appJankEvent->bundleVersion_,
        UCollectUtil::SYS_EVENT_PARAM_BEGIN_TIME, appJankEvent->beginTime_,
        UCollectUtil::SYS_EVENT_PARAM_END_TIME, appJankEvent->endTime_,
        UCollectUtil::SYS_EVENT_PARAM_JANK_LEVEL, 1); // 1: over 450ms
}

bool IsRemoteLogOpen()
{
    std::string remoteLogState = Parameter::GetString(HIVIEW_UCOLLECTION_STATE, HIVIEW_UCOLLECTION_STATE_FALSE);
    if (remoteLogState == HIVIEW_UCOLLECTION_STATE_TRUE) {
        return true;
    }
    return false;
}

bool IsDevelopTraceRecorderOpen()
{
    std::string traceRecorderState = Parameter::GetString(DEVELOP_HIVIEW_TRACE_RECORDER, DEVELOP_TRACE_RECORDER_FALSE);
    if (traceRecorderState == DEVELOP_TRACE_RECORDER_TRUE) {
        return true;
    }
    return false;
}

void InitDynamicTrace()
{
    bool s1 = Parameter::IsBetaVersion();
    bool s2 = IsRemoteLogOpen();
    bool s3 = IsDevelopTraceRecorderOpen();
    HIVIEW_LOGI("IsBetaVersion=%{public}d, IsRemoteLogOpen=%{public}d, IsDevelopTraceRecorderOpen=%{public}d",
        s1, s2, s3);
    AppCallerEvent::enableDynamicTrace_ = DYNAMIC_TRACE_FSM[s1][s2][s3];
    HIVIEW_LOGI("dynamic trace open:%{public}d", AppCallerEvent::enableDynamicTrace_);
}

void OnHiViewTraceRecorderChanged(const char* key, const char* value, void* context)
{
    if (key == nullptr || value == nullptr) {
        return;
    }

    if (!(std::string(DEVELOP_HIVIEW_TRACE_RECORDER) == key)) {
        return;
    }

    bool s1 = Parameter::IsBetaVersion();
    bool s2 = IsRemoteLogOpen();
    bool s3;
    if (std::string(DEVELOP_TRACE_RECORDER_TRUE) == value) {
        s3 = true;
    } else {
        s3 = false;
    }
    AppCallerEvent::enableDynamicTrace_ = DYNAMIC_TRACE_FSM[s1][s2][s3];
}

void OnSwitchRecordTraceStateChanged(const char* key, const char* value, void* context)
{
    OnHiViewTraceRecorderChanged(key, value, context);
    HIVIEW_LOGI("record trace state changed, ret: %{public}s", value);
    if (key == nullptr || value == nullptr) {
        HIVIEW_LOGE("record trace switch input ptr null");
        return;
    }
    if (strncmp(key, DEVELOP_HIVIEW_TRACE_RECORDER, strlen(DEVELOP_HIVIEW_TRACE_RECORDER)) != 0) {
        HIVIEW_LOGE("record trace switch param key error");
        return;
    }

    if (UnifiedCollector::IsEnableRecordTrace() == false && strncmp(value, "true", strlen("true")) == 0) {
        UnifiedCollector::SetRecordTraceStatus(true);
        TraceManager traceManager;
        int32_t resultOpenTrace = traceManager.OpenRecordingTrace(DEVELOPER_MODE_TRACE_ARGS);
        if (resultOpenTrace != 0) {
            HIVIEW_LOGE("failed to start trace service");
        }

        std::shared_ptr<UCollectUtil::TraceCollector> traceCollector = UCollectUtil::TraceCollector::Create();
        CollectResult<int32_t> resultTraceOn = traceCollector->TraceOn();
        if (resultTraceOn.retCode != UCollect::UcError::SUCCESS) {
            HIVIEW_LOGE("failed to start collection trace");
        }
    } else if (UnifiedCollector::IsEnableRecordTrace() == true && strncmp(value, "false", strlen("false")) == 0) {
        UnifiedCollector::SetRecordTraceStatus(false);
        std::shared_ptr<UCollectUtil::TraceCollector> traceCollector = UCollectUtil::TraceCollector::Create();
        CollectResult<std::vector<std::string>> resultTraceOff = traceCollector->TraceOff();
        if (resultTraceOff.retCode != UCollect::UcError::SUCCESS) {
            HIVIEW_LOGE("failed to stop collection trace");
        }
        TraceManager traceManager;
        int32_t resultCloseTrace = traceManager.CloseTrace();
        if (resultCloseTrace != 0) {
            HIVIEW_LOGE("failed to stop trace service");
        }
    }
}
}

bool UnifiedCollector::isEnableRecordTrace_ = false;

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

bool UnifiedCollector::OnStartCaptureTrace(std::shared_ptr<AppCallerEvent> appJankEvent)
{
    HIVIEW_LOGI("start trace to capture serval seconds for uid=%{public}d pid=%{public}d",
        appJankEvent->uid_, appJankEvent->pid_);

    std::shared_ptr<TraceFlowController> traceController = std::make_shared<TraceFlowController>();

    // app only has one trace file each day
    if (traceController->HasCallOnceToday(appJankEvent->uid_, appJankEvent->happenTime_)) {
        HIVIEW_LOGI("already capture trace uid=%{public}d pid=%{public}d",
            appJankEvent->uid_, appJankEvent->pid_);
        appJankEvent->resultCode_ = UCollect::UcError(UCollect::HAS_CAPTURE_TRACE);
        return false;
    }

    if (!StartCatpureAppTrace(appJankEvent)) {
        return false;
    }

    appJankEvent->eventName_ = UCollectUtil::STOP_CAPTURE_TRACE;
    DelayProcessEvent(appJankEvent, 3); // 3: delay 3 second to stop trace
    HIVIEW_LOGI("trace is on, wait for uid=%{public}d pid=%{public}d", appJankEvent->uid_, appJankEvent->pid_);
    return true;
}

bool UnifiedCollector::OnStopCaptureTrace(std::shared_ptr<AppCallerEvent> appJankEvent)
{
    HIVIEW_LOGI("stop trace for uid=%{public}d pid=%{public}d", appJankEvent->uid_, appJankEvent->pid_);

    StopRecordAppTrace(appJankEvent);

    // hicollie capture stack in application process, only need to share app event to application by hiview
    ShareAppEvent(appJankEvent);

    std::shared_ptr<TraceFlowController> traceController = std::make_shared<TraceFlowController>();
    traceController->AddNewFinishTask(appJankEvent);
    traceController->CleanAppTrace();

    ReportMainThreadJankForTrace(appJankEvent);
    HIVIEW_LOGI("has stop trace for uid=%{public}d pid=%{public}d", appJankEvent->uid_, appJankEvent->pid_);
    AppCallerEvent::isDynamicTraceOpen_ = false;
    return true;
}

bool UnifiedCollector::OnEvent(std::shared_ptr<Event>& event)
{
    if (event == nullptr) {
        return true;
    }
    HIVIEW_LOGI("Receive Event %{public}s", event->GetEventName().c_str());
    if (event->messageType_ == Event::MessageType::PLUGIN_MAINTENANCE) {
        if (event->eventName_ == UCollectUtil::START_CAPTURE_TRACE) {
            std::shared_ptr<AppCallerEvent> appCallerEvent = Event::DownCastTo<AppCallerEvent>(event);
            return OnStartCaptureTrace(appCallerEvent);
        }

        if (event->eventName_ == UCollectUtil::STOP_CAPTURE_TRACE) {
            std::shared_ptr<AppCallerEvent> appCallerEvent = Event::DownCastTo<AppCallerEvent>(event);
            return OnStopCaptureTrace(appCallerEvent);
        }
    }

    return true;
}

void UnifiedCollector::OnMainThreadJank(SysEvent& sysEvent)
{
    if (sysEvent.GetEventIntValue(UCollectUtil::SYS_EVENT_PARAM_JANK_LEVEL) <
        UCollectUtil::SYS_EVENT_JANK_LEVEL_VALUE_TRACE) {
        // hicollie capture stack in application process, only need to share app event to application by hiview
        Json::Value eventJson;
        eventJson[UCollectUtil::APP_EVENT_PARAM_UID] = sysEvent.GetUid();
        eventJson[UCollectUtil::APP_EVENT_PARAM_PID] = sysEvent.GetPid();
        eventJson[UCollectUtil::APP_EVENT_PARAM_TIME] = sysEvent.happenTime_;
        eventJson[UCollectUtil::APP_EVENT_PARAM_BUNDLE_NAME] = sysEvent.GetEventValue(
            UCollectUtil::SYS_EVENT_PARAM_BUNDLE_NAME);
        eventJson[UCollectUtil::APP_EVENT_PARAM_BUNDLE_VERSION] = sysEvent.GetEventValue(
            UCollectUtil::SYS_EVENT_PARAM_BUNDLE_VERSION);
        eventJson[UCollectUtil::APP_EVENT_PARAM_BEGIN_TIME] = sysEvent.GetEventIntValue(
            UCollectUtil::SYS_EVENT_PARAM_BEGIN_TIME);
        eventJson[UCollectUtil::APP_EVENT_PARAM_END_TIME] = sysEvent.GetEventIntValue(
            UCollectUtil::SYS_EVENT_PARAM_END_TIME);
        eventJson[UCollectUtil::APP_EVENT_PARAM_EXTERNAL_LOG] = sysEvent.GetEventValue(
            UCollectUtil::SYS_EVENT_PARAM_EXTERNAL_LOG);
        std::string param = Json::FastWriter().write(eventJson);

        HIVIEW_LOGI("send as stack trigger for uid=%{public}d pid=%{public}d", sysEvent.GetUid(), sysEvent.GetPid());
        EventPublish::GetInstance().PushEvent(sysEvent.GetUid(), UCollectUtil::MAIN_THREAD_JANK,
            HiSysEvent::EventType::FAULT, param);
        return;
    }
}

void UnifiedCollector::OnEventListeningCallback(const Event& event)
{
    SysEvent& sysEvent = static_cast<SysEvent&>(const_cast<Event&>(event));
    HIVIEW_LOGI("sysevent %{public}s", sysEvent.eventName_.c_str());

    if (sysEvent.eventName_ == UCollectUtil::MAIN_THREAD_JANK) {
        OnMainThreadJank(sysEvent);
        return;
    }

#if PC_APP_STATE_COLLECT_ENABLE

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
#endif // PC_APP_STATE_COLLECT_ENABLE
}

void UnifiedCollector::Dump(int fd, const std::vector<std::string>& cmds)
{
    dprintf(fd, "device beta state is %s.\n", Parameter::IsBetaVersion() ? "beta" : "is not beta");

    std::string remoteLogState = Parameter::GetString(HIVIEW_UCOLLECTION_STATE, HIVIEW_UCOLLECTION_STATE_FALSE);
    dprintf(fd, "remote log state is %s.\n", remoteLogState.c_str());

    std::string traceRecorderState = Parameter::GetString(DEVELOP_HIVIEW_TRACE_RECORDER, DEVELOP_TRACE_RECORDER_FALSE);
    dprintf(fd, "trace recorder state is %s.\n", traceRecorderState.c_str());

    dprintf(fd, "dynamic trace state is %s.\n", AppCallerEvent::enableDynamicTrace_ ? "true" : "false");
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

    InitDynamicTrace();

    observerMgr_ = std::make_shared<UcObserverManager>();

    if (Parameter::IsDeveloperMode()) {
        RunRecordTraceTask();
    }
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

    bool s2;
    if (HIVIEW_UCOLLECTION_STATE_TRUE == value) {
        s2 = true;
        unifiedCollectorPtr->RunCpuCollectionTask();
        unifiedCollectorPtr->RunIoCollectionTask();
        unifiedCollectorPtr->RunUCollectionStatTask();
        unifiedCollectorPtr->LoadHitraceService();
    } else {
        s2 = false;
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

    bool s3 = IsDevelopTraceRecorderOpen();
    AppCallerEvent::enableDynamicTrace_ = DYNAMIC_TRACE_FSM[COML_STATE][s2][s3];
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

void UnifiedCollector::RunRecordTraceTask()
{
    if (IsEnableRecordTrace() == false && Parameter::IsTraceCollectionSwitchOn()) {
        SetRecordTraceStatus(true);
        TraceManager traceManager;
        int32_t resultOpenTrace = traceManager.OpenRecordingTrace(DEVELOPER_MODE_TRACE_ARGS);
        if (resultOpenTrace != 0) {
            HIVIEW_LOGE("failed to start trace service");
        }

        std::shared_ptr<UCollectUtil::TraceCollector> traceCollector = UCollectUtil::TraceCollector::Create();
        CollectResult<int32_t> resultTraceOn = traceCollector->TraceOn();
        if (resultTraceOn.retCode != UCollect::UcError::SUCCESS) {
            HIVIEW_LOGE("failed to start collection trace");
        }
    }
    int ret = Parameter::WatchParamChange(DEVELOP_HIVIEW_TRACE_RECORDER, OnSwitchRecordTraceStateChanged, this);
    HIVIEW_LOGI("add ucollection trace switch param watcher ret: %{public}d", ret);
}
} // namespace HiviewDFX
} // namespace OHOS
