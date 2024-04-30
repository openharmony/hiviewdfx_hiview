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
#ifndef XPERF_CONTEXT_H
#define XPERF_CONTEXT_H

#include <map>
#include <vector>
#include "BaseContext.h"
#include "ISceneTimerInfrastructure.h"
#include "EventsPoster.h"
#include "ThrExecutor.h"

class NormalContext : public BaseContext {
public:
    NormalContext() = default;
    void CreateContext() override;

private:
    struct CommonParts {
        ThrExecutor* thr;
        ISceneTimerInfrastructure* timerInfra;
        EventsPoster* eventsPoster;

        CommonParts(ThrExecutor* thr, ISceneTimerInfrastructure* timerInfra, EventsPoster* eventsPoster)
        {
            this->thr = thr;
            this->timerInfra = timerInfra;
            this->eventsPoster = eventsPoster;
        }
    };

    CommonParts MakeCommonParts();
    void InitAppStartMonitor(const CommonParts& common);
    IMonitor* MakeAppStartMonitor(const CommonParts& common);
    void InitJankAnimatorMonitor(const CommonParts& common);
    IMonitor* MakeJankAnimatorMonitor(const CommonParts& common);
};
#endif