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

#include "BaseContext.h"

BaseContext::BaseContext() : monitors()
{
}

void BaseContext::RegisterMonitorByLogID(int logId, IMonitor* newMonitor)
{
    if (monitors.find(logId) == monitors.end()) {
        std::vector<IMonitor*> monitorVector;
        monitorVector.push_back(newMonitor);
        monitors.emplace(logId, monitorVector);
    } else {
        std::vector<IMonitor*> &monitorVec = monitors.at(logId);
        monitorVec.push_back(newMonitor);
    }
}

std::vector<IMonitor*> BaseContext::GetMonitorsByLogID(int logId)
{
    if (monitors.find(logId) == monitors.end()) {
        throw std::invalid_argument("logId: " + std::to_string(logId) + " not register.");
    }
    return monitors.at(logId);
}

IEventObservable* BaseContext::GetEventObservable()
{
    return eventObservable;
}