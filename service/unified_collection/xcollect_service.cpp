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
#include <cstdlib>
#include <ctime>
#include <dirent.h>
#include <fstream>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <string>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include "collect_item_result.h"
#include "collect_parameter.h"
#include "hitrace_dump.h"
#include "hilog/log.h"
#include "xcollect_service.h"

using namespace std;
using namespace OHOS::HiviewDFX::Hitrace;
namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D10, "HiView-xcollect_service" };
}
class TraceService  {
public:
    TraceService()
    {
        const std::vector<std::string> tagGroups = {"scene_performance"};
        if (OpenTrace(tagGroups) != TraceErrorCode::SUCCESS) {
            HiLog::Error(LABEL, "OpenTrace fail.");
        }
        HiLog::Info(LABEL, "OpenTrace success.");
    }
};

TraceService g_traceService;

class DefCollectCallback : public CollectCallback {
public:
    virtual void Handle(std::shared_ptr<CollectItemResult> dara, bool isFinish)
    {}
};

XcollectService::XcollectService(std::shared_ptr<CollectParameter> collectParameter)
    : collectParameter_(collectParameter), callback_(std::make_shared<DefCollectCallback>())
{}

XcollectService::XcollectService(std::shared_ptr<CollectParameter> collectParameter,
    std::shared_ptr<CollectCallback> callback) : collectParameter_(collectParameter), callback_(callback)
{
    if (callback_ == nullptr) {
        callback_ = std::make_shared<DefCollectCallback>();
    }
}

XcollectService::~XcollectService()
{}

void XcollectService::StartCollect()
{
    for (auto it = collectParameter_->items_.begin(); it != collectParameter_->items_.end(); it++) {
        if (it->rfind("/hitrace/")==0) {
            CollectHiTrace(*it);
        }
    }
}

namespace {
std::vector<std::pair<std::string, uint64_t>> g_traceDirTable;
bool CopyTmpFile(const std::string &src, const std::string &dest)
{
    char srcPath[PATH_MAX + 1] = {0x00};
    if (strlen(src.c_str()) > PATH_MAX || realpath(src.c_str(), srcPath) == nullptr) {
        return false;
    }
    int srcFd = open(srcPath, O_RDONLY | O_NONBLOCK);
    if (srcFd < 0) {
        close(srcFd);
        HiLog::Error(LABEL, "CopyFile: open %{public}s failed.", srcPath);
        return false;
    }

    char destPath[PATH_MAX + 1] = {0x00};
    if (strlen(dest.c_str()) > PATH_MAX || realpath(dest.c_str(), destPath) == nullptr) {
        return false;
    }
    int destFd = open(destPath, O_WRONLY | O_CREAT, FILE_RIGHT_READ);
    if (destFd < 0) {
        close(srcFd);
        HiLog::Error(LABEL, "CopyFile: open %{public}s failed.", destPath);
        return false;
    }

    const int bufferSize = 4096;
    char* buffer = (char*)calloc(bufferSize * sizeof(char), sizeof(char));
    if (buffer == nullptr) {
        HiLog::Error(LABEL, "CopyFile: calloc failed.");
        close(srcFd);
        close(destFd);
        return false;
    }
    do {
        ssize_t len = read(srcFd, buffer, bufferSize);
        if (len <= 0) {
            break;
        }
        write(destFd, buffer, len);
    } while (true);
    close(destFd);
    close(srcFd);
    return true;
}

void RemoveDir(const std::string &dirName)
{
    if (access(dirName.c_str(), F_OK) != 0) {
        return;
    }
    const std::string alignStr = "/data/log/hitrace/";
    if (dirName.compare(0, alignStr.size(), alignStr) != 0) {
        return;
    }
    DIR *dirPtr = opendir(dirName.c_str());
    if (!dirPtr) {
        return;
    }
    struct dirent *ptr;
    const int normalFileType = 8;
    while ((ptr = readdir(dirPtr)) != nullptr) {
        std::string subFile = dirName + "/" + ptr->d_name;
        if (ptr->d_type == normalFileType) {
            if (remove(subFile.c_str()) != 0) {
                HiLog::Error(LABEL, "error|RemoveDir: delete %{public}s failed.", subFile.c_str());
            }
        }
    }
    if (rmdir(dirName.c_str()) != 0) {
        HiLog::Error(LABEL, "error|RemoveDir: delete %{public}s failed.", dirName.c_str());
    }
}
} // namespace

