/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "memory_collector.h"

#include <fstream>
#include <string_ex.h>
#include <dlfcn.h>
#include <securec.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <regex>
#include <map>
#include <mutex>

#include "common_util.h"
#include "common_utils.h"
#include "file_util.h"
#include "time_util.h"
#include "string_util.h"
#include "logger.h"

const std::size_t MAX_FILE_SAVE_SIZE = 10;
const std::size_t WIDTH = 12;
const std::size_t DELAY_MILLISEC = 3;
using namespace OHOS::HiviewDFX::UCollect;

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
DEFINE_LOG_TAG("UCollectUtil");

std::mutex g_memMutex;

class MemoryCollectorImpl : public MemoryCollector {
public:
    MemoryCollectorImpl() = default;
    virtual ~MemoryCollectorImpl() = default;

public:
    virtual CollectResult<ProcessMemory> CollectProcessMemory(int32_t pid) override;
    virtual CollectResult<SysMemory> CollectSysMemory() override;
    virtual CollectResult<std::string> CollectRawMemInfo() override;
    virtual CollectResult<std::vector<ProcessMemory>> CollectAllProcessMemory() override;
    virtual CollectResult<std::string> ExportAllProcessMemory() override;
    virtual CollectResult<std::string> CollectRawSlabInfo() override;
    virtual CollectResult<std::string> CollectRawPageTypeInfo() override;
    virtual CollectResult<std::string> CollectRawDMA() override;
    virtual CollectResult<std::vector<AIProcessMem>> CollectAllAIProcess() override;
    virtual CollectResult<std::string> ExportAllAIProcess() override;
    virtual CollectResult<std::string> CollectRawSmaps(int32_t pid) override;
    virtual CollectResult<std::string> CollectHprof(int32_t pid) override;
};

static std::string GetCurrTimestamp()
{
    auto logTime = TimeUtil::GetMilliseconds() / TimeUtil::SEC_TO_MILLISEC;
    return TimeUtil::TimestampFormatToDate(logTime, "%Y%m%d%H%M%S");
}

static std::string GetSavePath(const std::string& preFix, const std::string& ext)
{
    std::lock_guard<std::mutex> lock(g_memMutex);   // lock when get save path
    std::string timeStamp = GetCurrTimestamp();
    std::string savePath = MEMINFO_SAVE_DIR + "/" + preFix + timeStamp + ext;
    int suffix = 0;
    while (FileUtil::FileExists(savePath)) {
        std::stringstream ss;
        ss << MEMINFO_SAVE_DIR << "/" << preFix << timeStamp << "_" << suffix << ext;
        suffix++;
        savePath = ss.str();
    }
    if (creat(savePath.c_str(), FileUtil::DEFAULT_FILE_MODE) == -1) {
        HIVIEW_LOGE("create %{public}s failed, errno=%{public}d.", savePath.c_str(), errno);
        return "";
    }
    return savePath;
}

static bool WriteProcessMemoryToFile(std::string& filePath, const std::vector<ProcessMemory>& processMems)
{
    std::ofstream file;
    file.open(filePath.c_str(), std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        HIVIEW_LOGE("open %{public}s failed.", filePath.c_str());
        return false;
    }

    file << "pid" << '\t' << "pname" << '\t' << "rss(KB)" << '\t' <<
            "pss(KB)" << '\t' << "swapPss(KB)"<< '\t' << "adj" << std::endl;
    for (auto& processMem : processMems) {
        file << processMem.pid << '\t' << processMem.name << '\t' << processMem.rss << '\t' <<
                processMem.pss << '\t' << processMem.swapPss << '\t' << processMem.adj << std::endl;
    }
    file.close();
    return true;
}

static bool WriteAIProcessMemToFile(std::string& filePath, const std::vector<AIProcessMem>& aiProcMems)
{
    std::ofstream file;
    file.open(filePath.c_str(), std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        HIVIEW_LOGE("open %{public}s failed.", filePath.c_str());
        return false;
    }

    file << "pid" << '\t' << "mem(byte)" << std::endl;
    for (auto& aiProcMem : aiProcMems) {
        file << std::setw(WIDTH) << std::left << aiProcMem.pid << '\t' << aiProcMem.size << std::endl;
    }
    file.close();
    return true;
}

