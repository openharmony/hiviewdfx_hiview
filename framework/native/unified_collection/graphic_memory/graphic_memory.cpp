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

#include "graphic_memory.h"

#include <v1_0/imemory_tracker_interface.h>
#include "file_helper.h"
#include "hilog/log.h"
#include "transaction/rs_interfaces.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D12

#undef LOG_TAG
#define LOG_TAG "graphic_memory"

namespace OHOS {
namespace HiviewDFX {
namespace Graphic {
namespace {
struct DmaInfo {
    uint64_t size = 0;
    uint64_t ino = 0;
    uint32_t pid = 0;
    std::string name;
};

const int BYTE_PER_KB = 1024;

int32_t GetGLByPid(int32_t pid)
{
    using namespace HDI::Memorytracker::V1_0;
    using namespace Rosen;
    int result = 0;
    sptr<IMemoryTrackerInterface> memtrack = IMemoryTrackerInterface::Get(true);
    if (memtrack == nullptr) {
        return result;
    }
    std::vector<MemoryRecord> records;
    if (memtrack->GetDevMem(pid, MEMORY_TRACKER_TYPE_GL, records) == HDF_SUCCESS) {
        int32_t value = 0;
        for (const auto& record : records) {
            if ((static_cast<uint32_t>(record.flags) & FLAG_UNMAPPED) == FLAG_UNMAPPED) {
                value = static_cast<int32_t>(record.size / BYTE_PER_KB);
                break;
            }
        }
        result = value;
    }
    auto& rsClient = RSInterfaces::GetInstance();
    std::unique_ptr<MemoryGraphic> memGraphic = std::make_unique<MemoryGraphic>(rsClient.GetMemoryGraphic(pid));
    if (memGraphic == nullptr) {
        return result;
    }
    result += memGraphic->GetGpuMemorySize() / BYTE_PER_KB;
    return result;
}

void CreateDmaInfo(const std::string &line, std::map<uint64_t, DmaInfo> &dmaInfoMap)
{
    DmaInfo dmaInfo;
    char name[256] = {0}; // 256: max size of name
    if (sscanf_s(line.c_str(), "%255s %llu %*llu %llu %llu %*n %*s %*s %*s",
                 name, sizeof(name), &dmaInfo.pid, &dmaInfo.size, &dmaInfo.ino) == 0) {
        return;
    }
    dmaInfo.name = name;
    dmaInfo.size /= BYTE_PER_KB;
    auto it = dmaInfoMap.find(dmaInfo.ino);
    if (it == dmaInfoMap.end()) {
        dmaInfoMap.insert(std::pair<uint64_t, DmaInfo>(dmaInfo.ino, dmaInfo));
    } else if ((dmaInfo.name != "composer_host" && dmaInfo.name != "render_service") ||
               (dmaInfo.name == "render_service" && it->second.name == "composer_host")) {
        it->second.pid = dmaInfo.pid;
    }
}

int32_t GetGraphByPid(pid_t pid)
{
    std::string path = "/proc/process_dmabuf_info";
    std::map<uint64_t, DmaInfo> dmaInfoMap;
    std::string content;
    FileHelper::ReadFileByLine(path, [&](std::string &line) -> bool {
        CreateDmaInfo(line, dmaInfoMap);
        return false;
    });

    std::map<uint32_t, uint64_t> dmaMap;
    for (const auto &it : dmaInfoMap) {
        auto dma = dmaMap.find(it.second.pid);
        if (dma != dmaMap.end()) {
            dma->second += it.second.size;
        } else {
            dmaMap.insert(std::pair<uint32_t, uint64_t>(it.second.pid, it.second.size));
        }
    }

    auto it = dmaMap.find(pid);
    if (it == dmaMap.end()) {
        return 0;
    }
    return static_cast<int32_t>(it->second);
}
} // namespace


CollectResult GetGraphicUsage(int32_t pid, Type type)
{
    CollectResult result{};
    result.retCode = ResultCode::SUCCESS;
    if (type == Type::GL) {
        result.graphicData = GetGLByPid(pid);
        HILOG_INFO(LOG_CORE, "pid=%{public}d, gl=%{public}d", pid, result.graphicData);
    } else if (type == Type::GRAPH) {
        result.graphicData = GetGraphByPid(pid);
        HILOG_INFO(LOG_CORE, "pid=%{public}d, graph=%{public}d", pid, result.graphicData);
    } else if (type == Type::TOTAL) {
        int32_t gl = GetGLByPid(pid);
        int32_t graph = GetGraphByPid(pid);
        result.graphicData = gl + graph;
        HILOG_INFO(LOG_CORE, "pid=%{public}d, total=%{public}d, gl=%{public}d, graph=%{public}d",
            pid, result.graphicData, gl, graph);
    } else {
        result.retCode = ResultCode::FAIL;
    }
    return result;
}

} // namespace Graphic
} // namespace HiviewDFX
} // namespace OHOS