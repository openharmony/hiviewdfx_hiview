/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "native_leak_state.h"

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <string>

#include <faultlogger.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "app_event_handler.h"
#include "app_event_publisher_factory.h"
#include "event_publish.h"
#include "fault_common_base.h"
#include "fault_detector_base.h"
#include "fault_info_base.h"
#include "fault_state_base.h"
#include "ffrt.h"
#include "hisysevent.h"
#include "kits/cpp/mutex.h"
#include "kits/cpp/sleep.h"
#include "logger.h"
#include "mem_profiler_collector.h"
#include "native_leak_info.h"
#include "native_leak_util.h"
#include "plugin_factory.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("NativeLeakState");

using namespace std;
using std::chrono::microseconds;

namespace {
constexpr uint32_t SHORT_WEIGHT = 8; // the ostream placeholder for a word when write memory info to file
constexpr uint32_t LOG_WEIGHT = 16;
constexpr size_t BASE_MEMBER_CNT = 4;
constexpr mode_t DEFAULT_LOG_FILE_MODE = 0644;
constexpr mode_t HIPROFILER_LOG_FILE_MODE = 0664;
constexpr uint32_t ABANDON_PROPORTION = 5;
constexpr uint32_t MAX_RECORD_NUM = 30;
constexpr uint32_t JUDGE_RECORD_NUM = 8;
constexpr uint32_t US_PER_SECOND = 1000 * 1000;
constexpr uint32_t WAIT_TRACKER_SO_LOAD_TIME_OUT = 3;
constexpr uint32_t WAIT_NMD_INFO_DUMP = 5;
constexpr uint32_t PROFILER_DEBUG_TIME_OUT = 900;
constexpr uint32_t WAIT_TO_DUMP_PROFILER_MEM = PROFILER_DEBUG_TIME_OUT + 5;
constexpr uint32_t PROFILER_SAMPLE_INTERVAL = 256;
constexpr uint32_t KILLER_MEM_LIMIT = 8 * 1024 * 1024; // 8GB

constexpr uint32_t HARD_THRESHOLD = 4 * 1024 * 1024; // 4GB

enum {
    SMAPS_INFO = 1,
};

map<uint32_t, string> extraInfo = {
    { SMAPS_INFO, "SMAPS_INFO"},
};

enum SmapsRollupRecordItem {
    PSS = 1,
    PSS_SIZE,
};

struct TrackCmd {
    uint32_t magic;
    uint32_t id;
    uint32_t type;
    uint64_t timestamp;
    enum MemCmd cmd;
};

struct DetailInfo {
    uint32_t magic { 0 };
    uint32_t id { 0 };
    uint32_t type { 0 };
    uint64_t size { 0 };
    uint64_t timestamp;
    char data[0];
};
}

ErrCode NativeLeakSampleState::StateProcess(shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj)
{
    auto userMonitorInfo = static_pointer_cast<NativeLeakInfo>(monitorInfo);
    HIVIEW_LOGI("NativeLeakSampleState::StateProcess pid: %{public}d", userMonitorInfo->GetPid());
    CollectBaseInfo(userMonitorInfo);
    return SUCCESSED;
}
ErrCode NativeLeakSampleState::ChangeNextState(shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj)
{
    return SUCCESSED;
}

void NativeLeakSampleState::CollectBaseInfo(shared_ptr<NativeLeakInfo> &userMonitorInfo) const
{
    CollectUserBaseInfo(userMonitorInfo);
    RemoveData(userMonitorInfo);
}