static bool GetSmapsFromProcPath(const std::string& procPath, ProcessMemory& procMem)
{
    std::string content;
    if (!FileUtil::FileExists(procPath)) {
        HIVIEW_LOGI("%{public}s is not exist, process exit.", procPath.c_str());
        return false;
    }
    if (!FileUtil::LoadStringFromFile(procPath, content)) {
        HIVIEW_LOGE("load string from %{public}s failed.", procPath.c_str());
        return false;
    }
    std::stringstream ss(content);
    std::string line;
    unsigned rss = 0;
    unsigned pss = 0;
    unsigned swapPss = 0;

    while (std::getline(ss, line)) {
        unsigned temp;
        if (sscanf_s(line.c_str(), "Rss: %u kB", &temp) == 1) {
            rss += temp;
        } else if (sscanf_s(line.c_str(), "Pss: %u kB", &temp) == 1) {
            pss += temp;
        } else if (sscanf_s(line.c_str(), "SwapPss: %u kB", &temp) == 1) {
            swapPss += temp;
        }
    }
    procMem.rss = rss;
    procMem.pss = pss;
    procMem.swapPss = swapPss;
    return true;
}

static bool ReadMemFromAILib(AIProcessMem memInfos[], int len, int& realSize)
{
    std::string libName = "libai_infra.so";
    std::string interface = "HIAI_Memory_QueryAllUserAllocatedMemInfo";
    void* handle = dlopen(libName.c_str(), RTLD_LAZY);
    if (!handle) {
        HIVIEW_LOGE("dlopen %{public}s failed, %{public}s.", libName.c_str(), dlerror());
        return false;
    }
    using AIFunc = int (*)(AIProcessMem[], int, int*);
    AIFunc aiFunc = reinterpret_cast<AIFunc>(dlsym(handle, interface.c_str()));
    if (!aiFunc) {
        HIVIEW_LOGE("dlsym %{public}s failed, %{public}s.", libName.c_str(), dlerror());
        dlclose(handle);
        return false;
    }
    int memInfoSize = len;
    int ret = aiFunc(memInfos, memInfoSize, &realSize);
    HIVIEW_LOGI("exec %{public}s, ret=%{public}d.", interface.c_str(), ret);
    if (realSize < 0) {
        dlclose(handle);
        return false;
    }
    dlclose(handle);
    return true;
}

static void DoClearFiles(const std::string& filePrefix)
{
    // Filter files with same prefix
    std::vector<std::string> files;
    FileUtil::GetDirFiles(MEMINFO_SAVE_DIR, files, false);
    std::map<uint64_t, std::string> fileLists;
    for (auto& file : files) {
        std::string fileName = FileUtil::ExtractFileName(file);
        if (!CommonUtil::StartWith(fileName, filePrefix)) {
            continue;
        }
        struct stat fileInfo;
        if (stat(file.c_str(), &fileInfo) != 0) {
            HIVIEW_LOGE("stat %{public}s failed.", file.c_str());
            continue;
        }
        fileLists.insert(std::pair<uint64_t, std::string>(fileInfo.st_mtime, file));
    }

    size_t len = fileLists.size();
    if (len <= MAX_FILE_SAVE_SIZE) {
        HIVIEW_LOGI("%{public}d files with same prefix %{public}s.", len, filePrefix.c_str());
        return;
    }
    // clear more than 10 old files
    size_t count = len - MAX_FILE_SAVE_SIZE;
    for (auto it = fileLists.begin(); it != fileLists.end() && count > 0; ++it, --count) {
        if (!FileUtil::RemoveFile(it->second)) {
            HIVIEW_LOGE("remove %{public}s failed.", it->second.c_str());
        }
        HIVIEW_LOGI("succ remove %{public}s.", it->second.c_str());
    }
}

