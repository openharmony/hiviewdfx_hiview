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
#ifndef OHOS_HIVIEWDFX_UCOLLECTUTIL_COMMON_UTIL_H
#define OHOS_HIVIEWDFX_UCOLLECTUTIL_COMMON_UTIL_H

#include <cinttypes>
#include <string>

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
const std::string PROC = "/proc/";
const std::string IO = "/io";
const std::string SMAPS_ROLLUP = "/smaps_rollup";
const std::string STATM = "/statm";
const std::string MEM_INFO = "/proc/meminfo";
const std::string GPU_CUR_FREQ = "/sys/class/devfreq/gpufreq/cur_freq";
const std::string GPU_MAX_FREQ = "/sys/class/devfreq/gpufreq/max_freq";
const std::string GPU_MIN_FREQ = "/sys/class/devfreq/gpufreq/min_freq";
const std::string GPU_LOAD = "/sys/class/devfreq/gpufreq/gpu_scene_aware/utilisation";
const std::string MEMINFO_SAVE_DIR = "/data/log/hiview/unified_collection/memory";
const static int VSS_BIT = 4;

class CommonUtil {
private:
    CommonUtil() = default;
    ~CommonUtil() = default;

public:
    template <typename T>
    static bool StrToNum(const std::string& sString, T &tX);
    static bool ParseTypeAndValue(const std::string &str, std::string &type, int32_t &value);
    static bool StartWith(const std::string& str, const std::string& sub);
    static bool EndWith(const std::string& str, const std::string& sub);
}; // MemoryCollector
} // UCollectUtil
} // HiviewDFX
} // OHOS
#endif // OHOS_HIVIEWDFX_UCOLLECTUTIL_COMMON_UTIL_H
