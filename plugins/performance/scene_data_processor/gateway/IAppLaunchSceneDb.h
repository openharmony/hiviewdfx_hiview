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
#ifndef I_APP_LAUNCH_SCENE_DB_H
#define I_APP_LAUNCH_SCENE_DB_H

#include <string>

class IAppLaunchSceneDb {
public:
    const static int INVALID = -1;

    struct InteractionResponse {
        uint64_t time{0};
        std::string domain{""};
        std::string eventName{""};

        int32_t appPid{0};
        int32_t versionCode{0};
        std::string versionName{""};
        std::string bundleName{""};
        std::string processName{""};
        std::string abilityName{""};
        std::string pageUrl{""};
        std::string sceneId{""};
        std::string sourceType{""};
        uint64_t inputTime{0};
        uint64_t animationStartTime{0};
        uint64_t renderTime{0};
        uint64_t responseLatency{0};
        std::string note{""};
    };

    struct StartAbility {
        uint64_t time{0};
        std::string domain{""};
        std::string eventName{""};

        std::string bundleName{""};
        std::string moduleName{""};
        std::string abilityName{""};
    };

    struct AppStartupType {
        uint64_t time{0};
        std::string domain{""};
        std::string eventName{""};

        int32_t appPid{0};
        int32_t versionCode{0};
        std::string versionName{""};
        std::string bundleName{""};
        std::string abilityName{""};
        int32_t startType{0}; //0-cool,1-hot
    };

    struct ProcessStart {
        uint64_t time{0};
        std::string domain{""};
        std::string eventName{""};

        std::string bundleName{""};
        uint64_t startupTime{0};
        int32_t startupAbilityType{0};
        int32_t startupExtensionType{0};
        std::string callerBundleName{""};
        int32_t callerUid{0};
        std::string callerProcessName{""};
    };

    struct AppAttach {
        uint64_t time{0};
        std::string domain{""};
        std::string eventName{""};

        std::string bundleName{""};
        int32_t appPid{0};
        int32_t versionCode{0};
        std::string versionName{""};
        std::string processName{""};
    };

    struct AppForeground {
        uint64_t time{0};
        std::string domain{""};
        std::string eventName{""};

        std::string bundleName{""};
        int32_t appPid{0};
        int32_t versionCode{0};
        std::string versionName{""};
        std::string processName{""};
        int32_t bundleType{0};
        std::string callerBundleName{""};
    };

    struct AbilityForeground {
        uint64_t time{0};
        std::string domain{""};
        std::string eventName{""};

        std::string bundleName{""};
        std::string moduleName{""};
        std::string abilityName{""};
        int32_t bundleType{0};
        std::string callerBundleName{""};
    };

    struct StartWindow {
        uint64_t time{0};
        std::string domain{""};
        std::string eventName{""};

        std::string bundleName{""};
        int32_t appPid{0};
        std::string processName{""};
        std::string windowName{""};
    };

    struct DrawnCompleted {
        uint64_t time{0};
        std::string domain{""};
        std::string eventName{""};

        int32_t appUid{0};
        int32_t appPid{0};
        std::string bundleName{""};
        std::string moduleName{""};
        std::string abilityName{""};
    };

    struct FirstFrameDrawn {
        uint64_t time{0};
        std::string domain{""};
        std::string eventName{""};

        int32_t appPid{0};
    };

    struct InteractionCompleted {
        uint64_t time{0};
        std::string domain{""};
        std::string eventName{""};

        std::string bundleName{""};
        int32_t appPid{0};
        int32_t versionCode{0};
        std::string versionName{""};
        std::string abilityName{""};
        std::string processName{""};
        std::string pageUrl{""};
        std::string sceneId{""};
        std::string sourceType{""};
        uint64_t inputTime{0};
        uint64_t animationStartLatency{0};
        uint64_t animationEndLatency{0};
        uint64_t e2eLatency{0};
        std::string note{""};
    };

    struct AppStartRecord {
        int32_t appPid{0};
        std::string bundleName{""};

        InteractionResponse interactionResponse;
        StartAbility startAbility;
        AppStartupType appStartupType;
        ProcessStart processStart;
        AppAttach appAttach;
        AppForeground appForeground;
        AbilityForeground abilityForeground;
        StartWindow startWindow;
        DrawnCompleted drawnCompleted;
        FirstFrameDrawn firstFrameDrawn;
        InteractionCompleted interactionCompleted;
    };
public:
    virtual void CreateRecord(const std::string& bundleName, const AppStartRecord& record) = 0;
    virtual void UpdateRecord(const std::string& bundleName, const AppStartRecord& record) = 0;
    virtual void DeleteRecord(const std::string& bundleName) = 0;
    virtual AppStartRecord QueryRecord(const std::string& bundleName) = 0;
    virtual void CreateRecord(const AppStartRecord& record) = 0;
    virtual void UpdateRecord(const AppStartRecord& record) = 0;
    virtual ~IAppLaunchSceneDb() = default;
};
#endif