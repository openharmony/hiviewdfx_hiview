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
#include "sys_event_dao.h"

#include <inttypes.h>

#include "logger.h"
#include "sys_event_database.h"
#include "sys_event_query_wrapper.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
DEFINE_LOG_TAG("HiView-SysEventDao");

std::shared_ptr<SysEventQuery> SysEventDao::BuildQuery(const std::string& domain,
    const std::vector<std::string>& names)
{
    HIVIEW_LOGD("start to query event, domain=%{public}s, names=%{public}zu", domain.c_str(), names.size());
    if (domain.empty() || names.empty()) {
        return nullptr;
    }
    return std::make_shared<SysEventQueryWrapper>(domain, names);
}

std::shared_ptr<SysEventQuery> SysEventDao::BuildQuery(const std::string& domain,
    const std::vector<std::string>& names, uint32_t type, int64_t toSeq)
{
    HIVIEW_LOGD("start to query event, domain=%{public}s, names=%{public}zu, type=%{public}u, seq=%{public}" PRId64,
        domain.c_str(), names.size(), type, toSeq);
    return std::make_shared<SysEventQueryWrapper>(domain, names, type, toSeq);
}

int SysEventDao::Insert(std::shared_ptr<SysEvent> sysEvent)
{
    HIVIEW_LOGD("start to insert sys event, domain=%{public}s, name=%{public}s",
        sysEvent->domain_.c_str(), sysEvent->eventName_.c_str());
    return SysEventDatabase::GetInstance().Insert(sysEvent);
}

int SysEventDao::Delete(std::shared_ptr<SysEventQuery> sysEventQuery, int limit)
{
    // if (sysEventQuery == nullptr) {
    //     return ERR_INVALID_QUERY;
    // }
    // std::vector<std::string> dbFiles;
    // if (sysEventQuery->GetDbFile().empty()) {
    //     GetDataFiles(dbFiles);
    // } else {
    //     dbFiles.push_back(sysEventQuery->GetDbFile());
    // }

    // DataQuery dataQuery;
    // sysEventQuery->GetDataQuery(dataQuery);
    // dataQuery.Limit(limit);
    // int delNum = 0;
    // for (auto dbFile : dbFiles) {
    //     HIVIEW_LOGD("delete event from db file %{public}s", dbFile.c_str());
    //     auto docStore = StoreMgrProxy::GetInstance().GetDocStore(dbFile);
    //     if (delNum = docStore->Delete(dataQuery); delNum < 0) {
    //         HIVIEW_LOGE("delete event error from db file %{public}s", dbFile.c_str());
    //         return ERR_FAILED_DB_OPERATION;
    //     }
    // }
    // return delNum;
    return 0;
}
} // EventStore
} // namespace HiviewDFX
} // namespace OHOS