bool NativeLeakSampleState::CollectUserBaseInfo(shared_ptr<NativeLeakInfo> &userMonitorInfo) const
{
    HIVIEW_LOGI("NativeLeakSampleState::CollectUserBaseInfo pid: %{public}d", userMonitorInfo->GetPid());
    userMonitorInfo->AddCpuTime(FaultDetectorUtil::GetRunningMonotonicTime());
    userMonitorInfo->AddRealTime(FaultDetectorUtil::GetRealTime());
    userMonitorInfo->AddMemory(FaultDetectorUtil::GetProcessRss(userMonitorInfo->GetPid()));
    string smapsRollupPath = "/proc/" + to_string(userMonitorInfo->GetPid()) + "/smaps_rollup";
    ifstream fin;
    fin.open(smapsRollupPath.c_str());
    if (!fin.is_open()) {
        HIVIEW_LOGE("failed to open: %{public}s", smapsRollupPath.c_str());
        return false;
    }

    string line;
    uint64_t pssMemoryKb = 0;
    while (getline(fin, line)) {
        regex recordRegex("^(.+):\\s*(.+) kB$");
        smatch matches;
        if (!regex_match(line, matches, recordRegex)) {
            continue;
        }
        if (matches[PSS].str() == "Pss" || matches[PSS].str() == "SwapPss") {
            pssMemoryKb += stoull(matches[PSS_SIZE]);
        }
    }
    userMonitorInfo->AddCpuTime(FaultDetectorUtil::GetRunningMonotonicTime());
    userMonitorInfo->AddRealTime(FaultDetectorUtil::GetRealTime());
    userMonitorInfo->AddMemory(pssMemoryKb);
    HIVIEW_LOGI("process: %{public}s, pid: %{public}d, pss:%{public}u KB",
        userMonitorInfo->GetProcessName().c_str(), userMonitorInfo->GetPid(), static_cast<uint32_t>(pssMemoryKb));
    if (pssMemoryKb > (userMonitorInfo->GetTopMemory())) {
        userMonitorInfo->SetTopMemory(pssMemoryKb);
    }
    return true;
}

void NativeLeakSampleState::RemoveData(shared_ptr<NativeLeakInfo> &userMonitorInfo) const
{
    size_t cpuSize = userMonitorInfo->GetCpuTime().size();
    if (cpuSize < MAX_RECORD_NUM) {
        return;
    }
    HIVIEW_LOGI("NativeLeakSampleState::RemoveData pid: %{public}d", userMonitorInfo->GetPid());
    userMonitorInfo->RemoveMemory(ABANDON_PROPORTION);
    userMonitorInfo->RemoveTime(ABANDON_PROPORTION);
}

ErrCode NativeLeakJudgeState::StateProcess(shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj)
{
    HIVIEW_LOGI("NativeLeakJudgeState::StateProcess pid: %{public}d", monitorInfo->GetPid());
    auto userMonitorInfo = static_pointer_cast<NativeLeakInfo>(monitorInfo);
    if (IsMemoryLeak(userMonitorInfo)) {
        HIVIEW_LOGE("pid: %{public}d does memory leak", userMonitorInfo->GetPid());
        userMonitorInfo->SetLeakGrade(JudgeMemoryLeakGrade(userMonitorInfo));
        return SUCCESSED;
    }
    HIVIEW_LOGI("pid: %{public}d dont memory leak", userMonitorInfo->GetPid());
    return FAILURE;
}

bool NativeLeakJudgeState::IsMemoryLeak(shared_ptr<NativeLeakInfo> &userMonitorInfo)
{
    uint64_t topMemory = userMonitorInfo->GetTopMemory();
    auto pid = userMonitorInfo->GetPid();
    if (topMemory >= HARD_THRESHOLD) {
        return true;
    }
    uint32_t memorySize = userMonitorInfo->GetMemory().size();
    if (memorySize < JUDGE_RECORD_NUM) {
        HIVIEW_LOGE("pid: %{public}d memorySize:%{public}u", pid, memorySize);
        return false;
    }
    uint64_t leakThreshold = userMonitorInfo->GetMemoryLimit();
    for (size_t i = memorySize - JUDGE_RECORD_NUM; i < memorySize; ++i) {
        if (userMonitorInfo->GetMemory().at(i) < leakThreshold) {
            HIVIEW_LOGE("pid: %{public}d some memory record is few, no leak", pid);
            return false;
        }
    }
    HIVIEW_LOGI("pid: %{public}d memory always over limit, seems leak", userMonitorInfo->GetPid());
    return true;
}

