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
#ifndef XPERF_EVT_H
#define XPERF_EVT_H

#include <string>
#include <map>

using JlogArgMap = std::map<std::string, std::string>;

// Animator info struct
struct BasicInfo {
    int32_t uniqueId{0};
    int32_t versionCode{0};
    std::string versionName{""};
    std::string sceneId{""};
    std::string moduleName{""};
    std::string processName{""};
    std::string abilityName{""};
    std::string pageUrl{""};
    std::string sourceType{""};
    std::string bundleNameEx{""};
};

struct CommonInfo {
    int32_t appPid{0};
    uint64_t startTime{0};
    uint64_t durition{0};
    int32_t totalFrames{0};
    int32_t totalMissedFrames{0};
    uint64_t maxFrameTime{0};
    int32_t maxSeqMissedFrames{0};
    float averageFrameTime{0};
    bool isFoldDisp{false};
    bool isDisplayAnimator{false};
    uint64_t happenTime{0};
};

struct AnimatorInfo {
    BasicInfo basicInfo;
    CommonInfo commonInfo;
};

struct XperfEvt {
    int32_t actionId{0};
    time_t tvSec{0};
    int tvNsec{0};
    int32_t pid{0};
    int32_t tid{0};
    int32_t uid{0};
    int32_t appPid{0};
    unsigned int logId{0};
    int64_t uptime{0};
    int64_t rltime{0};
    int datalen{0};
    char* message{nullptr};
    char* arg1{nullptr};
    int arg2{0};
    char* arg3{nullptr};
    JlogArgMap argMap;
    std::string domain{""};
    std::string eventName{""};
    std::string eventType{""};
    std::string appName{""};
    std::string pName{""};
    std::string uiName{""};
    std::string appVersion{""};
    std::string bundleName{""};
    // activitySwitch resp
    std::string sourceType{""};
    int32_t versionCode{0};
    uint64_t eventTime{0};
    std::string pageUrl{""};
    std::string sceneId{""};
    uint64_t inputTime{0};
    uint64_t animStartTime{0};
    uint64_t renderTime{0};
    uint64_t responseLatency{0};
    uint64_t e2eLatency{0};
    uint64_t animationLatency{0};
    // activitySwitch end
    // for boot
    std::string reason{""};
    std::string isFirst{""};
    std::string detailedTime{""};
    uint64_t totalTime{0};
    uint64_t bootLoaderLatency{0};
    // for power on
    uint32_t state{0};
    std::string wakeUpType{""};
    uint64_t powerStateLatency{0};
    // for app startup
    std::string abilityName{""};
    std::string processName{""};
    uint64_t animationStartTime{0};
    std::string moduleName{""};
    std::string versionName{""};
    int32_t startType{0};
    uint64_t startupTime{0};
    int32_t startupAbilityType{0};
    int32_t startupExtensionType{0};
    std::string callerBundleName{""};
    int32_t callerUid{0};
    std::string callerProcessName{""};
    std::string windowName{""};
    int32_t bundleType{0};
    uint64_t animationStartLatency{0};
    uint64_t animationEndLatency{0};
    uint64_t time{0};
    int32_t appUid{0};
    // Animator info
    AnimatorInfo animatorInfo;
    // for app exit
    uint64_t exitTime{0};
    int32_t exitResult{0};
    int32_t exitPid{0};
    std::string note{""};
    // for screen switch
    uint64_t screenId{0};
    std::string scenePanelName{""};
    std::string rotationStartTime{""};
    uint64_t rotationDuration{0};
    // for screen lock
    std::string packageName{""};
    bool isVerifySuccess{false};
    uint64_t verifyCostTime{0};
    // for PC quick wake
    uint64_t kernelStartTime{0};
    uint64_t kernelEndTime{0};
    int32_t wakeState{0};
    std::string beginTime{""};
    std::string endTime{""};
    // for skip frame time
    uint64_t skippedFrameTime{0};
    // for dh jank frame
    uint64_t dhHappenTimeJank{0};
    uint64_t dhTotalTimeSpentJank{0};
    std::string dhPackageNameJank{""};
    std::string dhVersionJank{""};
    // DH_COMPOSE_JANK_FRAME_INNER
    uint64_t dhStartTimeSeqJank{0};
    uint64_t dhEndTimeSeqJank{0};
    uint64_t skipFrameSeqJank{0};
    uint64_t vsyncIntervalSeqJank{0};
    // for dh app start
    int32_t missionId{0};
    int32_t taskId{0};
    uint64_t timestamp{0};
    uint64_t happenTime{0};
    // for app associated start
    std::string calleeProcessName{""};
    std::string calleeBundleName{""};
    // for fold expand
    int32_t powerOnScreen{0};
    int32_t powerOffScreen{0};
    // for perf factory
    std::string testTitle{""};
    // for limit frequency
    int32_t clientId{0};
    int32_t resId{0};
    int64_t config{0};
    bool onOffTag{false};
    // for shader malfunction
    int32_t malFunctionPid{0};
    std::string malFunctionProcessName{""};
    std::string malFunctionHashCode{""};
    int32_t malFunctionCompileTime{0};
    int64_t malFunctionTimeStamp{0};
    int32_t malFunctionPartFlag{0};
    // for shader stats
    int32_t statsPid{0};
    std::string statsProcessName{""};
    int32_t statsNonSkiaTotal{0};
    int64_t statsTimeStamp{0};
    int32_t statsPartFlag{0};
    // for webview page load
    int64_t navigationId{0};
    int64_t navigationStart{0};
    uint32_t redirectCount{0};
    int64_t redirectStart{0};
    int64_t redirectEnd{0};
    int64_t fetchStart{0};
    int64_t workerStart{0};
    int64_t domainLookupStart{0};
    int64_t domainLookupEnd{0};
    int64_t connectStart{0};
    int64_t secureConnectStart{0};
    int64_t connectEnd{0};
    int64_t requestStart{0};
    int64_t responseStart{0};
    int64_t responseEnd{0};
    int64_t domInteractive{0};
    int64_t domContentLoadedEventStart{0};
    int64_t domContentLoadedEventEnd{0};
    int64_t loadEventStart{0};
    int64_t loadEventEnd{0};
    int64_t firstPaint{0};
    int64_t firstContentfulPaint{0};
    int64_t largestContentfulPaint{0};
    // for webview dynamic frame drop
    uint16_t startTime{0};
    uint16_t duration{0};
    uint32_t totalAppFrames{0};
    uint64_t totalAppMissedFrames{0};
    uint64_t maxAppFrameTime{0};
    // for webview audio frame drop
    uint32_t audioBlankFrameCount{0};
    // for webview video frame drop
    uint32_t videoFrameDroppedCount{0};
    uint64_t videoFrameDroppedDuration{0};
    // for hitch time ratio
    uint64_t uiStartTime{0};
    uint64_t rsStartTime{0};
    uint64_t hitchTime{0};
    float hitchTimeRatio{0};
    bool isFoldDisp{false};
};
#endif