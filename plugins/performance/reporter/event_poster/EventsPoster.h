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
#ifndef EVENTS_POSTER_H
#define EVENTS_POSTER_H

#include "IAppStartEventPoster.h"
#include "IEventObservable.h"
#include "IScrollJankEventPoster.h"

using AppStartInfo = IEventObserver::AppStartInfo;
using ScrollJankInfo = IEventObserver::ScrollJankInfo;

class EventsPoster : public IEventObservable, public IAppStartEventPoster, public IScrollJankEventPoster {
public:
    void RegObserver(IEventObserver* observer) override;
    void PostAppStartEvent(const AppStartEventInfo& evt) override;
    void PostScrollJankEvent(const ScrollJankEventInfo& evt) override;

private:
    IEventObserver* observer{nullptr};

    AppStartInfo ConvertToAppStartInfo(const AppStartEventInfo& event);
    ScrollJankInfo ConvertToScrollJankInfo(const ScrollJankEventInfo& event);
};
#endif