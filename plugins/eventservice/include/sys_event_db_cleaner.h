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

#ifndef HIVIEW_PLUGINS_EVENT_SERVICE_INCLUDE_SYS_EVENT_DB_CLEANER_H
#define HIVIEW_PLUGINS_EVENT_SERVICE_INCLUDE_SYS_EVENT_DB_CLEANER_H

#include <string>

#include "sys_event_dao.h"

namespace OHOS {
namespace HiviewDFX {
class SysEventDbCleaner {
public:
    SysEventDbCleaner() : cleanTimes_(0) {}
    ~SysEventDbCleaner() = default;
    bool Clean();
    static bool IfNeedClean();

private:
    int CleanDbByHour(const std::string dbFile, int hour) const;
    int CleanDbByTime(const std::string dbFile, int64_t time) const;
    bool CleanDbs() const;
    bool CleanDb(EventStore::StoreType type, int saveHours) const;

private:
    int cleanTimes_;
}; // SysEventDbCleaner
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_EVENT_SERVICE_INCLUDE_SYS_EVENT_DB_CLEANER_H