string NativeLeakJudgeState::JudgeMemoryLeakGrade(shared_ptr<NativeLeakInfo> &userMonitorInfo)
{
    string leakGrade;
    if (userMonitorInfo->GetMemoryLimit() > MEMORY_RATING_LINE) {
        leakGrade = JudgeMemoryLeakGradeByRatio(userMonitorInfo);
    } else {
        leakGrade = JudgeSmallMemoryLeakGrade(userMonitorInfo);
    }
    HIVIEW_LOGI("pid: %{public}d leakGrade:%{public}s", userMonitorInfo->GetPid(), leakGrade.c_str());
    return leakGrade;
}

string NativeLeakJudgeState::JudgeSmallMemoryLeakGrade(shared_ptr<NativeLeakInfo> &userMonitorInfo)
{
    uint64_t diff = userMonitorInfo->GetTopMemory() - userMonitorInfo->GetMemoryLimit();
    if (diff <= MEMORY_WARNING_GRADE) {
        return MEMORY_LEAK_INFO;
    }
    if (diff <= MEMORY_ERROR_GRADE) {
        return MEMORY_LEAK_WARNING;
    }
    return MEMORY_LEAK_ERROR;
}

string NativeLeakJudgeState::JudgeMemoryLeakGradeByRatio(shared_ptr<NativeLeakInfo> &userMonitorInfo)
{
    uint64_t memoryLimit = userMonitorInfo->GetMemoryLimit();
    uint64_t diff = userMonitorInfo->GetTopMemory() - memoryLimit;
    if (memoryLimit == 0) {
        return MEMORY_LEAK_ERROR;
    }
    double diffRatio = diff * 1.0 / memoryLimit;
    constexpr uint32_t oneHundred = 100;
    diffRatio = floor(diffRatio * oneHundred) / oneHundred;
    if (diffRatio <= MEMORY_WARNING_RATIO && diff < MEMORY_WARNING_RATIO_GRADE) {
        return MEMORY_LEAK_INFO;
    }
    if (diffRatio <= MEMORY_ERROR_RATIO && diff < MEMORY_ERROR_RATIO_GRADE) {
        return MEMORY_LEAK_WARNING;
    }
    return MEMORY_LEAK_ERROR;
}

string NativeLeakJudgeState::JudgeOtherMemoryLeakGrade(shared_ptr<NativeLeakInfo> &userMonitorInfo)
{
    return MEMORY_LEAK_WARNING;
}

ErrCode NativeLeakJudgeState::ChangeNextState(shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj)
{
    return SUCCESSED;
}

NativeLeakDumpState::NativeLeakDumpState()
#ifdef HAS_HIPROFILER
    : memProfilerCollector_(UCollectUtil::MemProfilerCollector::Create())
#endif
{
}

ErrCode NativeLeakDumpState::StateProcess(shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj)
{
    auto userMonitorInfo = static_pointer_cast<NativeLeakInfo>(monitorInfo);
    HIVIEW_LOGI("NativeLeakDumpState::StateProcess pid: %{public}d", userMonitorInfo->GetPid());
    GetMemoryLeakLog(userMonitorInfo, GENERAL_STATISTICS);
    DumpExtraInfo(userMonitorInfo, SMAPS_INFO);
    LaunchMemoryDebug(userMonitorInfo);
    GetProfiler(userMonitorInfo);
    if (ChangeNextState(monitorInfo, detectorObj)) {
        HIVIEW_LOGE("ChangeNextState failed, pid: %{public}d", userMonitorInfo->GetPid());
        return FAILURE;
    }
    HIVIEW_LOGI("ChangeNextState success, pid: %{public}d", userMonitorInfo->GetPid());
    return SUCCESSED;
}

