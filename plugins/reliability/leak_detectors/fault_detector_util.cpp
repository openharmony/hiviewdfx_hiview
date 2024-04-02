/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "fault_detector_util.h"

#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <parameters.h>
#include <securec.h>
#include <sys/stat.h>

#include "bundle_mgr_client.h"
#include "dirent.h"
#include "logger.h"


namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("FaultLeakDetectorUtil");

using namespace OHOS::AppExecFwk;
using std::string;
using std::vector;
using std::stoi;
using std::to_string;
using std::ifstream;
using std::istringstream;
using std::stoull;
using std::getline;
using std::list;

namespace {
const string MEMORY_LEAK_ENABLE_PROPERTY = "hiview.memleakcheck";
const string MEMORY_LEAK_TEST_PROPERTY = "hiview.memleak.test";
constexpr uint32_t KBYTE_PER_BYTE = 1024;
constexpr uint32_t PID_KTHREADD = 2;
constexpr int HAS_LEN = 128;
constexpr int MIN_APP_USERID = 10000;

// index to split /proc/pid/stat info begin ")" with " "
enum StatIndex {
    MIN_INDEX = 0,
    PPID_INDEX,
    GROUP_INDEX, // GRUOP_INDEX
    RUN_TIME_INDEX = 11, // the time of process run from start
    START_TIME_INDEX = 19,
    VSS_INDEX = 20,
    RSS_INDEX = 21,
    MAX_INDEX
};
}

bool FaultDetectorUtil::IsMemLeakDisable()
{
    bool disable = false;
    string value = system::GetParameter(MEMORY_LEAK_ENABLE_PROPERTY, "");
    if (value == "disable" || value == "false") {
        HIVIEW_LOGI("disable memory leak function");
        disable = true;
    }
    return disable;
}

bool FaultDetectorUtil::IsMemTestEnable()
{
    bool testEnable = false;
    string value = system::GetParameter(MEMORY_LEAK_TEST_PROPERTY, "");
    if (value == "enable" || value == "true") {
        HIVIEW_LOGI("enable memory leak test");
        testEnable = true;
    }
    return testEnable;
}

bool FaultDetectorUtil::GetIsHmKernel()
{
    static int isHmKernel = -1;
    if (isHmKernel == -1) {
        string meminfoPath = "/proc/memview";
        auto fd = open(meminfoPath.c_str(), O_RDONLY);
        if (fd < 0) {
            isHmKernel = 0;
            HIVIEW_LOGE("open %{public}s failed, may be linux kernel, errno is %{public}d", meminfoPath.c_str(), errno);
            return false;
        }
        close(fd);
        isHmKernel = 1;
    }
    return isHmKernel == 1;
}

string FaultDetectorUtil::GetMemInfoPath()
{
    return GetIsHmKernel() ? "/proc/memview" : "proc/meminfo";
}

vector<string> FaultDetectorUtil::Split(const string &str, char delim)
{
    istringstream ss(str);
    vector<string> elems;
    for (string item; getline(ss, item, delim);) {
        if (item.empty()) {
            continue;
        }
        elems.push_back(item);
    }
    return elems;
}

bool FaultDetectorUtil::IsDirectory(const string &path)
{
    struct stat statBuffer;
    if (stat(path.c_str(), &statBuffer) == 0 && S_ISDIR(statBuffer.st_mode)) {
        return true;
    }
    return false;
}

vector<string> FaultDetectorUtil::GetSubDir(const string &path, bool digit)
{
    vector<string> subDirs;
    auto dir = opendir(path.c_str());
    if (dir == nullptr) {
        HIVIEW_LOGE("failed to open dir : %{public}s, errno: %{public}d", path.c_str(), errno);
        return subDirs;
    }
    for (struct dirent *ent = readdir(dir); ent != nullptr; ent = readdir(dir)) {
        string childNode = ent->d_name;
        if (childNode == "." || childNode == "..") {
            continue;
        }
        if (digit && !isdigit(childNode[0])) {
            continue;
        }
        if (!IsDirectory(path + "/" + childNode)) {
            continue; // skip directory
        }
        subDirs.push_back(childNode);
    }
    closedir(dir);
    return subDirs;
}

vector<string> FaultDetectorUtil::GetSubFile(const string &path, bool digit)
{
    vector<string> subFiles;
    auto dir = opendir(path.c_str());
    if (dir == nullptr) {
        HIVIEW_LOGE("failed to open dir: %{public}s, errno: %{public}d", path.c_str(), errno);
        return subFiles;
    }
    for (struct dirent *ent = readdir(dir); ent != nullptr; ent = readdir(dir)) {
        string childNode = ent->d_name;
        if (childNode == "." || childNode == "..") {
            continue;
        }
        if (digit && !isdigit(childNode[0])) {
            continue;
        }
        if (IsDirectory(path + "/" + childNode)) {
            continue; // skip directory
        }
        subFiles.push_back(childNode);
    }
    closedir(dir);
    return subFiles;
}