std::string XcollectService::DumpTraceToDir()
{
    TraceRetInfo ret = DumpTrace();
    if (ret.errorCode != TraceErrorCode::SUCCESS) {
        HiLog::Error(LABEL, "error|DumpTrace: failed to dump trace, error code (%{public}d).", ret.errorCode);
        return "";
    }
    const std::string defaultParentDir = "/data/log/hitrace/";
    const std::string logDir = "/data/log/";
    if (access(defaultParentDir.c_str(), F_OK) != 0) {
        if (access(logDir.c_str(), F_OK) != 0) {
            mkdir(logDir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
        }
        mkdir(defaultParentDir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IROTH);
    }
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    uint64_t nowSec = now.tv_sec;
    uint64_t nowUsec = now.tv_usec;
    const std::string outDir = defaultParentDir + "trace_" + std::to_string(nowSec) + "_" + std::to_string(nowUsec);
    mkdir(outDir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IROTH);
    g_traceDirTable.push_back({outDir, nowSec});

    for (auto trace : ret.outputFiles) {
        size_t pos = trace.rfind('/');
        if (pos == std::string::npos) {
            HiLog::Error(LABEL, "error|DumpTraceToDir Error: copy to this dir failed.");
            return "";
        }
        std::string dst = outDir + trace.substr(pos);
        CopyTmpFile(trace, dst);
    }
    const uint64_t agingTime = 30 * 60;
    for (auto iter = g_traceDirTable.begin(); iter != g_traceDirTable.end();) {
        if (nowSec - iter->second >= agingTime) {
            RemoveDir(iter->first);
            HiLog::Info(LABEL, "Info|RemoveDir: delete old %{public}s dir success.", iter->first.c_str());
            iter = g_traceDirTable.erase(iter);
            continue;
        }
        iter++;
    }
    return outDir;
}

void XcollectService::CollectHiTrace(const std::string &item)
{
    if (item == "/hitrace/client/dump") {
        std::string outTraceDir = DumpTraceToDir();
        std::shared_ptr<CollectItemResult> collectItem = std::make_shared<CollectItemResult>();
        collectItem->SetCollectItemValue(item, outTraceDir);
        callback_->Handle(collectItem, true);
        return;
    }

    if (item == "/hitrace/cmd/open") {
        std::string args = "tags::sched clockType:boot bufferSize:1024 overwrite:1";
        if (CloseTrace() != TraceErrorCode::SUCCESS) {
            HiLog::Error(LABEL, "CloseTrace failed");
        }
        if (OpenTrace(args) != TraceErrorCode::SUCCESS) {
            HiLog::Error(LABEL, "OpenTrace failed");
        }
        return;
    }

    if (item == "/hitrace/cmd/traceon") {
        if (DumpTraceOn() != TraceErrorCode::SUCCESS) {
            HiLog::Error(LABEL, "traceon failed");
        }
        return;
    }

    if (item == "/hitrace/cmd/traceoff") {
        TraceRetInfo ret = DumpTraceOff();
        if (ret.errorCode != TraceErrorCode::SUCCESS) {
            HiLog::Error(LABEL, "traceoff failed");
        }
        return;
    }

    if (item == "/hitrace/cmd/close") {
        const std::vector<std::string> tagGroups = {"scene_performance"};
        if (CloseTrace() != TraceErrorCode::SUCCESS) {
            HiLog::Error(LABEL, "CloseTrace failed");
        }
        if (OpenTrace(tagGroups) != TraceErrorCode::SUCCESS) {
            HiLog::Error(LABEL, "OpenTrace failed");
        }
        return;
    }
}
} // namespace HiviewDFX
} // namespace OHOS