void NativeLeakDumpState::GetMemoryLeakLog(shared_ptr<NativeLeakInfo> &userMonitorInfo, uint32_t flag)
{
    string filePath = userMonitorInfo->GetSmapsPath();
    HIVIEW_LOGE("filepath:%{public}s", filePath.c_str());
    ofstream fout;
    if (!userMonitorInfo->GetIsAppendSmapsFile()) {
        fout.open(filePath, ios::out);
    } else {
        fout.open(filePath, ios::out | ios::app);
    }
    if (!fout.is_open()) {
        HIVIEW_LOGE("FAILED TO OPEN FILE:%{public}s", filePath.c_str());
        return;
    }
    if (flag & GENERAL_STATISTICS) {
        DumpGeneralInfo(fout, userMonitorInfo);
        userMonitorInfo->SetIsAppendSmapsFile(true);
    }
    if (flag & ACCURATE_STATISTICS) {
        DumpDetailInfo(fout, userMonitorInfo);
    }
    if (flag & STACK_STATISTICS) {
        DumpStackInfo(userMonitorInfo);
    }
    DumpUserMemInfo(userMonitorInfo);
}

void NativeLeakDumpState::DumpGeneralInfo(ofstream &fout, shared_ptr<NativeLeakInfo> &userMonitorInfo) const
{
    if (userMonitorInfo == nullptr) {
        HIVIEW_LOGE("WRONG POINTER");
        return;
    }
    fout << endl;
    fout << "Generated by HivewDFX @OpenHarmony" << endl;
    fout << "LOGGER_MEMCHECK_GERNAL_INFO" << endl;
    fout << "\tpidNumber: " << userMonitorInfo->GetPid() << endl;
    fout << "\tprocessName: " << userMonitorInfo->GetProcessName() << endl;
    fout << "\tPidStartTime: " << userMonitorInfo->GetPidStartTime() << endl;
    fout << "\tTopPssMemory: " << userMonitorInfo->GetTopMemory() << endl;
    fout << endl;
    string meminfoPath = FaultDetectorUtil::GetMemInfoPath();
    ifstream fin(meminfoPath);
    if (!fin.is_open()) {
        HIVIEW_LOGE("failed to open file: %{public}s", meminfoPath.c_str());
        return;
    }
    fout << "******************************" << endl;
    fout << "LOGGER_MEMCHECK_MEMINFO" << endl;
    fout << fin.rdbuf();
    fout << endl;
    HIVIEW_LOGI("WRITE GENERAL STATIS ITICS FINISHED");
}

void NativeLeakDumpState::DumpDetailInfo(ofstream &fout, shared_ptr<NativeLeakInfo> &userMonitorInfo)
{
    dumpStateMtx_.lock();
    fout << endl;
    fout << "******************************" << endl;
    fout << "LOGGER_MEMCHECK_DETAIL_INFO" << endl;
    auto fd = open(BBOX_PATH.c_str(), O_RDONLY);
    if (fd < 0) {
        dumpStateMtx_.unlock();
        HIVIEW_LOGE("failed to open %{public}s, err: %{public}d", BBOX_PATH.c_str(), errno);
        return;
    }
    uint64_t detailSize = sizeof(DetailInfo) + MEMCHECK_DETAILINFO_MAXSIZE;
    auto detailInfo = static_cast<DetailInfo *>(calloc(1, detailSize));
    if (detailInfo == nullptr) {
        dumpStateMtx_.unlock();
        HIVIEW_LOGE("FAILED TO ALLOC MEMORY!");
        close(fd);
    }
    detailInfo->magic = MEMCHECK_MAGIC;
    detailInfo->id = userMonitorInfo->GetPid();
    detailInfo->size = MEMCHECK_DETAILINFO_MAXSIZE;
    detailInfo->type = MTYPE_USER_PSS;
    detailInfo->timestamp = userMonitorInfo->GetPidStartTime();
    int32_t ret = ioctl(fd, LOGGER_MEMCHECK_DETAIL_READ, detailInfo);
    if (ret != 0) {
        HIVIEW_LOGE("IOCTL READ DETAIL STATISITICS FAILED, ret=%{public}d", ret);
    }
    fout << detailInfo->data << endl;
    free(detailInfo);
    HIVIEW_LOGI("process %{public}s WRITE DETAIL STATISITICS FINISHED", userMonitorInfo->GetProcessName().c_str());
    close(fd);
    dumpStateMtx_.unlock();
}

