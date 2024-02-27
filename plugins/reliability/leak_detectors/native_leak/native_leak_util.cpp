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
#include "native_leak_util.h"

#include <memory>
#include <string>
#include <vector>

#include "fault_detector_util.h"
#include "fault_info_base.h"
#include "logger.h"
#include "native_leak_info.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("NativeLeakUtil");
using std::vector;
using std::pair;
using std::shared_ptr;
using std::string;
using std::static_pointer_cast;

namespace {
// calculate rssthreshold base on pssthreshold
vector<pair<uint64_t, float>> g_rssthresholdTimes = {
    {2048000, 1.1},
    {1024000, 1.3},
    {512000, 1.5},
    {307200, 2},
    {102400, 3},
    {51200, 4},
    {10240, 6},
    {0, 10}
};
}

void NativeLeakUtil::RemoveInvalidFile(shared_ptr<FaultInfoBase> monitorInfo)
{
    string path = MEMORY_LEAK_PATH;
    auto userMonitorInfo = static_pointer_cast<NativeLeakInfo>(monitorInfo);
    vector<string> subFiles = FaultDetectorUtil::GetSubFile(path, true);
    for (const auto &each : subFiles) {
        if (each == userMonitorInfo->GetSampleFilePath()) {
            string filePath = path + "/" + each;
            string newPath = path + "/" + userMonitorInfo->GetProcessName() + "_died";
            if (!FaultDetectorUtil::RenameFile(filePath, newPath)) {
                HIVIEW_LOGE("failed to remove file: %{public}s", each.c_str());
            }
        }
    }
}

uint64_t NativeLeakUtil::GetRSSMemoryThreshold(uint64_t threshold)
{
    for (auto rssthresholdTime : g_rssthresholdTimes) {
        if (threshold > rssthresholdTime.first) {
            return threshold * rssthresholdTime.second;
        }
    }
    return threshold;
}
} // namespace HiviewDFX
} // namespace OHOS
