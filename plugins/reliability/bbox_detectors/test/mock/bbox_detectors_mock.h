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

#ifndef MOCK_BBOX_DETECTORS_MOCK_H_
#define MOCK_BBOX_DETECTORS_MOCK_H_
#include <gmock/gmock.h>

#include "plugin.h"

namespace OHOS {
namespace HiviewDFX {
class MockHiviewContext : public HiviewContext {
public:
    MOCK_METHOD0(GetSharedWorkLoop, std::shared_ptr<EventLoop>());
};

class MockEventLoop : public EventLoop {
public:
    MockEventLoop() : EventLoop("testEventLoop") {};

    uint64_t AddTimerEvent(std::shared_ptr<EventHandler> handler, std::shared_ptr<Event> event, const Task &task,
                           uint64_t interval, bool repeat) override;
    MOCK_METHOD0(GetMockInterval, uint64_t());
};

}
}

#endif // MOCK_BBOX_DETECTORS_MOCK_H_