void NativeLeakDumpState::DumpStackInfo(shared_ptr<NativeLeakInfo> &userMonitorInfo)
{
#ifdef HAS_HIPROFILER
    HIVIEW_LOGW("HAS_HIPROFILER defined, is going to dump stack info.");
    string logFilePath = userMonitorInfo->GetLogFilePath();
    auto fd = open(logFilePath.c_str(), O_CREAT | O_RDWR, HIPROFILER_LOG_FILE_MODE);
    if (fd < 0) {
        HIVIEW_LOGE("failed to open %{public}s, err: %{public}d", logFilePath.c_str(), errno);
        return;
    }
    int ret = memProfilerCollector_->Start(fd, UCollectUtil::ProfilerType::MEM_PROFILER_CALL_STACK,
        userMonitorInfo->GetPid(), PROFILER_DEBUG_TIME_OUT, PROFILER_SAMPLE_INTERVAL);
    if (ret < 0) {
        close(fd);
        HIVIEW_LOGE("dump process %{public}s mem profiler failed, ret is: %{public}d",
            userMonitorInfo->GetProcessName().c_str(), ret);
        return;
    }
    ffrt::this_task::sleep_for(microseconds(WAIT_TO_DUMP_PROFILER_MEM * US_PER_SECOND));
    close(fd);
    HIVIEW_LOGI("process %{public}s WRITE STACK STATISITICS FINISHED", userMonitorInfo->GetProcessName().c_str());
#else
    HIVIEW_LOGW("HAS_HIPROFILER not define, will not dump stack info.");
#endif
}

void NativeLeakDumpState::DumpUserMemInfo(shared_ptr<NativeLeakInfo> &userMonitorInfo)
{
    dumpStateMtx_.lock();
    string filePath = userMonitorInfo->GetSampleFilePath();
    ofstream fout(filePath);
    if (!fout.is_open()) {
        HIVIEW_LOGE("failed to open file: %{public}s", filePath.c_str());
        dumpStateMtx_.unlock();
        return;
    }
    fout << "pid:\t" << userMonitorInfo->GetPid() << "\t" << endl;
    fout << "processName:\t" << userMonitorInfo->GetProcessName() << endl;
    fout << "threshold:\t" << userMonitorInfo->GetMemoryLimit() << "(KB)" << endl;
    fout << "actualRss:\t" << userMonitorInfo->GetActualRssThreshold() << "(KB)" << endl;
    fout << "TopPssMemory:\t" << userMonitorInfo->GetTopMemory() << "(KB)" << endl;
    fout << left;
    fout << endl;
    fout << setw(LOG_WEIGHT) << "time(s)" << setw(LOG_WEIGHT) << "PssMemory(KB)";
    fout << setw(LOG_WEIGHT) << "realtime" << endl;
    size_t cpuSize = userMonitorInfo->GetCpuTime().size();
    for (size_t i = 0; i < cpuSize; ++i) {
        if (cpuSize != userMonitorInfo->GetMemory().size()) {
            HIVIEW_LOGE("CPU SIZE NOT MATCH MEMORY SIZE");
            dumpStateMtx_.unlock();
            return;
        }
        fout << setw(LOG_WEIGHT) << userMonitorInfo->GetCpuTime().at(i);
        fout << setw(LOG_WEIGHT) << userMonitorInfo->GetMemory().at(i);
        fout << setw(LOG_WEIGHT) << userMonitorInfo->GetRealTime().at(i) << endl;
    }
    dumpStateMtx_.unlock();
}

