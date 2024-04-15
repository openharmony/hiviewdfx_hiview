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
#ifndef FAULT_DETECTOR_UTILS_H
#define FAULT_DETECTOR_UTILS_H

#include <ctime>
#include <list>
#include <memory>
#include <string>

#define LEAK_SHA256_LENGTH      32

namespace OHOS {
namespace HiviewDFX {
// common interface
constexpr int TASK_LOOP_INTERVAL = 5;
const std::string UNKNOWN_PROCESS = "unknown_process";
const std::string KEY_HIVIEW_USER_TYPE = "const.logsystem.versiontype";
const std::string RELIABILITY_PATH = "/data/log/reliability";
const std::string RESOURCE_LEAK_PATH = "/data/log/reliability/resource_leak";
const std::string MEMORY_LEAK_PATH = "/data/log/reliability/resource_leak/memory_leak";
const std::string DUMP_NOTIFY_LOG_PATH = "/data/log/faultlog/temp";
constexpr uint32_t UNIT_OFFSET_BYTE = 1024;

// leaked grade
const std::string MEMORY_LEAK_ERROR = "ERROR";
const std::string MEMORY_LEAK_WARNING = "WARNING";
const std::string MEMORY_LEAK_INFO = "INFO";
const std::string NO_MEMORY_LEAK = "/";

class FaultDetectorUtil {
public:
    static bool IsMemLeakEnable();
    static bool IsMemTestEnable();
    static bool GetIsHmKernel();
    static std::string GetMemInfoPath();
    static std::vector<std::string> Split(const std::string &str, char delim);
    static bool IsDirectory(const std::string &path);
    static std::vector<std::string> GetSubDir(const std::string &path, bool digit = false);
    static std::vector<std::string> GetSubFile(const std::string &path, bool digit = false);
    static std::vector<int32_t> GetAllPids();
    static std::string ReadFileByChar(const std::string &path);
    static bool RenameFile(const std::string &path, const std::string &newPath);
    static std::string GetRealTime();
    static std::string GetRealTimeStampStr();
    static std::string TimeStampToStr(const time_t timeStamp);
    static std::string TimeStampToSimplelyStr(const time_t timeStamp);
    static std::vector<std::string> GetStatInfo(const std::string &path);
    static std::string GetProcessName(int32_t pid);
    static time_t GetProcessStartTime(int pid); // jiffes unit
    static uint64_t GetProcessRss(int pid);
    static int GetParentPid(int pid);
    static bool IsKernelProcess(int pid);
    static time_t GetRunningMonotonicTime();
    static int32_t GetProcessUid(int32_t pid);
    static std::string GetApplicationNameByUid(int32_t uid);
    static std::string GetApplicationVersion(int32_t pid);
    static bool IsBetaVersion();
    static void GetStatm(int32_t pid, uint64_t &vss, uint64_t &rss);
    static void GetMeminfo(uint64_t &avaliableMem, uint64_t &freeMem, uint64_t &totalMem);
    static std::list<std::string> GetDightStrArr(const std::string &target);
};
} // HiviewDFX
} // OHOS
#endif // FAULT_DETECTOR_UTILS_H
