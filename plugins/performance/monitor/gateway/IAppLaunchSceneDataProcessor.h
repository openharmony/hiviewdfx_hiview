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
#ifndef I_APP_LAUNCH_SCENE_DATA_PROCESSOR_H
#define I_APP_LAUNCH_SCENE_DATA_PROCESSOR_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
class IAppLaunchSceneDataProcessor {
public:
    struct AppStartCheckPointData {
        uint64_t time{0};
        std::string domain{""};
        std::string eventName{""};
        std::string bundleName{""};
        std::string abilityName{""};
        std::string processName{""};
        std::string pageUrl{""};
        std::string sceneId{""};
        std::string sourceType{""};
        uint64_t inputTime{0};
        uint64_t animationStartTime{0};
        uint64_t renderTime{0};
        uint64_t responseLatency{0};
        std::string moduleName{""};
        int32_t versionCode{0};
        std::string versionName{""};
        int32_t startType{0};
        uint64_t startupTime{0};
        int32_t startupAbilityType{0};
        int32_t startupExtensionType{0};
        int32_t callerUid{0};
        std::string callerProcessName{""};
        std::string callerBundleName{""};
        int32_t appPid{0};
        int32_t appUid{0};
        std::string windowName{""};
        uint64_t animationStartLatency{0};
        uint64_t animationEndLatency{0};
        uint64_t e2eLatency{0};
        int32_t bundleType{0};
        std::string note{""};
    };

    struct AppStartMetrics {
        int32_t appPid{0};
        int32_t versionCode{0};
        std::string versionName{""};
        std::string processName{""};
        std::string bundleName{""};
        std::string abilityName{""};
        std::string pageUrl{""};
        std::string sceneId{""};
        int32_t startType{0};
        std::string sourceType{""};
        uint64_t inputTime{0};
        uint64_t happenTime{0};
        uint64_t responseLatency{0};
        uint64_t launcherToAmsStartAbilityDur{0};
        uint64_t amsStartAbilityToProcessStartDuration{0};
        uint64_t amsProcessStartToAppAttachDuration{0};
        uint64_t amsAppAttachToAppForegroundDuration{0};
        uint64_t amsStartAbilityToAppForegroundDuration{0};
        uint64_t amsAppFgToAbilityFgDur{0};
        uint64_t amsAbilityFgToWmsStartWinDur{0};
        uint64_t drawnLatency{0};
        uint64_t firstFrameDrawnLatency{0};
        uint64_t animationEndLatency{0}; //form animation_start to animation_end
        uint64_t animationLatency{0}; //from input_time to animation_end
        uint64_t e2eLatency{0};
    };

    class MetricReporter {
    public:
        virtual void ReportMetrics(const AppStartMetrics& metrics) = 0;
        virtual ~MetricReporter() = default;
    };
public:
    virtual void SetMetricReporter(MetricReporter* metricReporter) = 0;
    virtual void ProcessSceneData(const AppStartCheckPointData& data) = 0;
    virtual ~IAppLaunchSceneDataProcessor() = default;
};
} // HiviewDFX
} // OHOS
#endif