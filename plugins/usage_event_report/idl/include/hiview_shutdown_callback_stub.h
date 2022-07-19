/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef HIVIEW_PLUGINS_USAGE_EVENT_REPORT_IDL_HIVIEW_SHUTDOWN_CALLBACK_STUB_H
#define HIVIEW_PLUGINS_USAGE_EVENT_REPORT_IDL_HIVIEW_SHUTDOWN_CALLBACK_STUB_H

#include <iremote_proxy.h>
#include <nocopyable.h>

#include "ishutdown_callback.h"

namespace OHOS {
namespace HiviewDFX {
class HiViewShutdownCallbackStub : public IRemoteStub<PowerMgr::IShutdownCallback> {
public:
    DISALLOW_COPY_AND_MOVE(HiViewShutdownCallbackStub);
    HiViewShutdownCallbackStub() = default;
    virtual ~HiViewShutdownCallbackStub() = default;
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    void ShutdownStub();
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_USAGE_EVENT_REPORT_IDL_HIVIEW_SHUTDOWN_CALLBACK_STUB_H