void NativeLeakDumpState::DumpExtraInfo(shared_ptr<NativeLeakInfo> &userMonitorInfo, uint32_t type) const
{
    if (type & SMAPS_INFO) {
        ForkProcessToDumpExtraInfo(userMonitorInfo->GetSmapsPath(), userMonitorInfo, SMAPS_INFO);
    }
}

bool NativeLeakDumpState::ForkProcessToDumpExtraInfo(
    const string &path, shared_ptr<NativeLeakInfo> &userMonitorInfo, uint32_t type) const
{
    int writeFd = 0;
    if (type & SMAPS_INFO) {
        writeFd = open(path.c_str(), O_CREAT | O_RDWR | O_APPEND, DEFAULT_LOG_FILE_MODE);
    } else {
        writeFd = open(path.c_str(), O_CREAT | O_RDWR, DEFAULT_LOG_FILE_MODE);
    }
    if (writeFd < 0) {
        HIVIEW_LOGE("failed to open %{public}s, errno is: %{public}d", path.c_str(), errno);
        return false;
    }
    int ret = dup2(writeFd, STDIN_FILENO) + dup2(writeFd, STDOUT_FILENO) + dup2(writeFd, STDERR_FILENO);
    if (ret < 0) {
        HIVIEW_LOGE("dup2 writeFd fail, error is %{public}d", errno);
        close(writeFd);
        return false;
    }
    if (!DumpUserMemInfoToSmapsFile(writeFd, userMonitorInfo)) {
        HIVIEW_LOGE("dump user mem info to smaps failed, errno is %{public}d", errno);
    }
    string generalMessage = string("******************************\n")
        + string("LOGGER_MEMCHECK_") + GetExtraInfo(type) + "\n"
        + string("get info realtime:\t") + FaultDetectorUtil::GetRealTime() + "\n";
    size_t generalMessageSize = generalMessage.size();
    ssize_t watchdogWrite = write(writeFd, generalMessage.c_str(), generalMessageSize);
    if (watchdogWrite < 0) {
        HIVIEW_LOGE("write get extra info realtime failed, errno is %{public}d", errno);
    }
    pid_t childPid = fork();
    if (childPid < 0) {
        HIVIEW_LOGE("failed to fork process, errno is: %{public}d\n", errno);
        close(writeFd);
        return false;
    }
    if (childPid == 0) {
        if (type & SMAPS_INFO) {
            ExecuteChildProcessGetSmapsInfo(userMonitorInfo->GetPid());
        }
    } else {
        if (waitpid(childPid, nullptr, 0) != childPid) {
            HIVIEW_LOGE("waitpid fail, pid: %{public}d, error: %{public}d", childPid, errno);
            close(writeFd);
            return false;
        }
        HIVIEW_LOGI("waitpid %{public}d success", childPid);
    }
    close(writeFd);
    return true;
}

bool NativeLeakDumpState::DumpUserMemInfoToSmapsFile(int writeFd, shared_ptr<NativeLeakInfo> &userMonitorInfo) const
{
    string longWeight(LOG_WEIGHT, ' ');
    string userMemMessage = "pid:\t" + to_string(userMonitorInfo->GetPid()) + "\n"
        + "processName:\t" + userMonitorInfo->GetProcessName() + "\n"
        + "threshold:\t" + to_string(userMonitorInfo->GetMemoryLimit()) + "\n"
        + "TopPssMemory:\t" + to_string(userMonitorInfo->GetTopMemory()) + "\n"
        + "\n";
    userMemMessage += "time(s)" + longWeight + "PssMemory(KB)" + longWeight + "realtime\n";
    size_t cpuSize = userMonitorInfo->GetCpuTime().size();
    for (size_t i = 0; i < userMonitorInfo->GetCpuTime().size(); ++i) {
        if (cpuSize != userMonitorInfo->GetMemory().size()) {
            HIVIEW_LOGE("CPU SIZE NOT MATCH MEMORY SIZE");
            return false;
        }
        userMemMessage += to_string(userMonitorInfo->GetCpuTime().at(i)) + longWeight
            + to_string(userMonitorInfo->GetMemory().at(i)) + longWeight
            + userMonitorInfo->GetRealTime().at(i) + "\n";
    }
    size_t userMemMessageSize = userMemMessage.size();
    ssize_t watchdogWrite = write(writeFd, userMemMessage.c_str(), userMemMessageSize);
    if (watchdogWrite < 0) {
        HIVIEW_LOGE("write to smaps file failed, errno is %{public}d", errno);
        return false;
    }
    return true;
}

