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

#ifndef HIVIEW_BASE_EVENT_STORE_INCLUDE_SYS_EVENT_DAO_H
#define HIVIEW_BASE_EVENT_STORE_INCLUDE_SYS_EVENT_DAO_H

#include <memory>
#include <string>
#include <vector>

#include "sys_event.h"
#include "sys_event_query.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
enum class StoreType {
    FAULT = 1,
    STATISTIC = 2,
    SECURITY = 3,
    BEHAVIOR = 4,
};

class SysEventDao {
public:
    static std::shared_ptr<SysEventQuery> BuildQuery(StoreType type);
    static std::shared_ptr<SysEventQuery> BuildQuery(uint16_t eventType);
    static std::shared_ptr<SysEventQuery> BuildQuery(const std::string& dbFile);
    static int Insert(std::shared_ptr<SysEvent> sysEvent);
    static int Delete(std::shared_ptr<SysEventQuery> sysEventQuery, int limit = 100);
    static int Update(std::shared_ptr<SysEvent> sysEvent, bool isNotifyChange = true);
    static int BackupDB(StoreType type);
    static int CloseDB(StoreType type);
    static int DeleteDB(StoreType type);
    static int GetNum(StoreType type);
    static std::string GetDataDir();
    static std::string GetDataFile(StoreType type);
    static void GetDataFiles(std::vector<std::string>& dbFiles);
    static std::string GetBakFile(StoreType type);
}; // SysEventDao
} // EventStore
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_BASE_EVENT_STORE_INCLUDE_SYS_EVENT_DAO_H