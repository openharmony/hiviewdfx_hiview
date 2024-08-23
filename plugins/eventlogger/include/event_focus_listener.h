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
#ifndef HIVIEW_EVENT_FOCUS_LISTENER_H
#define HIVIEW_EVENT_FOCUS_LISTENER_H
#ifdef WINDOW_MANAGER_ENABLE

#include <mutex>

#include "focus_change_info.h"
#include "window_manager.h"

namespace OHOS {
namespace HiviewDFX {
class EventFocusListener : public Rosen::IFocusChangedListener {
public:
    static bool isRegistered_;
    static uint64_t lastChangedTime_;

    explicit EventFocusListener(){};
    ~EventFocusListener() = default;
    static sptr<EventFocusListener> GetInstance();
    static void RegisterFocusListener();
    static void UnRegisterFocusListener();

    void OnFocused(const sptr<Rosen::FocusChangeInfo>& focusChangeInfo) override;
    void OnUnfocused(const sptr<Rosen::FocusChangeInfo>& focusChangeInfo) override;

private:
    static sptr<EventFocusListener> instance_;
    static std::recursive_mutex mutex_;
    static bool isRegistering_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // WINDOW_MANAGER_ENABLE
#endif // HIVIEW_EVENT_FOCUS_LISTENER_H