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

#include "sys_event_dao.h"

#include "file_util.h"
#include "hiview_global.h"
#include "logger.h"
#include "store_mgr_proxy.h"
#include "sys_event_query_wrapper.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
namespace {
constexpr int ERR_INVALID_DB_FILE = -1;
constexpr int ERR_FAILED_DB_OPERATION = -2;
constexpr int ERR_INVALID_QUERY = -3;
}
DEFINE_LOG_TAG("HiView-SysEventDao");
std::shared_ptr<SysEventQuery> SysEventDao::BuildQuery(StoreType type)
{
    return std::make_shared<SysEventQueryWrapper>(GetDataFile(type));
}

std::shared_ptr<SysEventQuery> SysEventDao::BuildQuery(uint16_t eventType)
{
    return std::make_shared<SysEventQueryWrapper>(GetDataFile(static_cast<StoreType>(eventType)));
}

std::shared_ptr<SysEventQuery> SysEventDao::BuildQuery(const std::string& dbFile)
{
    return std::make_shared<SysEventQueryWrapper>(dbFile);
}

int SysEventDao::Insert(std::shared_ptr<SysEvent> sysEvent)
{
    std::string dbFile = GetDataFile(static_cast<StoreType>(sysEvent->what_));
    if (dbFile.empty()) {
        HIVIEW_LOGE("failed to get db file by eventType=%{public}d", sysEvent->what_);
        return ERR_INVALID_DB_FILE;
    }

    HIVIEW_LOGD("insert db file %{public}s with %{public}s", dbFile.c_str(), sysEvent->eventName_.c_str());
    Entry entry;
    entry.id = 0;
    entry.value = sysEvent->jsonExtraInfo_;
    auto docStore = StoreMgrProxy::GetInstance().GetDocStore(dbFile);
    if (docStore->Put(entry) != 0) {
        HIVIEW_LOGE("insert error for event %{public}s", sysEvent->eventName_.c_str());
        return ERR_FAILED_DB_OPERATION;
    }
    sysEvent->SetSeq(entry.id);
    return 0;
}

int SysEventDao::Update(std::shared_ptr<SysEvent> sysEvent, bool isNotifyChange)
{
    std::string dbFile = GetDataFile(static_cast<StoreType>(sysEvent->what_));
    if (dbFile.empty()) {
        HIVIEW_LOGE("failed to get db file by eventType=%{public}d", sysEvent->what_);
        return ERR_INVALID_DB_FILE;
    }

    HIVIEW_LOGD("update db file %{public}s", dbFile.c_str());
    Entry entry;
    entry.id = sysEvent->GetSeq();
    entry.value = sysEvent->jsonExtraInfo_;
    auto docStore = StoreMgrProxy::GetInstance().GetDocStore(dbFile);
    if (docStore->Merge(entry) != 0) {
        HIVIEW_LOGE("update error for event %{public}s", sysEvent->eventName_.c_str());
        return ERR_FAILED_DB_OPERATION;
    }

    if (isNotifyChange) {
        HiviewGlobal::GetInstance()->PostUnorderedEvent(sysEvent);
    }
    return 0;
}

int SysEventDao::Delete(std::shared_ptr<SysEventQuery> sysEventQuery, int limit)
{
    if (sysEventQuery == nullptr) {
        return ERR_INVALID_QUERY;
    }
    std::vector<std::string> dbFiles;
    if (sysEventQuery->GetDbFile().empty()) {
        GetDataFiles(dbFiles);
    } else {
        dbFiles.push_back(sysEventQuery->GetDbFile());
    }

    DataQuery dataQuery;
    sysEventQuery->GetDataQuery(dataQuery);
    dataQuery.Limit(limit);
    int delNum = 0;
    for (auto dbFile : dbFiles) {
        HIVIEW_LOGD("delete event from db file %{public}s", dbFile.c_str());
        auto docStore = StoreMgrProxy::GetInstance().GetDocStore(dbFile);
        if (delNum = docStore->Delete(dataQuery); delNum < 0) {
            HIVIEW_LOGE("delete event error from db file %{public}s", dbFile.c_str());
            return ERR_FAILED_DB_OPERATION;
        }
    }
    return delNum;
}

