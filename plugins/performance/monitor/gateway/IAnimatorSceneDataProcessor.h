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
#ifndef I_ANIMATOR_SCENE_DATA_PROCESSOR_H
#define I_ANIMATOR_SCENE_DATA_PROCESSOR_H

#include <string>
#include "XperfEvt.h"

namespace OHOS {
namespace HiviewDFX {
const int32_t ANIMATOR_MAX_POINTS = 2;

class IAnimatorSceneDataProcessor {
public:
    struct AnimatorPoint {
        BasicInfo basicInfo;
        CommonInfo commonInfo;
    };

    struct AnimatorRecord {
        int64_t startTime {0};
        int32_t allReceviedPoint {ANIMATOR_MAX_POINTS};
        int32_t receviedPoint {0};
        bool hasStartPoint {false};
        AnimatorPoint animatorPoints[ANIMATOR_MAX_POINTS];
    };

    struct AnimatorMetrics {
        BasicInfo basicInfo;
        CommonInfo appInfo;
        CommonInfo rsInfo;
        bool focus;
    };

    class MetricReporter {
    public:
        virtual void ReportMetrics(const AnimatorMetrics& metrics) = 0;
        virtual ~MetricReporter() = default;
    };

    virtual void ProcessSceneData(std::shared_ptr<XperfEvt> evt) = 0;
    virtual void CreateRecord(std::shared_ptr<XperfEvt> evt, int32_t indexPoint) = 0;
    virtual void DeleteRecord(const int32_t uniqueId) = 0;
    virtual ~IAnimatorSceneDataProcessor() = default;
};
} // HiviewDFX
} // OHOS
#endif