static CollectResult<std::string> CollectRawInfo(const std::string& filePath, const std::string& preFix,
                                                 bool doClearFlag = true)
{
    CollectResult<std::string> result;
    std::string content;
    if (!FileUtil::LoadStringFromFile(filePath, content)) {
        result.retCode = UcError::READ_FAILED;
        return result;
    }

    if ((!FileUtil::FileExists(MEMINFO_SAVE_DIR)) &&
        (!FileUtil::ForceCreateDirectory(MEMINFO_SAVE_DIR, FileUtil::FILE_PERM_755))) {
        HIVIEW_LOGE("create %{public}s dir failed.", MEMINFO_SAVE_DIR.c_str());
        result.retCode = UcError::WRITE_FAILED;
        return result;
    }
    result.data = GetSavePath(preFix, ".txt");
    if (result.data.empty()) {
        result.retCode = UcError::WRITE_FAILED;
        return result;
    }
    HIVIEW_LOGI("save path is %{public}s.", result.data.c_str());
    if (!FileUtil::SaveStringToFile(result.data, content)) {
        HIVIEW_LOGE("save to %{public}s failed, content is %{public}s.", result.data.c_str(), content.c_str());
        result.retCode = UcError::WRITE_FAILED;
        return result;
    }
    if (doClearFlag) {
        DoClearFiles(preFix);
    }
    result.retCode = UcError::SUCCESS;
    return result;
}

std::shared_ptr<MemoryCollector> MemoryCollector::Create()
{
    return std::make_shared<MemoryCollectorImpl>();
}

CollectResult<ProcessMemory> MemoryCollectorImpl::CollectProcessMemory(int32_t pid)
{
    CollectResult<ProcessMemory> result;
    std::string filename = PROC + std::to_string(pid) + SMAPS_ROLLUP;
    std::string content;
    FileUtil::LoadStringFromFile(filename, content);
    std::vector<std::string> vec;
    OHOS::SplitStr(content, "\n", vec);
    ProcessMemory& processMemory = result.data;
    processMemory.pid = pid;
    processMemory.name = CommonUtils::GetProcNameByPid(pid);
    std::string type;
    int32_t value = 0;
    for (const std::string &str : vec) {
        if (CommonUtil::ParseTypeAndValue(str, type, value)) {
            if (type == "Rss") {
                processMemory.rss = value;
                HIVIEW_LOGD("Rss=%{public}d", processMemory.rss);
            } else if (type == "Pss") {
                processMemory.pss = value;
                HIVIEW_LOGD("Pss=%{public}d", processMemory.pss);
            }
        }
    }
    result.retCode = UcError::SUCCESS;
    return result;
}

CollectResult<SysMemory> MemoryCollectorImpl::CollectSysMemory()
{
    CollectResult<SysMemory> result;
    std::string content;
    FileUtil::LoadStringFromFile(MEM_INFO, content);
    std::vector<std::string> vec;
    OHOS::SplitStr(content, "\n", vec);
    SysMemory& sysmemory = result.data;
    std::string type;
    int32_t value = 0;
    for (const std::string &str : vec) {
        if (CommonUtil::ParseTypeAndValue(str, type, value)) {
            if (type == "MemTotal") {
                sysmemory.memTotal = value;
                HIVIEW_LOGD("memTotal=%{public}d", sysmemory.memTotal);
            } else if (type == "MemFree") {
                sysmemory.memFree = value;
                HIVIEW_LOGD("memFree=%{public}d", sysmemory.memFree);
            } else if (type == "MemAvailable") {
                sysmemory.memAvailable = value;
                HIVIEW_LOGD("memAvailable=%{public}d", sysmemory.memAvailable);
            } else if (type == "ZramUsed") {
                sysmemory.zramUsed = value;
                HIVIEW_LOGD("zramUsed=%{public}d", sysmemory.zramUsed);
            } else if (type == "SwapCached") {
                sysmemory.swapCached = value;
                HIVIEW_LOGD("swapCached=%{public}d", sysmemory.swapCached);
            } else if (type == "Cached") {
                sysmemory.cached = value;
                HIVIEW_LOGD("cached=%{public}d", sysmemory.cached);
            }
        }
    }
    result.retCode = UcError::SUCCESS;
    return result;
}

CollectResult<std::string> MemoryCollectorImpl::CollectRawMemInfo()
{
    return CollectRawInfo(MEM_INFO, "proc_meminfo_");
}

