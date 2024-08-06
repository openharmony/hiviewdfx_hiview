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
#ifndef I_APP_START_REPORTER_H
#define I_APP_START_REPORTER_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
struct AppStartReportEvent {
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
    uint64_t animationEndLatency{0};
    uint64_t animationLatency{0};
    uint64_t e2eLatency{0};
    unsigned int actionId{0};
    unsigned int eventId{0};
    std::string traceFileName{""};
    std::string infoFileName{""};
    uint64_t happenTime{0};
};

class IAppStartReporter {
public:
    virtual ~IAppStartReporter() = default;
    virtual void ReportNormal(const AppStartReportEvent& report) = 0;
    virtual void ReportCritical(const AppStartReportEvent& report) = 0;
};
} // HiviewDFX
} // OHOS
#endif