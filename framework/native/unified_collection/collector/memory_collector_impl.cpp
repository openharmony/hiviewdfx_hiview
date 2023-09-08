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

#include "common_util.h"
#include "common_utils.h"
#include "file_util.h"
#include "logger.h"

using namespace OHOS::HiviewDFX::UCollect;

DEFINE_LOG_TAG("UCollectUtil");

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {

class MemoryCollectorImpl : public MemoryCollector {
public:
    MemoryCollectorImpl() = default;
    virtual ~MemoryCollectorImpl() = default;

public:
    virtual CollectResult<ProcessMemory> CollectProcessMemory(int32_t pid) override;
    virtual CollectResult<SysMemory> CollectSysMemory() override;
};

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
            }
        }
    }
    result.retCode = UcError::SUCCESS;
    return result;
}
} // UCollectUtil
} // HiViewDFX
} // OHOS