int SysEventDao::GetNum(StoreType type)
{
    std::string dbFile = GetDataFile(type);
    if (dbFile.empty()) {
        HIVIEW_LOGE("failed to get db file by eventType=%{public}d", type);
        return ERR_INVALID_DB_FILE;
    }
    int result = 0;
    auto docStore = StoreMgrProxy::GetInstance().GetDocStore(dbFile);
    if (result = docStore->GetNum(); result < 0) {
        HIVIEW_LOGE("failed to get the number of events from db=%{public}s", dbFile.c_str());
        return ERR_FAILED_DB_OPERATION;
    }
    return result;
}

int SysEventDao::BackupDB(StoreType type)
{
    std::string dbFile = GetDataFile(type);
    if (dbFile.empty()) {
        HIVIEW_LOGE("failed to get db file by storeType=%{public}d", type);
        return ERR_INVALID_DB_FILE;
    }
    return StoreMgrProxy::GetInstance().BackupDocStore(dbFile, GetBakFile(type));
}

int SysEventDao::DeleteDB(StoreType type)
{
    std::string dbFile = GetDataFile(type);
    if (dbFile.empty()) {
        HIVIEW_LOGE("failed to get db file by storeType=%{public}d", type);
        return ERR_INVALID_DB_FILE;
    }
    return StoreMgrProxy::GetInstance().DeleteDocStore(dbFile);
}

int SysEventDao::CloseDB(StoreType type)
{
    std::string dbFile = GetDataFile(type);
    if (dbFile.empty()) {
        HIVIEW_LOGE("failed to get db file by storeType=%{public}d", type);
        return ERR_INVALID_DB_FILE;
    }
    return StoreMgrProxy::GetInstance().CloseDocStore(dbFile);
}

std::string SysEventDao::GetDataDir()
{
    std::string workPath = HiviewGlobal::GetInstance()->GetHiViewDirectory(
        HiviewContext::DirectoryType::WORK_DIRECTORY);
    if (workPath.back() != '/') {
        workPath = workPath + "/";
    }
    std::string dbFilePath = workPath + "sys_event_db/";
    if (!FileUtil::FileExists(dbFilePath)) {
        if (FileUtil::ForceCreateDirectory(dbFilePath, FileUtil::FILE_PERM_770)) {
            HIVIEW_LOGE("create sys_event_db path successful");
        } else {
            dbFilePath = workPath;
            HIVIEW_LOGE("create sys_event_db path fail, use default");
        }
    }
    return dbFilePath;
}

void SysEventDao::GetDataFiles(std::vector<std::string>& dbFiles)
{
    dbFiles.push_back(GetDataFile(StoreType::FAULT));
    dbFiles.push_back(GetDataFile(StoreType::STATISTIC));
    dbFiles.push_back(GetDataFile(StoreType::SECURITY));
    dbFiles.push_back(GetDataFile(StoreType::BEHAVIOR));
}

std::string SysEventDao::GetDataFile(StoreType type)
{
    switch (type) {
        case StoreType::FAULT:
            return GetDataDir() + "fault.db";
        case StoreType::STATISTIC:
            return GetDataDir() + "statistic.db";
        case StoreType::SECURITY:
            return GetDataDir() + "security.db";
        case StoreType::BEHAVIOR:
            return GetDataDir() + "behavior.db";
        default:
            break;
    }
    return "";
}

std::string SysEventDao::GetBakFile(StoreType type)
{
    switch (type) {
        case StoreType::FAULT:
            return GetDataDir() + "back_fault.db";
        case StoreType::STATISTIC:
            return GetDataDir() + "back_statistic.db";
        case StoreType::SECURITY:
            return GetDataDir() + "back_security.db";
        case StoreType::BEHAVIOR:
            return GetDataDir() + "back_behavior.db";
        default:
            break;
    }
    return "";
}
} // EventStore
} // namespace HiviewDFX
} // namespace OHOS
