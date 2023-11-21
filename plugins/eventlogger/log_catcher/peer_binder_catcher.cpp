/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "peer_binder_catcher.h"

#include <ctime>
#include <cstdio>
#include <cstdlib>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <securec.h>

#include "common_utils.h"
#include "file_util.h"
#include "log_catcher_utils.h"
#include "logger.h"
#include "parameter_ex.h"
#include "string_util.h"

#include "open_stacktrace_catcher.h"
#include "perf_collector.h"
namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_LABEL(0xD002D01, "EventLogger-PeerBinderCatcher");
#ifdef HAS_HIPERF
using namespace OHOS::HiviewDFX::UCollectUtil;
constexpr char EVENT_LOG_PATH[] = "/data/log/eventlog";
#endif
PeerBinderCatcher::PeerBinderCatcher() : EventLogCatcher()
{
    name_ = "PeerBinderCatcher";
}

bool PeerBinderCatcher::Initialize(const std::string& perfCmd, int layer, int pid)
{
    pid_ = pid;
    layer_ = layer;
    perfCmd_ = perfCmd;
    char buf[BUF_SIZE_512] = {0};
    int ret = snprintf_s(buf, BUF_SIZE_512, BUF_SIZE_512 - 1,
        "PeerBinderCatcher -- pid==%d layer_ == %d\n", pid_, layer_);
    if (ret > 0) {
        description_ = buf;
    }
    return true;
}

void PeerBinderCatcher::Init(std::shared_ptr<SysEvent> event, const std::string& filePath)
{
    event_ = event;
    if ((filePath != "") && FileUtil::FileExists(filePath)) {
        binderPath_ = filePath;
    }
}

int PeerBinderCatcher::Catch(int fd)
{
    if (pid_ <= 0) {
        return -1;
    }

    auto originSize = GetFdSize(fd);

    if (!FileUtil::FileExists(binderPath_)) {
        std::string content = binderPath_ + " : file isn't exists\r\n";
        FileUtil::SaveStringToFd(fd, content);
        return -1;
    }

    std::set<int> pids = GetBinderPeerPids(fd);
    if (pids.empty()) {
        std::string content = "PeerBinder pids is empty\r\n";
        FileUtil::SaveStringToFd(fd, content);
    }
#ifdef HAS_HIPERF
    ForkToDumpHiperf(pids);
#endif
    std::string pidStr = "";
    for (auto pidTemp : pids) {
        if (pidTemp != pid_) {
            CatcherStacktrace(fd, pidTemp);
            pidStr += "," + std::to_string(pidTemp);
        }
    }

    if (event_ != nullptr) {
        event_->SetValue(LOGGER_EVENT_PEERBINDER, StringUtil::TrimStr(pidStr, ','));
    }

    logSize_ = GetFdSize(fd) - originSize;
    return logSize_;
}

std::map<int, std::list<PeerBinderCatcher::BinderInfo>> PeerBinderCatcher::BinderInfoParser(
    std::ifstream& fin, int fd) const
{
    std::map<int, std::list<BinderInfo>> manager;
    const int DECIMAL = 10;
    std::string line;
    bool findBinderHeader = false;
    FileUtil::SaveStringToFd(fd, "\nBinderCatcher --\n\n");
    while (getline(fin, line)) {
        FileUtil::SaveStringToFd(fd, line + "\n");
        if (findBinderHeader) {
            continue;
        }

        if (line.find("async") != std::string::npos) {
            continue;
        }

        std::istringstream lineStream(line);
        std::vector<std::string> strList;
        std::string tmpstr;
        while (lineStream >> tmpstr) {
            strList.push_back(tmpstr);
        }

        auto stringSplit = [](const std::string& str, uint16_t index) -> std::string {
            std::vector<std::string> strings;
            StringUtil::SplitStr(str, ":", strings);
            if (index < strings.size()) {
                return strings[index];
            }
            return "";
        };

        if (strList.size() == 7) { // 7: valid array size
            BinderInfo info = {0};
            // 2: binder peer id,
            std::string server = stringSplit(strList[2], 0);
            // 0: binder local id,
            std::string client = stringSplit(strList[0], 0);
            // 5: binder wait time, s
            std::string wait = stringSplit(strList[5], 1);
            if (server == "" || client == "" || wait == "") {
                continue;
            }
            info.server = std::strtol(server.c_str(), nullptr, DECIMAL);
            info.client = std::strtol(client.c_str(), nullptr, DECIMAL);
            info.wait = std::strtol(wait.c_str(), nullptr, DECIMAL);
            HIVIEW_LOGI("server:%{public}d, client:%{public}d, wait:%{public}d", info.server, info.client, info.wait);
            manager[info.client].push_back(info);
        }
        if (line.find("context") != line.npos) {
            findBinderHeader = true;
        }
    }
    FileUtil::SaveStringToFd(fd, "\n\nPeerBinder Stacktrace --\n\n");
    HIVIEW_LOGI("manager size: %{public}zu", manager.size());
    return manager;
}