vector<int32_t> FaultDetectorUtil::GetAllPids()
{
    string path = "/proc";
    vector<string> allPids = GetSubDir(path, true);
    vector<int32_t> pids;
    for (const auto &pid : allPids) {
        if (!isdigit(pid[0])) {
            continue;
        }
        pids.push_back(stoi(pid));
    }
    return pids;
}

string FaultDetectorUtil::ReadFileByChar(const string &path)
{
    ifstream fin(path);
    if (!fin.is_open()) {
        return UNKNOWN_PROCESS;
    }
    string content;
    char c;
    while (!fin.eof()) {
        fin >> std::noskipws >> c;
        if (c == '\0' || c == '\n') {
            break;
        }
        content += c;
    }
    return content;
}

bool FaultDetectorUtil::RenameFile(const string &path, const string &newPath)
{
    if (rename(path.c_str(), newPath.c_str())) {
        HIVIEW_LOGE("failed to move %{public}s to %{public}s", path.c_str(), newPath.c_str());
        return false;
    }
    return true;
}

string FaultDetectorUtil::GetRealTime()
{
    time_t now = time(nullptr);
    return TimeStampToStr(now);
}

string FaultDetectorUtil::GetRealTimeStampStr()
{
    time_t now = time(nullptr);
    return TimeStampToSimplelyStr(now);
}

string FaultDetectorUtil::TimeStampToStr(const time_t timeStamp)
{
    struct tm tm;
    constexpr int timeLength = 64;
    char stampStr[timeLength] = { 0 };

    if (localtime_r(&timeStamp, &tm) == nullptr || strftime(stampStr, timeLength, "%Y/%m/%d %H/%M/%S", &tm) == 0) {
        HIVIEW_LOGE("failed to get real time");
        return "error time fourmat!";
    }
    return string(stampStr);
}

string FaultDetectorUtil::TimeStampToSimplelyStr(const time_t timeStamp)
{
    struct tm tm;
    constexpr int timeLength = 64;
    char stampStr[timeLength] = { 0 };

    if (localtime_r(&timeStamp, &tm) == nullptr || strftime(stampStr, timeLength, "%Y%m%d%H%M%S", &tm) == 0) {
        HIVIEW_LOGE("failed to get real time");
        return "error time fourmat!";
    }
    return string(stampStr);
}

vector<string> FaultDetectorUtil::GetStatInfo(const string &path)
{
    string statInfo = ReadFileByChar(path);
    // process_name was included in pair (), find ")" as start for skip special in process_name.
    size_t pos = statInfo.find(')');
    if (pos != string::npos) {
        statInfo = statInfo.substr(++pos);
    }
    return Split(statInfo, ' ');
}

string FaultDetectorUtil::GetProcessName(int32_t pid)
{
    string path = "/proc/" + to_string(pid) + "/cmdline";
    string name = ReadFileByChar(path);
    // cmdline is empty ? use comm instead
    if (name.length() < 2) { // we consider namelen < 2 as empty.
        path = "/proc/" + to_string(pid) + "/comm";
        name = ReadFileByChar(path);
    }
    auto pos = name.find_last_not_of(" \n\r\t");
    if (pos != string::npos) {
        name.erase(pos + 1);
    }
    pos = name.find_last_of("/");
    name = name.substr(pos + 1);
    return name;
}

time_t FaultDetectorUtil::GetProcessStartTime(int pid)
{
    string path = "/proc/" + to_string(pid) + "/stat";
    vector<string> statInfo = GetStatInfo(path);
    if (statInfo.size() < MAX_INDEX) {
        return -1;
    }
    string startTime = statInfo[START_TIME_INDEX];
    if (!isdigit(startTime[0])) {
        HIVIEW_LOGE("failed to get process start time, reason: not digital, pid: %{public}d", pid);
        return -1;
    }
    return stol(startTime);
}

uint64_t FaultDetectorUtil::GetProcessRss(int pid)
{
    string path = "/proc/" + to_string(pid) + "/rss";
    string rssString = ReadFileByChar(path);
    string rss = "";
    for (char c : rssString) {
        if (isdigit(c)) {
            rss += c;
        }
    }
    if (rss.empty()) {
        HIVIEW_LOGD("failed to get process rss, reason: not digital, pid: %{public}d", pid);
        return 0;
    }
    return stoull(rss);
}

