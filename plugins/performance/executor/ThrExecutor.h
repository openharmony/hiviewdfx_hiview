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
#ifndef THR_EXECUTOR_H
#define THR_EXECUTOR_H

#include <map>
#include "ThrTaskContainer.h"
#include "ITimeoutExecutor.h"
#include "IAppThrExecutor.h"
#include "IMonitorThrExecutor.h"

namespace OHOS {
namespace HiviewDFX {
using IProcessAppEvtTask = IAppThrExecutor::IProcessAppEvtTask;
using AppEvtData = IAppThrExecutor::AppEvtData;


class ThrExecutor : public IAppThrExecutor, public IMonitorThrExecutor, public ITimeoutExecutor {
public:
    ThrExecutor();
    ~ThrExecutor();

    void ExecuteTimeoutInMainThr(ITimeoutHandler* task, std::string name) override;
    void ExecuteHandleEvtInMainThr(IProcessAppEvtTask* task, const AppEvtData& data) override;
    void ExecuteMonitorInMainThr(IHandleMonitorEvt* task, std::shared_ptr <XperfEvt> evt) override;

protected:
    enum ThrType {
        MAIN_THR = 0,
    };

    std::map<int, ThrTaskContainer*> containers;

    static void ValidateNonNull(void* task);
};
} // HiviewDFX
} // OHOS
#endif