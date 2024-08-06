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
#ifndef BASE_CONTEXT_H
#define BASE_CONTEXT_H

#include <map>
#include <vector>
#include "IXperfContext.h"
#include "IMonitor.h"

namespace OHOS {
namespace HiviewDFX {
class BaseContext : public IXperfContext {
public:
    BaseContext();
    virtual ~BaseContext() = default;

    typedef std::map<int, std::vector<IMonitor*>> MonitorMap;
    IEventObservable* GetEventObservable() override;
    void RegisterMonitorByLogID(int logId, IMonitor *newMonitor) override;
    std::vector<IMonitor*> GetMonitorsByLogID(int logId) override;

protected:
    IEventObservable* eventObservable{nullptr};

private:
    MonitorMap monitors;
};
} // HiviewDFX
} // OHOS
#endif // BASE_CONTEXT_H