int FaultDetectorUtil::GetParentPid(int pid)
{
    string path = "/proc/" + to_string(pid) + "/stat";
    vector<string> statInfo = GetStatInfo(path);
    if (statInfo.size() < MAX_INDEX) {
        return 0;
    }
    string ppid = statInfo[PPID_INDEX];
    if (!isdigit(ppid[0])) {
        HIVIEW_LOGE("failed to get process parent pid, reason: not digital, pid: %{public}d", pid);
        return 0;
    }
    return stoi(ppid);
}

bool FaultDetectorUtil::IsKernelProcess(int pid)
{
    int ppid = GetParentPid(pid);
    return ppid == PID_KTHREADD;
}

time_t FaultDetectorUtil::GetRunningMonotonicTime()
{
    struct timespec now;
    now.tv_sec = 0;
    now.tv_nsec = 0;
    int32_t ret = clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    if (ret != 0) {
        HIVIEW_LOGE("failed to get monotonic time");
    }
    return now.tv_sec;
}

int32_t FaultDetectorUtil::GetProcessUid(int32_t pid)
{
    int32_t uid = -1;
    struct stat st;
    string path = "/proc/" + to_string(pid) + "/attr";
    if (stat(path.c_str(), &st) == -1) {
        HIVIEW_LOGE("stat %{public}s error", path.c_str());
        return uid;
    }
    uid = static_cast<int32_t>(st.st_uid);
    return uid;
}

string FaultDetectorUtil::GetApplicationNameByUid(int32_t uid)
{
    string bundleName;
    BundleMgrClient client;
    if (client.GetNameForUid(uid, bundleName) != ERR_OK) {
        HIVIEW_LOGE("failed to query bundleName from bms, uid: %{public}d", uid);
        return "";
    }
    HIVIEW_LOGI("bundleName of uid: %{public}d is %{public}s", uid, bundleName.c_str());
    return bundleName;
}

string FaultDetectorUtil::GetApplicationVersion(int32_t pid)
{
    int32_t uid = GetProcessUid(pid);
    if (uid < MIN_APP_USERID) {
        return "";
    }
    const string bundleName = GetApplicationNameByUid(uid);
    if (bundleName.empty()) {
        return "";
    }
    BundleInfo info;
    BundleMgrClient client;
    if (!client.GetBundleInfo(bundleName, BundleFlag::GET_BUNDLE_DEFAULT, info, Constants::ALL_USERID)) {
        HIVIEW_LOGE("failed to query BundleInfo from bms, uid: %{public}d", uid);
        return "";
    }
    HIVIEW_LOGI("the version of %{public}s is %{public}s", bundleName.c_str(), info.versionName.c_str());
    return info.versionName;
}

bool FaultDetectorUtil::IsBetaVersion()
{
    string userType = system::GetParameter(KEY_HIVIEW_USER_TYPE, "");
    return userType == "beta";
}

void FaultDetectorUtil::GetStatm(int32_t pid, uint64_t &vss, uint64_t &rss)
{
    ifstream statmStream("/proc/" + to_string(pid) + "/statm");
    if (!statmStream) {
        HIVIEW_LOGE("Fail to open /proc/%{public}d/statm", pid);
        return;
    }
    string statmLine;
    getline(statmStream, statmLine);
    HIVIEW_LOGI("/proc/%{public}d/statm : %{public}s", pid, statmLine.c_str());
    statmStream.close();
    list<string> numStrArr = GetDightStrArr(statmLine);
    auto it = numStrArr.begin();
    unsigned long long multiples = 4;
    vss = multiples * stoull(*it);
    it++;
    rss = multiples * stoull(*it);
}

void FaultDetectorUtil::GetMeminfo(uint64_t &avaliableMem, uint64_t &freeMem, uint64_t &totalMem)
{
    ifstream meminfoStream("/proc/meminfo");
    if (!meminfoStream) {
        HIVIEW_LOGE("Fail to open /proc/meminfo");
        return;
    }
    string meminfoLine;
    getline(meminfoStream, meminfoLine);
    totalMem = stoull(GetDightStrArr(meminfoLine).front());
    getline(meminfoStream, meminfoLine);
    freeMem = stoull(GetDightStrArr(meminfoLine).front());
    getline(meminfoStream, meminfoLine);
    avaliableMem = stoull(GetDightStrArr(meminfoLine).front());
    meminfoStream.close();
}

list<string> FaultDetectorUtil::GetDightStrArr(const string &target)
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

} // namespace HiviewDFX
} // namespace OHOS
