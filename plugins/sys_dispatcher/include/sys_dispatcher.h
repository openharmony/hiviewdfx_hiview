/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef HIVIEW_TEST_PLUGIN_H
#define HIVIEW_TEST_PLUGIN_H

#include "plugin.h"
#include "sys_event_service_adapter.h"

namespace OHOS {
namespace HiviewDFX {
class SysEventDispatcher : public Plugin, SysEventServiceBase {
public:
    bool OnEvent(std::shared_ptr<Event> &event) override;
    void OnLoad() override;
    void OnUnload() override;
    void DispatchEvent(std::shared_ptr<SysEvent>& sysEvent);
private:
    std::shared_ptr<SysEvent> Convert2SysEvent(std::shared_ptr<Event>& event);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_HICOLLIE_PLUGIN_H
