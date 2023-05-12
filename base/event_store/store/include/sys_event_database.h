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

#ifndef HIVIEW_BASE_EVENT_STORE_SYS_EVENT_DATABASE_H
#define HIVIEW_BASE_EVENT_STORE_SYS_EVENT_DATABASE_H

#include <memory>
#include <shared_mutex>
#include <string>

#include "base_def.h"
#include "doc_query.h"
#include "singleton.h"
#include "sys_event_query.h"
#include "sys_event_doc_lru_cache.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
class SysEventDatabase : public OHOS::DelayedRefSingleton<SysEventDatabase> {
public:
    SysEventDatabase();
    ~SysEventDatabase() {}
    int Insert(const std::shared_ptr<SysEvent>& sysEvent);
    int Delete(const std::string& domain, const std::string& name);
    int Query(const SysEventQuery& query, std::vector<Entry>& entries);
    std::string GetDatabaseDir();

private:
    void GetQueryFiles(const SysEventQueryArg& queryArg, std::vector<std::string>& queryFiles);
    void GetQueryDirsByDomain(const std::string& domain, std::vector<std::string>& queryDirs);
    bool IsContainQueryArg(const std::string file, const SysEventQueryArg& queryArg);
    int QueryByFiles(const SysEventQuery& query, std::vector<Entry>& entries,
        const std::vector<std::string>& queryFiles);

    std::unique_ptr<SysEventDocLruCache> lruCache_;
    mutable std::shared_mutex mutex_;
}; // SysEventDatabase
} // EventStore
} // HiviewDFX
} // OHOS
#endif // HIVIEW_BASE_EVENT_STORE_SYS_EVENT_DATABASE_H