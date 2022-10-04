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
    double curDelPct = 0;
    int64_t saveHour = config.second;
    while (saveHour >= 0) {
        if (int result = CleanDbByHour(type, saveHour); result != 0) {
            HIVIEW_LOGE("failed to clean db, saveHour=%{public}d, err=%{public}d", saveHour, result);
            return;
        }
        if (curNum = SysEventDao::GetNum(type); curNum < config.first) {
            HIVIEW_LOGI("succ to clean db, curNum=%{public}d", curNum);
            return;
        }
        curDelPct += delPct;
        saveHour = config.second - config.second * curDelPct;
    }
    HIVIEW_LOGI("end to clean db, curNum=%{public}d", curNum);
}

int SysEventDbCleaner::CleanDbByTime(StoreType type, int64_t time)
{
    auto sysEventQuery = SysEventDao::BuildQuery(type);
    (*sysEventQuery).Where(EventCol::TS, EventStore::Op::LT, time);
    return SysEventDao::Delete(sysEventQuery, 10000); // 10000 means delete limit
}

int SysEventDbCleaner::CleanDbByHour(StoreType type, int64_t hour)
{
    int64_t now = static_cast<int64_t>(TimeUtil::GetMilliseconds());
    return CleanDbByTime(type, now - hour * 3600000); // 3600000 means ms per hour
}
} // namespace HiviewDFX
} // namespace OHOS