string NativeLeakDumpState::GetExtraInfo(uint32_t type) const
{
    auto it = find_if(extraInfo.begin(), extraInfo.end(),
                      [&type](const pair<uint32_t, string>& item) {
                          return type & item.first;
                      });
    if (it != extraInfo.end()) {
        return it->second;
    }
    return "";
}

void NativeLeakDumpState::ExecuteChildProcessGetSmapsInfo(int pid) const
{
    int ret = execl("/system/bin/hidumper", "hidumper", "--mem-smaps", to_string(pid).c_str(), nullptr);
    if (ret < 0) {
        HIVIEW_LOGE("execl return %{public}d, type is SMAPS_INFO, errno: %{public}d", ret, errno);
        _exit(-1);
    }
    _exit(0);
}

void NativeLeakDumpState::LaunchMemoryDebug(shared_ptr<NativeLeakInfo> &userMonitorInfo)
{
    if (userMonitorInfo->GetIsProcessDied()) {
        HIVIEW_LOGE("process %{public}s has died, stop get native log.", userMonitorInfo->GetProcessName().c_str());
        return;
    }
    userMonitorInfo->SetDebugStartTime(FaultDetectorUtil::GetRunningMonotonicTime());
    if (!SuccessToSendCmd(userMonitorInfo, MEMCMD_ENABLE)) {
        return;
    }
    // wait memleak_tracker so dlopen
    ffrt::this_task::sleep_for(microseconds(WAIT_TRACKER_SO_LOAD_TIME_OUT * US_PER_SECOND));
    GetMemoryLeakLog(userMonitorInfo, ACCURATE_STATISTICS);
    if (!SuccessToSendCmd(userMonitorInfo, MEMCMD_CLEAR_LOG)) {
        return;
    }
}

void NativeLeakDumpState::GetProfiler(shared_ptr<NativeLeakInfo> &userMonitorInfo)
{
    // musl hooking leak, hiprofiler is not allowed, wait for nmd info dump
    ffrt::this_task::sleep_for(microseconds(WAIT_NMD_INFO_DUMP * US_PER_SECOND));
    GetMemoryLeakLog(userMonitorInfo, STACK_STATISTICS);
}

bool NativeLeakDumpState::SuccessToSendCmd(shared_ptr<NativeLeakInfo> &userMonitorInfo, MemCmd cmdType)
{
    if (SendCmd(userMonitorInfo, cmdType)) {
        return false;
    }
    return true;
}

int32_t NativeLeakDumpState::SendCmd(shared_ptr<NativeLeakInfo> &userMonitorInfo, MemCmd cmdType) const
{
    TrackCmd trackCmd;
    trackCmd.magic = MEMCHECK_MAGIC;
    trackCmd.cmd = cmdType;
    trackCmd.id = userMonitorInfo->GetPid();
    trackCmd.type = userMonitorInfo->GetJavaState() ? MTYPE_USER_PSS_JAVA : MTYPE_USER_PSS_NATIVE;
    trackCmd.timestamp = userMonitorInfo->GetPidStartTime();
    HIVIEW_LOGI("ID:%{public}d, type:%{public}u, cmd:%{public}d", trackCmd.id, trackCmd.type, cmdType);
    int fd = open(BBOX_PATH.c_str(), O_RDONLY);
    if (fd < 0) {
        HIVIEW_LOGE("failed to open %{public}s", BBOX_PATH.c_str());
        return FAILURE;
    }
    int32_t ret = ioctl(fd, LOGGER_MEMCHECK_COMMAND, &trackCmd);
    if (ret != 0) {
        HIVIEW_LOGE("SEND CMD ERROR");
    }
    close(fd);
    return ret;
}

