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
#include "event_focus_listener.h"

#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
sptr<EventFocusListener> EventFocusListener::instance_ = nullptr;
std::recursive_mutex EventFocusListener::mutex_;

sptr<EventFocusListener> EventFocusListener::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::recursive_mutex> lock_l(mutex_);
        instance_ = new (std::nothrow) EventFocusListener();
    }
    return instance_;
}

void EventFocusListener::OnFocused(const sptr<Rosen::FocusChangeInfo>& focusChangeInfo)
{
    lastChangedTime_ = TimeUtil::GetMilliseconds();
}

void EventFocusListener::OnUnfocused(const sptr<Rosen::FocusChangeInfo>& focusChangeInfo)
{
    lastChangedTime_ = TimeUtil::GetMilliseconds();
}
} // namesapce HiviewDFX
} // namespace OHOSs