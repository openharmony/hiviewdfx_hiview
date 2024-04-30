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

#ifndef I_JANK_ANIMATOR_REPORTER_INFRASTRUCTURE_H
#define I_JANK_ANIMATOR_REPORTER_INFRASTRUCTURE_H

#include <string>

struct JankAnimatorReportData {
    int32_t appPid{0};
    int32_t versionCode{0};
    std::string versionName{""};
    std::string bundleName{""};
    std::string processName{""};
    std::string abilityName{""};
    std::string pageUrl{""};
    std::string sceneId{""};
    std::string bundleNameEx{""};
    bool isFocus;
    uint64_t startTime{0};
    uint64_t durition{0};
    int32_t totalAppFrames{0};
    int32_t totalAppMissedFrames{0};
    uint64_t maxAppFrameTime{0};
    int32_t maxAppSeqMissedFrames{0};
    bool isDisplayAnimator{false};
    int32_t totalRenderFrames{0};
    int32_t totalRenderMissedFrames{0};
    uint64_t maxRenderFrameTime{0};
    float averageRenderFrameTime{0};
    int32_t maxRenderSeqMissedFrames{0};
    bool isFoldDisp{false};
    /* only for critical */
    std::string traceFileName{""};
    std::string infoFileName{""};
    uint64_t happenTime{0};
};

class IJankAnimatorReportInfrastructure {
public:
    virtual void ReportNormal(const JankAnimatorReportData& data) = 0;
    virtual void ReportCritical(const JankAnimatorReportData& data) = 0;
};

#endif // I_JANK_ANIMATOR_REPORTER_INFRASTRUCTURE_H