/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "sys_event_db_mgr.h"

#include <cinttypes>
#include <ctime>
#include <string>

#include "file_util.h"
#include "logger.h"
#include "sys_event_dao.h"
#include "sys_event_db_backup.h"
#include "sys_event_db_cleaner.h"
#include "sys_event.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
using EventStore::StoreType;
using EventStore::SysEventDao;
DEFINE_LOG_TAG("HiView-SysEventDbMgr");
namespace {
constexpr StoreType STORE_TYPES[] = {
    StoreType::BEHAVIOR, StoreType::FAULT, StoreType::STATISTIC, StoreType::SECURITY
};
}

void SysEventDbMgr::SaveToStore(std::shared_ptr<SysEvent> event) const
{
    SysEventDao::Insert(event);
    HIVIEW_LOGD("save sys event %{public}" PRId64 ", %{public}s", event->GetSeq(), event->eventName_.c_str());
}

void SysEventDbMgr::StartCheckStoreTask(std::shared_ptr<EventLoop> looper)
{
    if (looper == nullptr) {
        HIVIEW_LOGE("can not init check store task as looper null");
        return;
    }
    HIVIEW_LOGI("init check store task");
    auto statusTask = std::bind(&SysEventDbMgr::CheckStore, this);
    int delay = TimeUtil::SECONDS_PER_HOUR; // 1 hour
    looper->AddTimerEvent(nullptr, nullptr, statusTask, delay, true);
}

void SysEventDbMgr::CheckStore()
{
    HIVIEW_LOGI("start to check store");
    SysEventDbCleaner::TryToClean();
}

void SysEventDbMgr::TryToRecover()
{
    for (auto type : STORE_TYPES) {
        SysEventDbBackup dbBackup(type);
        if (dbBackup.IsBroken() && !dbBackup.Recover()) {
            HIVIEW_LOGE("failed to recover db, type=%{public}d", type);
        }
    }
}
} // namespace HiviewDFX
} // namespace OHOS