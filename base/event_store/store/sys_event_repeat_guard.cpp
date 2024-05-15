/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "sys_event_repeat_guard.h"

#include "calc_fingerprint.h"
#include "file_util.h"
#include "hiview_logger.h"
#include "parameter_ex.h"
#include "string_util.h"
#include "sys_event_repeat_db.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-SysEvent-Repeat-Guard");
namespace {
constexpr time_t TIME_RANGE_COMMERCIAL = 24 * 60 * 60; // 24h
constexpr time_t TIME_RANGE_BETA = 1 * 60 * 60; // 1h

inline bool GetShaStr(uint8_t* eventData, std::string& hashStr)
{
    EventRawDataInfo<EventRaw::HiSysEventHeader> eventInfo(eventData);
    constexpr int buffLen = SHA256_DIGEST_LENGTH * 2 + 1;
    char buff[buffLen] = {0};
    if (CalcFingerprint::CalcBufferSha(
        eventData + eventInfo.dataPos, eventInfo.dataSize - eventInfo.dataPos, buff, buffLen) != 0) {
        HIVIEW_LOGE("fail to calc sha.");
        return false;
    }
    hashStr = buff;
    return true;
}
}

int64_t SysEventRepeatGuard::GetMinValidTime()
{
    static time_t timeRange = Parameter::IsBetaVersion() ? TIME_RANGE_BETA : TIME_RANGE_COMMERCIAL;
    time_t timeNow = time(nullptr);
    time_t timeMin = timeNow > timeRange ? (timeNow - timeRange) : 0;
    return timeMin;
}

 
void SysEventRepeatGuard::Check(std::shared_ptr<SysEvent> event)
{
    if (event->GetEventType() != SysEventCreator::EventType::FAULT) {
        return;
    }
    if (IsEventRepeat(event)) {
        event->SetLog(LOG_NOT_ALLOW_PACK|LOG_REPEAT);
        return;
    }
    event->SetLog(LOG_ALLOW_PACK|LOG_PACKED);
    return;
}

bool SysEventRepeatGuard::IsEventRepeat(std::shared_ptr<SysEvent> event)
{
    uint8_t* eventData = event->AsRawData();
    if (eventData == nullptr) {
        HIVIEW_LOGE("invalid eventData.");
        return false;
    }

    SysEventHashRecord sysEventHashRecord(event->domain_, event->eventName_);
    if (!GetShaStr(eventData, sysEventHashRecord.eventHash) || sysEventHashRecord.eventHash.empty()) {
        HIVIEW_LOGE("GetShaStr failed.");
        return false;
    }
    auto happentime = SysEventRepeatDb::GetInstance().QueryHappentime(sysEventHashRecord);
    int64_t minValidTime = GetMinValidTime();
    if (happentime > minValidTime) {    // event repeat
        return true;
    }

    sysEventHashRecord.happentime = time(nullptr);
    if (happentime > 0) {   // > 0 means has record
        SysEventRepeatDb::GetInstance().Update(sysEventHashRecord);
    } else {
        SysEventRepeatDb::GetInstance().Insert(sysEventHashRecord);
        SysEventRepeatDb::GetInstance().CheckAndClearDb(minValidTime);
    }
    return false;
}
} // HiviewDFX
} // OHOS