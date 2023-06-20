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

#include <cinttypes>
#include <map>

#include "logger.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-SysEventDbCleaner");
using EventStore::EventCol;
using EventStore::SysEventDao;
using EventStore::StoreType;
namespace {
const std::map<StoreType, std::pair<int, int>> CLEAN_CONFIG_MAP = {
    { StoreType::BEHAVIOR, { 300000, 7 * 24 } },    // behavior event, maxNum: 300k, maxTime: 7 days
    { StoreType::FAULT, { 20000, 30 * 24 } },       // fault event, maxNum: 20k, maxTime: 30 days
    { StoreType::STATISTIC, { 200000, 30 * 24 } },  // statistic event, maxNum: 200k, maxTime: 30 days
    { StoreType::SECURITY, { 30000, 90 * 24} },     // security event, maxNum: 30k, maxTime: 90 days
};
const int64_t MILLISECS_PER_HOUR = TimeUtil::SECONDS_PER_HOUR * TimeUtil::SEC_TO_MILLISEC;
const int64_t DELETE_LIMIT = 10000;
}

void SysEventDbCleaner::TryToClean()
{
    for (auto cleanConfig : CLEAN_CONFIG_MAP) {
        TryToCleanDb(cleanConfig.first, cleanConfig.second);
    }
}

void SysEventDbCleaner::TryToCleanDb(StoreType type, const std::pair<int, int>& config)
{
    int curNum = SysEventDao::GetNum(type);
    HIVIEW_LOGI("try to clean db: type=%{public}d, curNum=%{public}d, maxNum=%{public}d, maxTime=%{public}d",
        type, curNum, config.first, config.second);

    // delete events if the num of events exceeds 10%
    const double delPct = 0.1;
    if (curNum <= (config.first + config.first * delPct)) {
        return;
    }

    int64_t delEndTime = static_cast<int64_t>(TimeUtil::GetMilliseconds());
    int saveHour = config.second;
    int64_t delTime = delEndTime > saveHour * MILLISECS_PER_HOUR ?
        (delEndTime - saveHour * MILLISECS_PER_HOUR) : delEndTime;
    bool isDeleteLess = false;
    while (curNum > config.first && delTime <= delEndTime) {
        auto delNum = CleanDbByTime(type, delTime);
        if (delNum < 0) { // db is corrupt
            HIVIEW_LOGW("failed to clean db, err=%{public}d", delNum);
            DeleteDb(type);
            return;
        }
        curNum = SysEventDao::GetNum(type);
        if (curNum < 0) {
            HIVIEW_LOGW("failed to get num from db, err=%{public}d", curNum);
            DeleteDb(type);
            return;
        }
        if (delNum < DELETE_LIMIT) { // change to the next time interval
            saveHour -= isDeleteLess ? 1 : (config.second * delPct); // change from 10% to 1h
            const int delBdyHour = 24;
            if (saveHour < delBdyHour && !isDeleteLess) {
                saveHour = delBdyHour;
                isDeleteLess = true;
            }
            delTime = delEndTime > saveHour * MILLISECS_PER_HOUR ?
                (delEndTime - saveHour * MILLISECS_PER_HOUR) : delEndTime;
        }
        HIVIEW_LOGD("cleaning db: curNum=%{public}d, delTime=%{public}" PRId64 ", delEndTime=%{public}" PRId64,
            curNum, delTime, delEndTime);
    }
    HIVIEW_LOGI("end to clean db, curNum=%{public}d", curNum);
}

int SysEventDbCleaner::CleanDbByTime(StoreType type, int64_t time)
{
    auto sysEventQuery = SysEventDao::BuildQuery(type);
    sysEventQuery->Where(EventCol::TS, EventStore::Op::LE, time);
    return SysEventDao::Delete(sysEventQuery, DELETE_LIMIT);
}

void SysEventDbCleaner::DeleteDb(StoreType type)
{
    if (SysEventDao::DeleteDB(type) != 0) {
        HIVIEW_LOGE("failed to delete db file, type=%{public}d", type);
    }
}
} // namespace HiviewDFX
} // namespace OHOS