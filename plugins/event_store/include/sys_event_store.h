/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef HIVIEW_PLUGINS_EVENT_SERVICE_INCLUDE_SYS_EVENT_SERVICE_H
#define HIVIEW_PLUGINS_EVENT_SERVICE_INCLUDE_SYS_EVENT_SERVICE_H
#include <memory>

#include "event.h"
#include "plugin.h"
#include "sys_event_db_mgr.h"

namespace OHOS {
namespace HiviewDFX {
class SysEventStore : public Plugin {
public:
    SysEventStore();
    ~SysEventStore();
    void OnLoad() override;
    void OnUnload() override;
    bool OnEvent(std::shared_ptr<Event>& event) override;

private:
    std::shared_ptr<SysEvent> Convert2SysEvent(std::shared_ptr<Event>& event);
    bool IsNeedBackup(const std::string& dateStr);

private:
    std::unique_ptr<SysEventDbMgr> sysEventDbMgr_ = nullptr;
    std::atomic<bool> hasLoaded_ { false };
    std::string lastBackupTime_;
}; // SysEventService
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_EVENT_SERVICE_INCLUDE_SYS_EVENT_SERVICE_H
