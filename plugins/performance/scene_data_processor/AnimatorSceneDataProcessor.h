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
#ifndef ANIMATOR_SCENE_DATA_PROCESSOR_H
#define ANIMATOR_SCENE_DATA_PROCESSOR_H

#include <string>
#include <map>
#include <iostream>
#include "XperfEvt.h"
#include "IAnimatorSceneDataProcessor.h"

namespace OHOS {
namespace HiviewDFX {
using AnimatorRecord = IAnimatorSceneDataProcessor::AnimatorRecord;
using AnimatorMetrics = IAnimatorSceneDataProcessor::AnimatorMetrics;

class AnimatorSceneDataProcessor : public IAnimatorSceneDataProcessor {
public:
    void ProcessSceneData(std::shared_ptr<XperfEvt> evt) override;
    void SetCb(IAnimatorSceneDataProcessor::MetricReporter* cb);

private:
    IAnimatorSceneDataProcessor::MetricReporter* cb;
    std::map<int32_t, AnimatorRecord*> db;
    int32_t focusWindowPid{0};
    std::string focusBundleName{""};

    void CreateRecord(std::shared_ptr<XperfEvt> evt, int32_t indexPoint) override;
    void DeleteRecord(const int32_t uniqueId) override;
    bool HasStartPoint(std::shared_ptr<XperfEvt> evt);
    bool AllPointsReceived(std::shared_ptr<XperfEvt> evt);
    void SaveAnimatorPoint(std::shared_ptr<XperfEvt> evt, int32_t indexPoint);
    AnimatorMetrics CalcMetrics(const AnimatorRecord& record);
    void Report(const AnimatorMetrics& metrics);
    AnimatorRecord* GetRecord(const int32_t uniqueId);
    void HandleTimeOutPoints();
    void CacheFocusWindowPid(std::shared_ptr<XperfEvt> evt);
};
} // HiviewDFX
} // OHOS
#endif