CollectResult<std::vector<ProcessMemory>> MemoryCollectorImpl::CollectAllProcessMemory()
{
    CollectResult<std::vector<ProcessMemory>> result;
    std::vector<ProcessMemory> procMemoryVec;
    std::vector<std::string> procFiles;
    FileUtil::GetDirFiles(PROC, procFiles, false);
    for (auto& procFile : procFiles) {
        std::string fileName = FileUtil::ExtractFileName(procFile);
        int value = 0;
        if (!StringUtil::StrToInt(fileName, value)) {
            HIVIEW_LOGD("%{public}s is not num string, value=%{public}d.", fileName.c_str(), value);
            continue;
        }
        std::string smapsPath = PROC + fileName + "/smaps_rollup";
        std::string adjPath = PROC + fileName + "/oom_score_adj";
        ProcessMemory procMemory;
        procMemory.pid = value;
        procMemory.name = CommonUtils::GetProcNameByPid(value);
        if (!GetSmapsFromProcPath(smapsPath, procMemory)) {
            continue;
        }
        std::string adj;
        if (!FileUtil::LoadStringFromFile(adjPath, adj)) {
            HIVIEW_LOGE("load string from %{public}s failed.", adjPath.c_str());
            continue;
        }
        procMemory.adj = std::stoi(adj);

        procMemoryVec.emplace_back(procMemory);
    }
    result.data = procMemoryVec;
    result.retCode = UcError::SUCCESS;
    return result;
}

CollectResult<std::string> MemoryCollectorImpl::ExportAllProcessMemory()
{
    CollectResult<std::string> result;
    CollectResult<std::vector<ProcessMemory>> processMemory = this->CollectAllProcessMemory();
    if (processMemory.retCode != UcError::SUCCESS) {
        result.retCode = processMemory.retCode;
        return result;
    }

    std::string savePath = GetSavePath("all_processes_mem_", ".txt");
    if (savePath.empty()) {
        result.retCode = UcError::WRITE_FAILED;
        return result;
    }
    if (!WriteProcessMemoryToFile(savePath, processMemory.data)) {
        result.retCode = UcError::WRITE_FAILED;
        return result;
    }
    DoClearFiles("all_processes_mem_");
    result.data = savePath;
    result.retCode = UcError::SUCCESS;
    return result;
}

CollectResult<std::string> MemoryCollectorImpl::CollectRawSlabInfo()
{
    return CollectRawInfo("/proc/slabinfo", "proc_slabinfo_");
}

CollectResult<std::string> MemoryCollectorImpl::CollectRawPageTypeInfo()
{
    return CollectRawInfo("/proc/pagetypeinfo", "proc_pagetypeinfo_");
}

CollectResult<std::string> MemoryCollectorImpl::CollectRawDMA()
{
    return CollectRawInfo("/proc/process_dmabuf_info", "proc_process_dmabuf_info_");
}

CollectResult<std::vector<AIProcessMem>> MemoryCollectorImpl::CollectAllAIProcess()
{
    CollectResult<std::vector<AIProcessMem>> result;
    AIProcessMem memInfos[HIAI_MAX_QUERIED_USER_MEMINFO_LIMIT];
    int realSize = 0;
    if (!ReadMemFromAILib(memInfos, HIAI_MAX_QUERIED_USER_MEMINFO_LIMIT, realSize)) {
        result.retCode = UcError::READ_FAILED;
        return result;
    }

    for (int i = 0; i < realSize; ++i) {
        result.data.emplace_back(memInfos[i]);
        HIVIEW_LOGD("memInfo: pid=%{public}d, size=%{public}d.", memInfos[i].pid, memInfos[i].size);
    }
    result.retCode = UcError::SUCCESS;
    return result;
}

CollectResult<std::string> MemoryCollectorImpl::ExportAllAIProcess()
{
    CollectResult<std::string> result;
    CollectResult<std::vector<AIProcessMem>> aiProcessMem = this->CollectAllAIProcess();
    if (aiProcessMem.retCode != UcError::SUCCESS) {
        result.retCode = aiProcessMem.retCode;
        return result;
    }

    std::string savePath = GetSavePath("all_ai_processes_mem_", ".txt");
    if (savePath.empty()) {
        result.retCode = UcError::WRITE_FAILED;
        return result;
    }
    if (!WriteAIProcessMemToFile(savePath, aiProcessMem.data)) {
        result.retCode = UcError::WRITE_FAILED;
        return result;
    }
    DoClearFiles("all_ai_processes_mem_");
    result.data = savePath;
    result.retCode = UcError::SUCCESS;
    return result;
}

