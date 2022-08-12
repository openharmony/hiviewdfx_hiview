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

#include "sys_event_db_backup.h"

#include <cinttypes>

#include "file_util.h"
#include "logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-SysEventDbBak");
using EventStore::SysEventDao;
using EventStore::SysEventQuery;
using EventStore::EventCol;
using EventStore::Op;
using EventStore::ResultSet;

SysEventDbBackup::SysEventDbBackup(EventStore::StoreType type)
    : type_(type)
{
    dbFile_ = SysEventDao::GetDataFile(type);
    dbBakFile_ = SysEventDao::GetBakFile(type);
}

bool SysEventDbBackup::IsBroken()
{
    auto sysEventQuery = SysEventDao::BuildQuery(dbFile_);
    ResultSet result = (*sysEventQuery).Where(EventCol::TS, Op::GT, 0).And(EventCol::TS, Op::LT, 1000).Execute(1); // 1s
    if (result.GetErrCode() != 0) {
        HIVIEW_LOGE("sys event db=%{public}s is broken", dbFile_.c_str());
        return true;
    }
    return false;
}

bool SysEventDbBackup::BackupOnline()
{
    if (SysEventDao::BackupDB(type_) < 0) {
        HIVIEW_LOGE("sys event backup db failed");
        return false;
    }
    HIVIEW_LOGI("sys event backup db success");
    return true;
}

bool SysEventDbBackup::Recover()
{
    HIVIEW_LOGW("start to recover db file=%{public}s", dbFile_.c_str());
    return RecoverFromBackup() ? true : RecoverByRebuild();
}

bool SysEventDbBackup::RecoverFromBackup()
{
    if (!FileUtil::FileExists(dbBakFile_)) {
        HIVIEW_LOGE("the backup file does not exists");
        return false;
    }
    if (SysEventDao::DeleteDB(type_) < 0) {
        HIVIEW_LOGE("can not delete db data");
        return false;
    }
    RemoveDbFile();
    if (!FileUtil::RenameFile(dbBakFile_, dbFile_)) {
        HIVIEW_LOGE("failed to rename the backup file");
        return false;
    }
    return true;
}

bool SysEventDbBackup::RecoverByRebuild()
{
    HIVIEW_LOGW("recover sys event db by rebuild");
    if (SysEventDao::CloseDB(type_) < 0) {
        return false;
    }
    RemoveDbFile();
    return true;
}

void SysEventDbBackup::RemoveDbFile()
{
    if (FileUtil::FileExists(dbFile_) && !FileUtil::RemoveFile(dbFile_)) {
        HIVIEW_LOGW("failed to remove sys event wal db");
    }

    std::string walDbFile = dbFile_ + "-wal";
    if (FileUtil::FileExists(walDbFile) && !FileUtil::RemoveFile(walDbFile)) {
        HIVIEW_LOGW("failed to remove sys event wal db");
    }
}
} // namespace HiviewDFX
} // namespace OHOS