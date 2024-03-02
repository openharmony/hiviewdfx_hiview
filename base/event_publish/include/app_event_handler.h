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
#ifndef HIVIEW_BASE_APP_EVENT_HANDLER_H
#define HIVIEW_BASE_APP_EVENT_HANDLER_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
class AppEventHandler {
public:
    struct BundleInfo {
        std::string bundleName;
        std::string bundleVersion;
    };

    struct ProcessInfo {
        std::string processName;
    };

    struct AbilityInfo {
        std::string abilityName;
    };

    struct TimeInfo {
        uint64_t time = 0;
        uint64_t beginTime = 0;
        uint64_t endTime = 0;
    };

    struct MemoryInfo {
        uint64_t pss = 0;
        uint64_t rss = 0;
        uint64_t vss = 0;
        uint64_t avaliableMem = 0;
        uint64_t freeMem = 0;
        uint64_t totalMem = 0;
    };

    struct AppLaunchInfo : public BundleInfo, public ProcessInfo {
        int32_t startType = 0;
        uint64_t iconInputTime = 0;
        uint64_t animationFinishTime = 0;
        uint64_t extendTime = 0;
    };

    struct ScrollJankInfo : public BundleInfo, public ProcessInfo, public AbilityInfo {
        uint64_t beginTime = 0;
        uint64_t duration = 0;
        int32_t totalAppFrames = 0;
        int32_t totalAppMissedFrames = 0;
        uint64_t maxAppFrametime = 0;
        int32_t maxAppSeqFrames = 0;
        int32_t totalRenderFrames = 0;
        int32_t totalRenderMissedFrames = 0;
        uint64_t maxRenderFrametime = 0;
        int32_t maxRenderSeqFrames = 0;
    };

    struct CpuUsageHighInfo : public BundleInfo, public TimeInfo {
        bool isForeground = false;
        uint64_t usage = 0;
    };

    struct UsageStatInfo {
        std::vector<uint64_t> fgUsages = std::vector<uint64_t>(24); // 24 : statistics per hour, fg : foreground
        std::vector<uint64_t> bgUsages = std::vector<uint64_t>(24); // 24 : statistics per hour, bg : background
    };

    struct BatteryUsageInfo : public BundleInfo, public TimeInfo {
        UsageStatInfo usage;
        UsageStatInfo cpuEnergy;
        UsageStatInfo gpuEnergy;
        UsageStatInfo ddrEnergy;
        UsageStatInfo displayEnergy;
        UsageStatInfo audioEnergy;
        UsageStatInfo modemEnergy;
        UsageStatInfo romEnergy;
        UsageStatInfo wifiEnergy;
        UsageStatInfo othersEnergy;
    };

    struct ResourceOverLimitInfo : public BundleInfo, public MemoryInfo {
        int32_t pid = 0;
        int32_t uid = 0;
        std::string resourceType;
    };

    int PostEvent(const AppLaunchInfo& event);
    int PostEvent(const ScrollJankInfo& event);
    int PostEvent(const CpuUsageHighInfo& event);
    int PostEvent(const BatteryUsageInfo& event);
    int PostEvent(const ResourceOverLimitInfo& event);
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIVIEW_BASE_APP_EVENT_HANDLER_H