CollectResult<std::string> MemoryCollectorImpl::CollectRawSmaps(int32_t pid)
{
    std::string pidStr = std::to_string(pid);
    std::string fileName = PROC + pidStr + "/smaps";
    std::string preFix = "proc_smaps_" + pidStr + "_";
    CollectResult<std::string> result = CollectRawInfo(fileName, preFix, false);
    DoClearFiles("proc_smaps_");
    return result;
}

static std::string GetNewestSnapshotPath(const std::string& path)
{
    std::string latestFilePath;
    time_t newestFileTime;
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        return "";
    }
    while (true) {
        struct dirent *ptr = readdir(dir);
        if (ptr == nullptr) {
            break;
        }
        if ((!CommonUtil::StartWith(ptr->d_name, "jsheap") && !CommonUtil::EndWith(ptr->d_name, "heapsnapshot"))) {
            continue;
        }

        std::string snapshotPath = FileUtil::IncludeTrailingPathDelimiter(path) + std::string(ptr->d_name);
        struct stat st;
        if ((stat(snapshotPath.c_str(), &st) == 0) && (st.st_mtime > newestFileTime)) {
            newestFileTime = st.st_mtime;
            latestFilePath = snapshotPath;
        }
    }
    closedir(dir);
    return latestFilePath;
}

static std::string GetSnapshotPath(const std::string& dirPath, const std::string& pidStr)
{
    std::string fileName = GetNewestSnapshotPath(dirPath);
    if (fileName.empty()) {
        HIVIEW_LOGI("not newest snapshot file gerenated.");
        return "";
    }

    std::string parsePidStr;
    std::regex pattern(".*-(\\d+)-.*");
    std::smatch match;
    if (std::regex_search(fileName, match, pattern)) {
        parsePidStr = match[1].str();
    }
    if (pidStr.compare(parsePidStr) != 0) {
        HIVIEW_LOGI("%{public}s is not suitable pid.", fileName.c_str());
        return "";
    }
    return fileName;
}

CollectResult<std::string> MemoryCollectorImpl::CollectHprof(int32_t pid)
{
    CollectResult<std::string> result;
    std::string pidStr = std::to_string(pid);
    if (kill(pid, 40) != 0) {   // kill -40
        HIVIEW_LOGE("send kill-signal failed, pid=%{public}d, errno=%{public}d.", pid, errno);
        result.retCode = UcError::UNSUPPORT;
        return result;
    }
    TimeUtil::Sleep(DELAY_MILLISEC);

    std::string preFix = "jsheap_" + pidStr + "_";
    std::string savePath = GetSavePath(preFix, ".snapshot");
    if (savePath.empty()) {
        result.retCode = UcError::WRITE_FAILED;
        return result;
    }

    std::string srcFilePath = GetSnapshotPath("/data/log/faultlog/temp", pidStr);
    if (srcFilePath.empty()) {
        srcFilePath = GetSnapshotPath("/data/log/reliability/resource_leak/memory_leak", pidStr);
        if (srcFilePath.empty()) {
            std::string procName = CommonUtils::GetProcNameByPid(pid);
            std::string content = "unsupport dump js heap snapshot: " + procName;
            if (!FileUtil::SaveStringToFile(savePath, content)) {
                HIVIEW_LOGE("save to %{public}s failed, content is %{public}s.", savePath.c_str(), content.c_str());
                result.retCode = UcError::WRITE_FAILED;
                return result;
            }
            DoClearFiles("jsheap_");
            result.data = savePath;
            result.retCode = UcError::SUCCESS;
            return result;
        }
    }

    if (FileUtil::CopyFile(srcFilePath, savePath) != 0) {
        HIVIEW_LOGE("copy from %{public}s to %{public}s failed.", srcFilePath.c_str(), savePath.c_str());
        result.retCode = UcError::WRITE_FAILED;
        return result;
    }
    DoClearFiles("jsheap_");
    result.data = savePath;
    result.retCode = UcError::SUCCESS;
    return result;
}
} // UCollectUtil
} // HiViewDFX
} // OHOS
