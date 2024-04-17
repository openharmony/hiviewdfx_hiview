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
#ifndef NATIVE_LEAK_UTIL_H
#define NATIVE_LEAK_UTIL_H

#include <memory>
#include <string>

#include <sys/ioctl.h>

#include "fault_info_base.h"

namespace OHOS {
namespace HiviewDFX {
constexpr uint32_t MTYPE_USER_PSS_JAVA = 1;
constexpr uint32_t MTYPE_USER_PSS_NATIVE = (1 << 1);
constexpr uint32_t MTYPE_USER_PSS = (MTYPE_USER_PSS_JAVA | MTYPE_USER_PSS_NATIVE);

enum {
    NATIVE_MEMORY = 0,
    MEMORY_TYPE_CNT
};

constexpr uint32_t MEMCHECK_STACKINFO_MAXSIZE = 600 * 1024;
constexpr uint32_t MEMCHECK_DETAILINFO_MAXSIZE = 100 * 1024;
constexpr uint32_t MEMORY_ERROR_GRADE = 10 * 1024;
constexpr uint32_t MEMORY_WARNING_GRADE = 6 * 1024;
constexpr uint32_t MEMORY_RATING_LINE = 20 * 1024;
constexpr uint32_t DEFAULT_THRESHOLD = 800 * 1024;
constexpr float MEMORY_WARNING_RATIO = 0.3;
constexpr float MEMORY_ERROR_RATIO = 0.5;
constexpr uint32_t MEMORY_WARNING_RATIO_GRADE = 50 * 1024;
constexpr uint32_t MEMORY_ERROR_RATIO_GRADE = 100 * 1024;

constexpr int EVENT_ID_MEMORY_LEAK = 901003023;
constexpr int32_t MEMCHECK_PID_INVALID = 0xFFDEAE00;
constexpr uint32_t MEMCHECK_MAGIC = 0x5377FEFA;
constexpr uint32_t MEMCHECKIO = 0xAE;
constexpr uint32_t LOGGER_MEMCHECK_RSS_READ = _IO(MEMCHECKIO, 1);
constexpr uint32_t LOGGER_MEMCHECK_COMMAND = _IO(MEMCHECKIO, 2);
constexpr uint32_t LOGGER_MEMCHECK_STACK_READ = _IO(MEMCHECKIO, 3);
constexpr uint32_t LOGGER_MEMCHECK_DETAIL_READ = _IO(MEMCHECKIO, 4);
const std::string BBOX_PATH = "/dev/bbox";
const std::string RENDER_SERVICE_PROCESS = "render_service";

enum MemCmd {
    MEMCMD_NONE,
    MEMCMD_ENABLE,
    MEMCMD_DISABLE,
    MEMCMD_SAVE_LOG,
    MEMCMD_CLEAR_LOG,
    MEMCMD_MAX
};

class NativeLeakUtil {
public:
    static void RemoveInvalidFile(std::shared_ptr<FaultInfoBase> monitorInfo);
    static uint64_t GetRSSMemoryThreshold(uint64_t threshold);
};

} // namespace HiviewDFX
} // namespace OHOS
#endif // NATIVE_LEAK_UTIL_H
