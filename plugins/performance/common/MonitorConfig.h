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
#ifndef MONITOR_CONFIG_H
#define MONITOR_CONFIG_H

#include <map>
#include "FaultHandlerConfig.h"

namespace OHOS {
namespace HiviewDFX {
enum ParseType {
    ACTION_TYPE = 0, // Parsed by aciton
    SCENE_TYPE //Parsed by scene id
};

struct MonitorConfig {
    unsigned int actionId{2};
    unsigned int eventId{0};
    std::map<unsigned int, FaultLevelThreshold> faultLevelThresholds;
};

struct SceneConfig {
    unsigned int actionId{0};
    unsigned int dumpBitmap{0};
    std::map<unsigned int, FaultLevelThreshold> faultLevelThresholds;
};
} // HiviewDFX
} // OHOS
#endif