ErrCode NativeLeakDumpState::ChangeNextState(shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj)
{
    FaultStateType nextState = PROC_REPORT_STATE;
    auto userMonitorInfo = static_pointer_cast<NativeLeakInfo>(monitorInfo);

    int ret = detectorObj.ExeNextStateProcess(monitorInfo, nextState);
    if (ret) {
        HIVIEW_LOGE("dump finished, change to %{public}s state process failed, ret is %{public}d\n",
            FaultStateName[monitorInfo->GetState()].c_str(), ret);
        return FAILURE;
    }
    return SUCCESSED;
}

ErrCode NativeLeakReportState::StateProcess(shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj)
{
    auto userMonitorInfo = static_pointer_cast<NativeLeakInfo>(monitorInfo);
    PostEvent(monitorInfo);

    if (ChangeNextState(monitorInfo, detectorObj)) {
        return FAILURE;
    }
    return SUCCESSED;
}

void NativeLeakReportState::setEventHandler(std::shared_ptr<AppEventHandler> handler)
{
    HIVIEW_LOGI("NativeLeakReportState::setEventHandler");
    this->appEventHandler_ = handler;
}

void NativeLeakReportState::PostEvent(shared_ptr<FaultInfoBase> &monitorInfo)
{
    auto userMonitorInfo = static_pointer_cast<NativeLeakInfo>(monitorInfo);

    if (appEventHandler_ == nullptr) {
        HIVIEW_LOGE("appEventHandler_ is null, will not postEvent.");
        return;
    }

    AppEventHandler::ResourceOverLimitInfo info;
    info.pid = userMonitorInfo->GetPid();
    info.uid = FaultDetectorUtil::GetProcessUid(info.pid);
    info.bundleName = userMonitorInfo->GetProcessName();
    info.bundleVersion = userMonitorInfo->GetHapVersion();
    info.resourceType = "pss_memory";
    info.pss = userMonitorInfo->GetTopMemory();
    FaultDetectorUtil::GetStatm(info.pid, info.vss, info.rss);
    FaultDetectorUtil::GetMeminfo(info.avaliableMem, info.freeMem, info.totalMem);

    HIVIEW_LOGD("PostFaultEvent start");
    appEventHandler_->PostEvent(info);
    HIVIEW_LOGD("PostFaultEvent end");
}

ErrCode NativeLeakReportState::ChangeNextState(shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj)
{
    FaultStateType nextState = PROC_FINISHED_STATE;
    auto userMonitorInfo = static_pointer_cast<NativeLeakInfo>(monitorInfo);

    int ret = detectorObj.ExeNextStateProcess(monitorInfo, nextState);
    if (ret) {
        HIVIEW_LOGE("report finished, Exe %{public}s state process failed, ret is %{public}d\n",
            FaultStateName[nextState].c_str(), ret);
        return FAILURE;
    }
    return SUCCESSED;
}

ErrCode NativeLeakRemovalState::StateProcess(shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj)
{
    HIVIEW_LOGI("NativeLeakRemovalState StateProcess not define\n"); // wzy:kiding me? maybe can use interface
    if (ChangeNextState(monitorInfo, detectorObj)) {
        return FAILURE;
    }
    return SUCCESSED;
}

ErrCode NativeLeakRemovalState::ChangeNextState(shared_ptr<FaultInfoBase> &monitorInfo, FaultDetectorBase &detectorObj)
{
    int ret = detectorObj.ExeNextStateProcess(monitorInfo, PROC_FINISHED_STATE);
    if (ret) {
        HIVIEW_LOGE("remove finished, Exe PROC_FINISHED_STATE state process failed, ret is %{public}d\n", ret);
        return FAILURE;
    }
    return SUCCESSED;
}
} // namespace HiviewDFX
} // namespace OHOS
