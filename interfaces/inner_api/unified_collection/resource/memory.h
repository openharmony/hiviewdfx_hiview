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

#ifndef INTERFACES_INNER_API_UNIFIED_COLLECTION_RESOURCE_MEMORY_H
#define INTERFACES_INNER_API_UNIFIED_COLLECTION_RESOURCE_MEMORY_H
#include <cinttypes>
#include <string>

namespace OHOS {
namespace HiviewDFX {
struct SysMemory {
    int32_t memTotal;     // total amount of usable RAM, unit KB
    int32_t memFree;      // unit KB
    int32_t memAvailable; // unit KB
    int32_t zramUsed;     // unit KB
    int32_t swapCached;   // unit KB
    int32_t cached;       // unit KB
};

struct ProcessMemory {
    int32_t pid;            // process id
    std::string name;       // process name
    int32_t rss;            // resident set size, unit KB
    int32_t pss;            // proportional set Size, unit KB
    int32_t swapPss;        // swap pss, unit KB
    int32_t adj;            // /proc/$pid/oom_score_adj
    int32_t sharedDirty;   //process Shared_Dirty
    int32_t privateDirty;  //process Private_Dirty
};

struct MemoryLimit {
    uint64_t rssLimit;
    uint64_t vssLimit;
};

extern "C" {
const int HIAI_MAX_QUERIED_USER_MEMINFO_LIMIT = 256;

typedef struct {
    int pid;    // process id
    int size;   // byte
} AIProcessMem;
};
} // HiviewDFX
} // OHOS
#endif // INTERFACES_INNER_API_UNIFIED_COLLECTION_RESOURCE_MEMORY_H
