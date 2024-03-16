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

#include "test_util.h"

#include <iostream>
#include <fstream>
#include <regex>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>

#include "ffrt.h"
#include "fault_detector_util.h"
#include "logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("TestUtil");
using namespace std;

constexpr int OVERTIME_TO_RESTART_PROCESS = 10; // 10s
constexpr int BUFFER_LENGTH = 128;
constexpr int MS_PER_SECOND = 1000;

pid_t TestUtil::leakProcessPid_ = -1;

bool TestUtil::FileExists(const string &path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

string TestUtil::GetSmapsPath(const string &processName)
{
    HIVIEW_LOGI("GetSmapsPath start");
    string pid = GetPidByProcess(processName);
    if (pid.empty()) {
        HIVIEW_LOGW("GetSmapsPath failed because pid is empty");
        return "";
    }

    string path = MEMORY_LEAK_PATH + "/memleak-native-" + processName + "-" + pid + "-smaps.txt";
    return path;
}

string TestUtil::GetSampleFile(const string &processName)
{
    HIVIEW_LOGI("GetSampleFile start");
    string pid = GetPidByProcess(processName);
    if (pid.empty()) {
        HIVIEW_LOGW("GetSampleFile failed because pid is empty");
        return "";
    }

    string path = MEMORY_LEAK_PATH + "/memleak-native-" + processName + "-" + pid + "-sample.txt";
    HIVIEW_LOGI("GetSampleFile success, path:%{public}s", path.c_str());
    return path;
}

bool TestUtil::RestartProcess(const string &name)
{
    string pidBefore = GetPidByProcess(name);
    if (pidBefore.empty()) {
        return false;
    }
    KillProcess(name);
    size_t waitTime = 0;
    string pidAfter;

    while (waitTime++ <= OVERTIME_TO_RESTART_PROCESS) {
        pidAfter = GetPidByProcess(name);
        if (!pidAfter.empty()) {
            break;
        }
        ffrt::this_task::sleep_for(chrono::milliseconds(MS_PER_SECOND));
    }
    return pidBefore != pidAfter;
}

void TestUtil::KillProcess(const string &name)
{
    ExecCmd(CmdType::KILL_ALL, name);
}

string TestUtil::GetPidByProcess(const string &name)
{
    string pid = ExecCmd(CmdType::PID_OF, name);
    while (pid.length() > 0 && pid[pid.length() - 1] == '\n') {
        pid = pid.substr(0, pid.length() - 1);
    }
    HIVIEW_LOGI("pid: %{public}s", pid.c_str());
    return pid;
}

void TestUtil::ClearDir(string name)
{
    HIVIEW_LOGI("ClearDir:%{public}s", name.c_str());
    if (name.length() <= 0) {
        return;
    }
    while (name.length() > 0 && name[name.length() - 1] == '/') {
        name[name.length() - 1] = '\0';
    }
    ExecCmd(CmdType::CLEAR_DIR, name);
}

bool TestUtil::IsValidTarget(const string& target)
{
    return regex_match(target, regex("^[a-zA-Z0-9_\\/]+$"));
}

string TestUtil::BuildCommand(CmdType type, const string& target)
{
    if (!IsValidTarget(target)) {
        HIVIEW_LOGW("BuildCommand failed, by target invalid");
        return "";
    }

    string cmd;
    switch (type) {
        case CmdType::KILL_ALL:
            cmd = "killall " + target;
            break;
        case CmdType::CLEAR_DIR:
            cmd = "rm " + target + "/*";
            break;
        case CmdType::PID_OF:
            cmd = "pidof " + target;
            break;
        default:
            return "";
    }
    HIVIEW_LOGI("BuildCommand success, cmd:%{public}s", cmd.c_str());
    return cmd;
}

string TestUtil::ExecCmd(CmdType type, const string& target)
{
    string cmd = BuildCommand(type, target);
    if (cmd.empty()) {
        HIVIEW_LOGW("ExecCmd failed, by cmd empty");
        return "";
    }

    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        HIVIEW_LOGW("ExecCmd failed, by pipe popen failed");
        return "";
    }

    array<char, BUFFER_LENGTH> buffer;
    string result;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    HIVIEW_LOGW("ExecCmd success, result:%{public}s", result.c_str());
    return result;
}

bool TestUtil::IsLeakingProcessAlive()
{
    HIVIEW_LOGI("leakProcessPid_:%{public}d", leakProcessPid_);
    return leakProcessPid_ != -1;
}

bool TestUtil::IsSelfOverLimit(int leakTarget)
{
    uint64_t rss;
    if (!GetSelfStatm(rss)) {
        HIVIEW_LOGW("GetSelfStatm failed");
        return false;
    }

    if (rss > leakTarget) {
        HIVIEW_LOGI("has over limit");
        return true;
    }
    HIVIEW_LOGI("has not over limit");
    return false;
}

bool TestUtil::GetStatm(uint64_t &rss)
{
    if (!IsLeakingProcessAlive()) {
        HIVIEW_LOGW("GetStatm failed, leak process died");
        return false;
    }
    ifstream statmStream("/proc/" + to_string(leakProcessPid_) + "/statm");
    if (!statmStream) {
        HIVIEW_LOGW("GetStatm failed, open statm failed");
        return false;
    }
    string statmLine;
    getline(statmStream, statmLine);
    statmStream.close();
    list<string> numStrArr = GetDightStrArr(statmLine);
    auto it = numStrArr.begin();
    unsigned long long multiples = 4;
    it++;
    rss = multiples * stoull(*it);
    HIVIEW_LOGI("rss:%{public}llu", rss);
    return true;
}

bool TestUtil::GetSelfStatm(uint64_t &rss)
{
    ifstream statmStream("/proc/self/statm");
    if (!statmStream) {
        HIVIEW_LOGW("GetStatm failed, open statm failed");
        return false;
    }
    string statmLine;
    getline(statmStream, statmLine);
    statmStream.close();
    list<string> numStrArr = GetDightStrArr(statmLine);
    auto it = numStrArr.begin();
    unsigned long long multiples = 4;
    it++;
    rss = multiples * stoull(*it);
    HIVIEW_LOGI("self rss:%{public}llu", rss);
    return true;
}

list<string> TestUtil::GetDightStrArr(const string &target)
{
    list<string> ret;
    string temp = "";
    for (size_t i = 0, len = target.size(); i < len; i++) {
        if (target[i] >= '0' && target[i] <= '9') {
            temp += target[i];
            continue;
        }
        if (temp.size() != 0) {
            ret.push_back(temp);
            temp = "";
        }
    }
    if (temp.size() != 0) {
        ret.push_back(temp);
    }
    ret.push_back("0");
    return ret;
}

void TestUtil::CopyFile(const string &srcFile, const string &dstFile)
{
    ifstream src(srcFile, std::ios::binary);
    ofstream dst(dstFile, std::ios::binary);
    dst << src.rdbuf();
}

void TestUtil::WriteFile(const string &file, const string &line)
{
    std::ofstream fs(file, std::ios::out);
    if (fs.is_open()) {
        fs << line.c_str() << "\n";
        fs.close();
    }
}

void TestUtil::DeleteFile(const string &file)
{
    if (std::remove(file.c_str()) != 0) {
        HIVIEW_LOGW("delete file failed");
        return;
    }
    HIVIEW_LOGI("delete file successfully");
}

} // namespace HiviewDFX
} // namespace OHOS

