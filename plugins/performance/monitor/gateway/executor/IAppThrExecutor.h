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
#ifndef I_APP_THR_EXECUTOR_H
#define I_APP_THR_EXECUTOR_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
class IAppThrExecutor {
public:

    struct AppEvtData {
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
        uint64_t exitTime{0};
        int32_t exitResult{0};
        int32_t exitPid{0};
        std::string note{""};
    };

    class IProcessAppEvtTask {
    public:
        virtual void ExecuteProcessAppEvtTaskInMainThr(const AppEvtData& data) = 0;

        virtual ~IProcessAppEvtTask() = default;
    };

    virtual void ExecuteHandleEvtInMainThr(IProcessAppEvtTask* task, const AppEvtData& data) = 0;

    virtual ~IAppThrExecutor() = default;
};
} // HiviewDFX
} // OHOS
#endif