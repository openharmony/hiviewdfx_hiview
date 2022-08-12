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

#include "sys_event_db_cleaner.h"

#include <cmath>

#include "file_util.h"
#include "logger.h"
#include "sys_event_dao.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-SysEventDbCleaner");
using EventStore::SysEventDao;
using EventStore::SysEventQuery;
using EventStore::StoreType;
namespace {
#ifdef CUSTOM_MAX_FILE_SIZE_MB
const int MAX_FILE_SIZE = 1024 * 1024 * CUSTOM_MAX_FILE_SIZE_MB;
#else
const int MAX_FILE_SIZE = 1024 * 1024 * 100; // 100M
#endif // CUSTOM_MAX_FILE_SIZE

constexpr std::pair<EventStore::StoreType, int> DB_CLEAN_CONFIGS[] = {
    { StoreType::BEHAVIOR, 2 * 24 }, { StoreType::FAULT, 30 * 24 },
    { StoreType::STATISTIC, 30 * 24 }, { StoreType::SECURITY, 90 * 24 },
};
}

bool SysEventDbCleaner::IfNeedClean()
{
    return FileUtil::GetFolderSize(SysEventDao::GetDataDir()) >= MAX_FILE_SIZE;
}

int SysEventDbCleaner::CleanDbByTime(const std::string dbFile, int64_t time) const
{
    auto sysEventQuery = SysEventDao::BuildQuery(dbFile);
    (*sysEventQuery).Where(EventStore::EventCol::TS, EventStore::Op::LT, time);
    return SysEventDao::Delete(sysEventQuery, 10000); // 10000 means delete limit
}

int SysEventDbCleaner::CleanDbByHour(const std::string dbFile, int hour) const
{
    int64_t delTime = TimeUtil::GetMilliseconds() - hour * TimeUtil::SECONDS_PER_HOUR * TimeUtil::SEC_TO_MILLISEC;
    return CleanDbByTime(dbFile, delTime);
}

bool SysEventDbCleaner::CleanDb(EventStore::StoreType type, int saveHours) const
{
    std::string dbFile = SysEventDao::GetDataFile(type);
    if (dbFile.empty()) {
        HIVIEW_LOGE("failed to get db file");
        return false;
    }
    return CleanDbByHour(dbFile, saveHours / std::pow(2, cleanTimes_)) == 0; // 2 means the power of cleaning period
}

bool SysEventDbCleaner::CleanDbs() const
{
    bool res = true;
    for (auto config : DB_CLEAN_CONFIGS) {
        if (!CleanDb(config.first, config.second)) {
            HIVIEW_LOGE("failed to clean db, type=%{public}d", config.first);
            res = false;
        }
    }
    return res;
}

bool SysEventDbCleaner::Clean()
{
    HIVIEW_LOGI("start to delete db files");
    for (auto config : DB_CLEAN_CONFIGS) {
        if (SysEventDao::DeleteDB(config.first) != 0) {
            HIVIEW_LOGE("failed to delete db file, type=%{public}d", config.first);
            return false;
        }
        if (!IfNeedClean()) {
            break;
        }
    }
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS