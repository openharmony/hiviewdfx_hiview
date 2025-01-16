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

#include <ctime>
#include <memory>

#include "app_caller_event.h"
#include "app_trace_context.h"
#include "collect_event.h"
#include "event_publish.h"
#include "ffrt.h"
#include "file_util.h"
#include "hitrace_dump.h"
#include "hiview_logger.h"
#include "io_collector.h"
#include "memory_collector.h"
#include "parameter_ex.h"
#include "plugin_factory.h"
#include "process_status.h"
#include "sys_event.h"
#include "time_util.h"
#include "trace_behavior_controller.h"
#include "trace_flow_controller.h"
#include "trace_manager.h"
#include "uc_observer_mgr.h"
#include "unified_collection_stat.h"

#define LOW_MEM_THRESHOLD 1024000

#if defined(LOW_MEM_THRESHOLD) && (LOW_MEM_THRESHOLD != 0)
constexpr int CACHE_LOW_MEM_THRESHOLD = LOW_MEM_THRESHOLD;
#else
constexpr int CACHE_LOW_MEM_THRESHOLD = 0;
#endif

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
const std::string DEVELOPER_MODE_TRACE_ARGS = "tags:sched, freq, disk, sync, binder, mmc, membus, load, pagecache, \
    workq, ipa, hdf, virse, net, dsched, graphic, multimodalinput, dinput, ark, ace, window, zaudio, daudio, zmedia, \
    dcamera, zcamera, dhfwk, app, gresource, ability, power, samgr, ffrt clockType:boot1 bufferSize:32768 overwrite:0 \
    fileLimit:20 fileSize:102400";
const std::string OTHER = "Other";
const std::string HIVIEW_UCOLLECTION_STATE_TRUE = "true";
const std::string HIVIEW_UCOLLECTION_STATE_FALSE = "false";
const std::string DEVELOP_TRACE_RECORDER_TRUE = "true";
const std::string DEVELOP_TRACE_RECORDER_FALSE = "false";
const std::string HIVIEW_UCOLLECTION_TEST_APP_TRACE_STATE_TRUE = "true";
constexpr char KEY_FREEZE_DETECTOR_STATE[] = "persist.hiview.freeze_detector";
constexpr int HIVEW_PERF_MONITOR_INTERVAL = 5;
constexpr int HITRACE_CACHE_DURATION_LIMIT_DAILY_TOTAL = 10 * 60;
constexpr int HITRACE_CACHE_DURATION_LIMIT_PER_EVENT = 2 * 60;

const int8_t STATE_COUNT = 2;
const int8_t COML_STATE = 0;
// [coml:0][is remote log][is trace recorder]
// [beta:1][is remote log][is trace recorder]
const bool DYNAMIC_TRACE_FSM[STATE_COUNT][STATE_COUNT][STATE_COUNT] = {
    {{true,  false}, {false, false}},
    {{false, false}, {false, false}},
};

// [dev mode:0][test app trace state]
// [dev mode:1][test app trace state]
const bool CHECK_DYNAMIC_TRACE_FSM[STATE_COUNT][STATE_COUNT] = {
    {true, true}, {false, true}
};

void OnTestAppTraceStateChanged(const char* key, const char* value, void* context)
{
    if (key == nullptr || value == nullptr) {
        return;
    }

    if (!(std::string(HIVIEW_UCOLLECTION_TEST_APP_TRACE_STATE) == key)) {
        return;
    }

    bool isBetaVersion = Parameter::IsBetaVersion();
    bool isUCollectionSwitchOn = Parameter::IsUCollectionSwitchOn();
    bool isTraceCollectionSwitchOn = Parameter::IsTraceCollectionSwitchOn();

    bool isDeveloperMode = Parameter::IsDeveloperMode();
    bool isTraceStateActive = std::string(HIVIEW_UCOLLECTION_TEST_APP_TRACE_STATE_TRUE) == value;
    AppCallerEvent::enableDynamicTrace_ = CHECK_DYNAMIC_TRACE_FSM[isDeveloperMode][isTraceStateActive] &&
        DYNAMIC_TRACE_FSM[isBetaVersion][isUCollectionSwitchOn][isTraceCollectionSwitchOn];
    HIVIEW_LOGI("dynamic trace change to:%{public}d as test trace state", AppCallerEvent::enableDynamicTrace_);
}

