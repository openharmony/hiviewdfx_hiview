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
#ifndef HIVIEW_BASE_APP_EVENT_PUBLISHER_H
#define HIVIEW_BASE_APP_EVENT_PUBLISHER_H

#include "app_event_handler.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventPublisher : public Plugin {
public:
    virtual ~AppEventPublisher() = default;
    virtual void AddAppEventHandler(std::shared_ptr<AppEventHandler> handler) = 0;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIVIEW_BASE_APP_EVENT_PUBLISHER_H