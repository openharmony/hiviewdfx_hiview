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
#ifndef I_MONITOR_H
#define I_MONITOR_H

#include "XperfEvt.h"

class IMonitor {
public:
    virtual ~IMonitor() = default;

    virtual void ListenEvents() = 0;
    virtual void HandleEvt(std::shared_ptr <XperfEvt> evt) = 0;
};

#endif