std::set<int> PeerBinderCatcher::GetBinderPeerPids(int fd) const
{
    std::set<int> pids;
    std::ifstream fin;
    std::string path = binderPath_;
    fin.open(path.c_str());
    if (!fin.is_open()) {
        HIVIEW_LOGE("open binder file failed, %{public}s.", path.c_str());
        std::string content = "open binder file failed :" + path + "\r\n";
        FileUtil::SaveStringToFd(fd, content);
        return pids;
    }

    std::map<int, std::list<PeerBinderCatcher::BinderInfo>> manager = BinderInfoParser(fin, fd);
    fin.close();

    if (manager.size() == 0 || manager.find(pid_) == manager.end()) {
        return pids;
    }

    if (layer_ == LOGGER_BINDER_STACK_ONE) {
        for (auto each : manager[pid_]) {
            pids.insert(each.server);
        }
    } else if (layer_ == LOGGER_BINDER_STACK_ALL) {
        ParseBinderCallChain(manager, pids, pid_);
    }
    return pids;
}

void PeerBinderCatcher::ParseBinderCallChain(std::map<int, std::list<PeerBinderCatcher::BinderInfo>>& manager,
    std::set<int>& pids, int pid) const
{
    for (auto& each : manager[pid]) {
        if (pids.find(each.server) != pids.end()) {
            continue;
        }
        pids.insert(each.server);
        ParseBinderCallChain(manager, pids, each.server);
    }
}

void PeerBinderCatcher::CatcherStacktrace(int fd, int pid) const
{
    std::string content =  "PeerBinderCatcher start catcher stacktrace for pid : " + std::to_string(pid) + "\r\n";
    FileUtil::SaveStringToFd(fd, content);

    LogCatcherUtils::DumpStacktrace(fd, pid);
}

#ifdef HAS_HIPERF
void PeerBinderCatcher::DoExecHiperf(const std::string& fileName, const std::set<int>& pids)
{
    std::shared_ptr<PerfCollector> perfCollector = PerfCollector::Create();
    perfCollector->SetOutputFilename(fileName);
    constexpr int collectTime = 1;
    perfCollector->SetTimeStopSec(collectTime);
    if (perfCmd_.find("a") == std::string::npos) {
        std::vector<pid_t> selectPids;
        selectPids.push_back(pid_);
        for (const auto& pid : pids) {
            if (pid > 0) {
                selectPids.push_back(pid);
            }
        }
        perfCollector->SetSelectPids(selectPids);
    } else {
        perfCollector->SetTargetSystemWide(true);
    }
    perfCollector->SetFrequency(1000); // 1000 : 1kHz
    CollectResult<bool> ret = perfCollector->StartPerf(EVENT_LOG_PATH);
    if (ret.retCode == UCollect::UcError::SUCCESS) {
        HIVIEW_LOGI("Success to call perf result : %{public}d", ret.data);
    } else {
        HIVIEW_LOGI("Failed to  call perf result : %{public}d", ret.data);
    }
}

void PeerBinderCatcher::ForkToDumpHiperf(const std::set<int>& pids)
{
#if defined(__aarch64__)
    if (perfCmd_.empty()) {
        HIVIEW_LOGI("BinderPeer perf is not configured.");
        return;
    }

    if (!Parameter::IsBetaVersion()) {
        HIVIEW_LOGI("BinderPeer perf is only enabled in beta version.");
        return;
    }

    static std::mutex lock;
    std::unique_lock<std::mutex> mlock(lock);
    std::string fileName = "hiperf-" + std::to_string(pid_) + ".data";
    std::string fullPath = std::string(EVENT_LOG_PATH) + "/" + fileName;
    if (access(fullPath.c_str(), F_OK) == 0) {
        struct stat statBuf;
        auto now = time(nullptr);
        if (stat(fullPath.c_str(), &statBuf) == -1) {
            HIVIEW_LOGI("Failed to stat file, error:%{public}d.", errno);
            FileUtil::RemoveFile(fullPath);
        } else if (now - statBuf.st_mtime < PERF_LOG_EXPIRE_TIME) {
            HIVIEW_LOGI("Target log has exist, reuse it.");
            return;
        } else {
            FileUtil::RemoveFile(fullPath);
        }
    }

    pid_t child = fork();
    if (child < 0) {
        // failed to fork child
        return;
    } else if (child == 0) {
        pid_t grandChild = fork();
        if (grandChild == 0) {
            DoExecHiperf(fileName, pids);
        }
        _exit(0);
    } else {
        // do not left a zombie
        waitpid(child, nullptr, 0);
    }
#endif
}
#endif
} // namespace HiviewDFX
} // namespace OHOS