void InitDynamicTrace()
{
    bool isBetaVersion = Parameter::IsBetaVersion();
    bool isUCollectionSwitchOn = Parameter::IsUCollectionSwitchOn();
    bool isTraceCollectionSwitchOn = Parameter::IsTraceCollectionSwitchOn();
    HIVIEW_LOGI("IsBetaVersion=%{public}d, IsUCollectionSwitchOn=%{public}d, IsTraceCollectionSwitchOn=%{public}d",
        isBetaVersion, isUCollectionSwitchOn, isTraceCollectionSwitchOn);

    bool isDeveloperMode = Parameter::IsDeveloperMode();
    bool isTestAppTraceOn = Parameter::IsTestAppTraceOn();
    HIVIEW_LOGI("IsDeveloperMode=%{public}d, IsTestAppTraceOn=%{public}d", isDeveloperMode, isTestAppTraceOn);
    AppCallerEvent::enableDynamicTrace_ = CHECK_DYNAMIC_TRACE_FSM[isDeveloperMode][isTestAppTraceOn] &&
        DYNAMIC_TRACE_FSM[isBetaVersion][isUCollectionSwitchOn][isTraceCollectionSwitchOn];
    HIVIEW_LOGI("dynamic trace open:%{public}d", AppCallerEvent::enableDynamicTrace_);

    int ret = Parameter::WatchParamChange(HIVIEW_UCOLLECTION_TEST_APP_TRACE_STATE, OnTestAppTraceStateChanged, nullptr);
    HIVIEW_LOGI("add ucollection test app trace param watcher ret: %{public}d", ret);
}

void OnHiViewTraceRecorderChanged(const char* key, const char* value)
{
    bool isBetaVersion = Parameter::IsBetaVersion();
    bool isUCollectionSwitchOn = Parameter::IsUCollectionSwitchOn();
    bool isTraceCollectionSwitchOn = std::string(DEVELOP_TRACE_RECORDER_TRUE) == value;

    bool isDeveloperMode = Parameter::IsDeveloperMode();
    bool isTestAppTraceOn = Parameter::IsTestAppTraceOn();
    AppCallerEvent::enableDynamicTrace_ = CHECK_DYNAMIC_TRACE_FSM[isDeveloperMode][isTestAppTraceOn] &&
        DYNAMIC_TRACE_FSM[isBetaVersion][isUCollectionSwitchOn][isTraceCollectionSwitchOn];
    HIVIEW_LOGI("dynamic trace change to:%{public}d as trace recorder state", AppCallerEvent::enableDynamicTrace_);
}

void LoadHitraceService()
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

void ExitHitraceService()
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

void OnSwitchRecordTraceStateChanged(const char* key, const char* value, void* context)
{
    if (key == nullptr || value == nullptr) {
        HIVIEW_LOGE("record trace switch input ptr null");
        return;
    }
    if (strncmp(key, DEVELOP_HIVIEW_TRACE_RECORDER, strlen(DEVELOP_HIVIEW_TRACE_RECORDER)) != 0) {
        HIVIEW_LOGE("record trace switch param key error");
        return;
    }
    HIVIEW_LOGI("record trace state changed, value: %{public}s", value);
    OnHiViewTraceRecorderChanged(key, value);

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
        if (Parameter::IsBetaVersion() || Parameter::IsUCollectionSwitchOn()
            || Parameter::GetBoolean(KEY_FREEZE_DETECTOR_STATE, false)) {
            LoadHitraceService();
        }
    }
}

void OnFreezeDetectorParamChanged(const char* key, const char* value, void* context)
{
    if (key == nullptr || value == nullptr) {
        HIVIEW_LOGW("key or value is null");
        return;
    }
    if (strncmp(key, KEY_FREEZE_DETECTOR_STATE, strlen(KEY_FREEZE_DETECTOR_STATE)) != 0) {
        HIVIEW_LOGW("key is not wanted, key: %{public}s", key);
        return;
    }
    HIVIEW_LOGI("freeze detector param changed, value: %{public}s", value);
    if (strncmp(value, "true", strlen("true")) == 0) {
        LoadHitraceService();
    } else {
        if (Parameter::IsUCollectionSwitchOn() || Parameter::IsTraceCollectionSwitchOn()) {
            HIVIEW_LOGI("in remote mode or trace recording, no need to close trace");
        } else {
            ExitHitraceService();
        }
    }
}

int GetCurrentDateAsInt() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);

    int year = 1900 + localTime->tm_year;  // tm_year is years since 1900
    int month = 1 + localTime->tm_mon;    // tm_mon is months since January (0-11)
    int day = localTime->tm_mday;

    return year * 10000 + month * 100 + day;
}

