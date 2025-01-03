/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef HIVIEW_PLUGINS_EVENT_SERVICE_INCLUDE_SYS_EVENT_DB_MGR_H
#define HIVIEW_PLUGINS_EVENT_SERVICE_INCLUDE_SYS_EVENT_DB_MGR_H

#include <memory>

#include "event_loop.h"
#include "sys_event.h"

namespace OHOS {
namespace HiviewDFX {
class SysEventDbMgr {
public:
    SysEventDbMgr() = default;
    ~SysEventDbMgr() = default;

    void SaveToStore(std::shared_ptr<SysEvent> event) const;
    void StartCheckStoreTask(std::shared_ptr<EventLoop> looper);
    void CheckStore();
}; // SysEventDbMgr
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_EVENT_SERVICE_INCLUDE_SYS_EVENT_DB_MGR_H