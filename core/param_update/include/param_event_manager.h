/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef PARAM_COMMON_EVENT_H
#define PARAM_COMMON_EVENT_H

#include <functional>
#include <map>
#include <memory>
#include <singleton.h>
#include <string>

#include "common_event_subscriber.h"
#include "want.h"

namespace OHOS {
namespace HiviewDFX {
class ParamEventManager {
    DECLARE_SINGLETON(ParamEventManager)
public:
    void SubscriberEvent();
    void UnSubscriberEvent();
    void OnReceiveEvent(const AAFwk::Want &want);

private:
    class ParamCommonEventSubscriber : public EventFwk::CommonEventSubscriber {
    public:
        explicit ParamCommonEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscriberInfo,
            ParamEventManager &registry)
            : CommonEventSubscriber(subscriberInfo), registry_(registry)
        {}
        ~ParamCommonEventSubscriber() = default;

        void OnReceiveEvent(const EventFwk::CommonEventData &data) override
        {
            registry_.OnReceiveEvent(data.GetWant());
        }
        
    private:
        ParamEventManager &registry_;
    };
    std::shared_ptr<ParamCommonEventSubscriber> subscriber_ = nullptr;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // PARAM_COMMON_EVENT_H