std::chrono::system_clock::time_point GetNextDay() {
    // Get the current time
    auto now = std::chrono::system_clock::now();

    // Convert to time_t for easier manipulation
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    // Convert to a tm structure
    std::tm now_tm = *std::localtime(&now_time_t);

    // Adjust to midnight of the next day
    now_tm.tm_sec = 0;
    now_tm.tm_min = 0;
    now_tm.tm_hour = 0;
    now_tm.tm_mday += 1;

    // Convert back to time_t
    std::time_t next_day_time_t = std::mktime(&now_tm);

    // Convert back to time_point
    return std::chrono::system_clock::from_time_t(next_day_time_t);
}

std::string TimePointToString(const std::chrono::system_clock::time_point& timePoint) {
    // Convert time_point to time_t
    std::time_t timeT = std::chrono::system_clock::to_time_t(timePoint);

    // Convert to tm structure for formatting
    std::tm* tmPtr = std::localtime(&timeT);

    // Format as string (e.g., "YYYY-MM-DD HH:MM:SS")
    std::ostringstream oss;
    oss << std::put_time(tmPtr, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// check whether the cache duration exceeds the limit, if so, return false. otherwise, update database for 5 seconds
bool UpdateCacheLimit(std::shared_ptr<TraceBehaviorController> &traceBehaviorController,
    int detectInterval)
{
    BehaviorRecord record;
    record.behaviorId = CACHE_LOW_MEM;
    // store current time in format YYYYMMDD in dateNum
    record.dateNum = GetCurrentDateAsInt();
    // check whether this entry exists in db, if not, insert it
    HIVIEW_LOGE("Get record for date : %{public}d", record.dateNum);
    if (!traceBehaviorController->GetRecord(record) && traceBehaviorController->InsertRecord(record)) {
        return false;
    }
    HIVIEW_LOGE("usedQuota : %{public}d", record.usedQuota);
    if (record.usedQuota >= HITRACE_CACHE_DURATION_LIMIT_DAILY_TOTAL) {
        HIVIEW_LOGE("usedQuota exceeds limit");
        return false;
    }
    record.usedQuota += detectInterval;
    // update db   
    traceBehaviorController->UpdateRecord(record);
    return true;
}
}

bool UnifiedCollector::isEnableRecordTrace_ = false;

void UnifiedCollector::OnLoad()
{
    HIVIEW_LOGI("start to load UnifiedCollector plugin");
    TraceManager::RecoverTmpTrace();
    Init();
}

void UnifiedCollector::OnUnload()
{
    HIVIEW_LOGI("start to unload UnifiedCollector plugin");
    UcObserverManager::GetInstance().UnregisterObservers();
}

bool UnifiedCollector::OnStartAppTrace(std::shared_ptr<AppCallerEvent> appCallerEvent)
{
    auto nextState = std::make_shared<StartTraceState>(appTraceContext_, appCallerEvent, shared_from_this());
    return appTraceContext_->TransferTo(nextState) == 0;
}

bool UnifiedCollector::OnDumpAppTrace(std::shared_ptr<AppCallerEvent> appCallerEvent)
{
    auto nextState = std::make_shared<DumpTraceState>(appTraceContext_, appCallerEvent, shared_from_this());
    return appTraceContext_->TransferTo(nextState) == 0;
}

bool UnifiedCollector::OnStopAppTrace(std::shared_ptr<AppCallerEvent> appCallerEvent)
{
    auto nextState = std::make_shared<StopTraceState>(appTraceContext_, appCallerEvent);
    return appTraceContext_->TransferTo(nextState) == 0;
}

bool UnifiedCollector::OnEvent(std::shared_ptr<Event>& event)
{
    if (event == nullptr) {
        return true;
    }

    HIVIEW_LOGI("Receive Event %{public}s", event->GetEventName().c_str());
    if (event->messageType_ == Event::MessageType::PLUGIN_MAINTENANCE) {
        std::shared_ptr<AppCallerEvent> appCallerEvent = Event::DownCastTo<AppCallerEvent>(event);
        if (event->eventName_ == UCollectUtil::START_APP_TRACE) {
            return OnStartAppTrace(appCallerEvent);
        }

        if (event->eventName_ == UCollectUtil::DUMP_APP_TRACE) {
            return OnDumpAppTrace(appCallerEvent);
        }

        if (event->eventName_ == UCollectUtil::STOP_APP_TRACE) {
            return OnStopAppTrace(appCallerEvent);
        }
    }

    return true;
}

void UnifiedCollector::OnMainThreadJank(SysEvent& sysEvent)
{
    appTraceContext_->PublishStackEvent(sysEvent);
}

void UnifiedCollector::OnEventListeningCallback(const Event& event)
{
    SysEvent& sysEvent = static_cast<SysEvent&>(const_cast<Event&>(event));
    HIVIEW_LOGI("sysevent %{public}s", sysEvent.eventName_.c_str());

    if (sysEvent.eventName_ == UCollectUtil::MAIN_THREAD_JANK) {
        OnMainThreadJank(sysEvent);
        return;
    }
}

void UnifiedCollector::Dump(int fd, const std::vector<std::string>& cmds)
{
    dprintf(fd, "device beta state is %s.\n", Parameter::IsBetaVersion() ? "beta" : "is not beta");

    std::string remoteLogState = Parameter::GetString(HIVIEW_UCOLLECTION_STATE, HIVIEW_UCOLLECTION_STATE_FALSE);
    dprintf(fd, "remote log state is %s.\n", remoteLogState.c_str());

    std::string traceRecorderState = Parameter::GetString(DEVELOP_HIVIEW_TRACE_RECORDER, DEVELOP_TRACE_RECORDER_FALSE);
    dprintf(fd, "trace recorder state is %s.\n", traceRecorderState.c_str());

    dprintf(fd, "develop state is %s.\n", Parameter::IsDeveloperMode() ? "true" : "false");
    dprintf(fd, "test app trace state is %s.\n", Parameter::IsTestAppTraceOn() ? "true" : "false");

    dprintf(fd, "dynamic trace state is %s.\n", AppCallerEvent::enableDynamicTrace_ ? "true" : "false");

    TraceManager traceManager;
    dprintf(fd, "trace mode is %d.\n", traceManager.GetTraceMode());
}

void UnifiedCollector::Init()
{
    HIVIEW_LOGE("UnifiedCollector init");
    if (GetHiviewContext() == nullptr) {
        HIVIEW_LOGE("hiview context is null");
        return;
    }

    appTraceContext_ = std::make_shared<AppTraceContext>(std::make_shared<StopTraceState>(nullptr, nullptr));

    InitWorkLoop();
    InitWorkPath();
    bool isAllowCollect = Parameter::IsBetaVersion() || Parameter::IsUCollectionSwitchOn();
    RunHiviewMonitorTask();
    if (isAllowCollect) {
        RunIoCollectionTask();
        RunUCollectionStatTask();
        LoadHitraceService();
    }
    if (isAllowCollect || Parameter::IsDeveloperMode()) {
        RunCpuCollectionTask();
    }
    if (!Parameter::IsBetaVersion()) {
        int watchUcollectionRet = Parameter::WatchParamChange(HIVIEW_UCOLLECTION_STATE, OnSwitchStateChanged, this);
        int watchFreezeRet =
            Parameter::WatchParamChange(KEY_FREEZE_DETECTOR_STATE, OnFreezeDetectorParamChanged, nullptr);
        HIVIEW_LOGI("watchUcollectionRet:%{public}d, watchFreezeRet:%{public}d", watchUcollectionRet, watchFreezeRet);
    }
    UcObserverManager::GetInstance().RegisterObservers();

    InitDynamicTrace();

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

    bool isUCollectionSwitchOn;
    if (value == HIVIEW_UCOLLECTION_STATE_TRUE) {
        isUCollectionSwitchOn = true;
        unifiedCollectorPtr->RunCpuCollectionTask();
        unifiedCollectorPtr->RunIoCollectionTask();
        unifiedCollectorPtr->RunUCollectionStatTask();
        LoadHitraceService();
    } else {
        isUCollectionSwitchOn = false;
        if (!Parameter::IsDeveloperMode()) {
            unifiedCollectorPtr->isCpuTaskRunning_ = false;
        }
        for (const auto &it : unifiedCollectorPtr->taskList_) {
            unifiedCollectorPtr->workLoop_->RemoveEvent(it);
        }
        unifiedCollectorPtr->taskList_.clear();
        ExitHitraceService();
        unifiedCollectorPtr->CleanDataFiles();
    }

    bool isTraceCollectionSwitchOn = Parameter::IsTraceCollectionSwitchOn();

    bool isDeveloperMode = Parameter::IsDeveloperMode();
    bool isTestAppTraceOn = Parameter::IsTestAppTraceOn();
    AppCallerEvent::enableDynamicTrace_ = CHECK_DYNAMIC_TRACE_FSM[isDeveloperMode][isTestAppTraceOn] &&
        DYNAMIC_TRACE_FSM[COML_STATE][isUCollectionSwitchOn][isTraceCollectionSwitchOn];
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
    auto task = [this] { this->CpuCollectionFfrtTask(); };
    ffrt::submit(task, {}, {}, ffrt::task_attr().name("dft_uc_cpu").qos(ffrt::qos_default));
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
    auto ioCollectionTask = [this] { this->IoCollectionTask(); };
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
    auto statTask = [this] { this->UCollectionStatTask(); };
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

void UnifiedCollector::RunHiviewMonitorTask()
{
    if (workPath_.empty() || isHiviewPerfMonitorRunning_.load() || CACHE_LOW_MEM_THRESHOLD == 0) {
        HIVIEW_LOGE("Hiview Monitor Task prerequisites are not met.");
        return;
    }
    HIVIEW_LOGE("Hiview Monitor running.");
    isHiviewPerfMonitorRunning_.store(true);
    auto task = [this] { this->HiviewPerfMonitorFfrtTask(); };
    ffrt::submit(task, {}, {}, ffrt::task_attr().name("dft_uc_hiviewMonitor").qos(ffrt::qos_default));
}


void UnifiedCollector::HiviewPerfMonitorFfrtTask()
{
    // get db 
    std::shared_ptr<TraceBehaviorController> traceBehaviorController = std::make_shared<TraceBehaviorController>();
    if (traceBehaviorController == nullptr) {
        HIVIEW_LOGE("failed to create trace behavior controller");
        return;
    }
    std::shared_ptr<UCollectUtil::MemoryCollector> collector = UCollectUtil::MemoryCollector::Create();
    int cacheOffCountdown = 2;
    bool isCacheOn = false;
    int cacheDuration = 0;
    while (!UpdateCacheLimit(traceBehaviorController, HIVEW_PERF_MONITOR_INTERVAL)) {
        // clear db for other entries?
        // sleep until the next day
        auto nextDay = GetNextDay();
        HIVIEW_LOGI("sleep until next day, %{public}s", TimePointToString(nextDay).c_str());
        ffrt::this_task::sleep_until(nextDay);
    }
    // check db for availability, quit if overflow
    while (true) {
        HIVIEW_LOGE("Hiview Monitor running.");
        if (!isHiviewPerfMonitorRunning_.load()) {
            HIVIEW_LOGE("exit hiview low availability detection task");
            break;
        }
        // check memory availability
        CollectResult<SysMemory> data = collector->CollectSysMemory();

        // check target state
        // if target state == current state, continue
        // if target state != current state, change state
        bool targetCacheState = data.data.memAvailable < CACHE_LOW_MEM_THRESHOLD;
        HIVIEW_LOGI("target cache state: %{public}d, current cache state: %{public}d", targetCacheState, isCacheOn);
        if (targetCacheState != isCacheOn) {
            if (targetCacheState) {
                // open cache
                isCacheOn = true;
                // cache on
            } else {
                cacheOffCountdown--;
                if (cacheOffCountdown == 0) {
                    isCacheOn = false;
                    // cache off 
                }
            }
        } else {
            cacheOffCountdown = 2;
        }

        ffrt::this_task::sleep_for(5s); // 5s: monitoring interval
        if (isCacheOn && !UpdateCacheLimit(traceBehaviorController, HIVEW_PERF_MONITOR_INTERVAL)) {
            // cache off, wait for next day
            auto nextDay = GetNextDay();
            HIVIEW_LOGI("sleep until next day, %{public}s", TimePointToString(nextDay).c_str());
            ffrt::this_task::sleep_until(nextDay);
        }

        // check db for availability, and update db for interval = 5s
        // if overflow, close loop, wait for next day. sleep until next day

        // check current cache state and single event duration, if overlimit, close cache, wait until return to normal
        HIVIEW_LOGI("UpdateCacheLimit");
        if (!UpdateCacheLimit(traceBehaviorController, HIVEW_PERF_MONITOR_INTERVAL)) {
            isCacheOn = false;
            cacheOffCountdown = 2;
        }
        if (isCacheOn) {
            cacheDuration += HIVEW_PERF_MONITOR_INTERVAL;
            if (cacheDuration >= HITRACE_CACHE_DURATION_LIMIT_PER_EVENT) {
                isCacheOn = false;
            }
        }
        // sleep for 5s

    }
}
} // namespace HiviewDFX
} // namespace OHOS
