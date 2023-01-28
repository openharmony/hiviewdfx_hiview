/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "faultlog_database.h"

#include <algorithm>
#include <cinttypes>
#include <list>
#include <mutex>
#include <string>

#include "faultlog_info.h"
#include "faultlog_util.h"
#include "hisysevent.h"
#include "hiview_global.h"
#include "logger.h"
#include "log_analyzer.h"
#include "string_util.h"
#include "sys_event.h"
#include "sys_event_dao.h"
#include "time_util.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("FaultLogDatabase");
namespace {
static const std::vector<std::string> QUERY_ITEMS =
    { "time_", "name_", "uid_", "pid_", "MODULE", "REASON", "SUMMARY", "LOG_PATH", "FAULT_TYPE" };
static const std::string LOG_PATH_BASE = "/data/log/faultlog/faultlogger/";
bool ParseFaultLogInfoFromJson(const std::string& jsonStr, FaultLogInfo& info)
{
    auto sysEvent = std::make_unique<SysEvent>("FaultLogDatabase", nullptr, jsonStr);
    HIVIEW_LOGI("parse FaultLogInfo from %{public}s. 0", jsonStr.c_str());
    if (sysEvent->ParseJson() < 0) {
        HIVIEW_LOGI("Failed to parse FaultLogInfo from queryResult.");
        return false;
    }
    info.time = static_cast<int64_t>(std::atoll(sysEvent->GetEventValue("HAPPEN_TIME").c_str()));
    if (info.time == 0) {
        info.time = sysEvent->GetEventIntValue("HAPPEN_TIME");
    }
    info.pid = sysEvent->GetEventIntValue("PID");
    if (info.pid == 0) {
        info.pid = sysEvent->GetEventIntValue("pid_");
    }
    info.id = sysEvent->GetEventIntValue("UID");
    if (info.id == 0) {
        info.id = sysEvent->GetEventIntValue("uid_");
    }
    info.faultLogType = std::atoi(sysEvent->GetEventValue("FAULT_TYPE").c_str());
    info.module = sysEvent->GetEventValue("MODULE");
    info.reason = sysEvent->GetEventValue("REASON");
    info.summary = StringUtil::UnescapeJsonStringValue(sysEvent->GetEventValue("SUMMARY"));
    info.logPath = LOG_PATH_BASE + GetFaultLogName(info);
    return true;
}
}

void FaultLogDatabase::SaveFaultLogInfo(FaultLogInfo& info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::map<std::string, std::string> eventInfos;
    AnalysisFaultlog(info, eventInfos);
    HiSysEvent::Write("RELIABILITY", GetFaultNameByType(info.faultLogType, false), HiSysEvent::EventType::FAULT,
        "FAULT_TYPE", std::to_string(info.faultLogType),
        "PID", info.pid,
        "UID", info.id,
        "MODULE", info.module,
        "REASON", info.reason,
        "SUMMARY", info.summary,
        "LOG_PATH", info.logPath,
        "VERSION", info.sectionMap.find("VERSION") != info.sectionMap.end() ? info.sectionMap.at("VERSION") : "",
        "HAPPEN_TIME", std::to_string(info.time),
        "PNAME", eventInfos["PNAME"].empty() ? "/" : eventInfos["PNAME"],
        "FIRST_FRAME", eventInfos["FIRST_FRAME"].empty() ? "/" : eventInfos["FIRST_FRAME"],
        "SECOND_FRAME", eventInfos["SECOND_FRAME"].empty() ? "/" : eventInfos["SECOND_FRAME"],
        "LAST_FRAME", eventInfos["LAST_FRAME"].empty() ? "/" : eventInfos["LAST_FRAME"],
        "FINGERPRINT", eventInfos["fingerPrint"].empty() ? "/" : eventInfos["fingerPrint"]
    );
}

std::list<FaultLogInfo> FaultLogDatabase::GetFaultInfoList(const std::string& module, int32_t id,
    int32_t faultType, int32_t maxNum)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::list<FaultLogInfo> queryResult;
    auto query = EventStore::SysEventDao::BuildQuery(EventStore::StoreType::FAULT);
    EventStore::Cond uidCond("UID", EventStore::Op::EQ, id);
    EventStore::Cond hiviewCond("uid_", EventStore::Op::EQ, static_cast<int64_t>(getuid()));
    EventStore::Cond condLeft = uidCond.And(hiviewCond);
    EventStore::Cond condRight("uid_", EventStore::Op::EQ, id);
    EventStore::Cond condTotal;
    if (faultType == FaultLogType::CPP_CRASH || faultType == FaultLogType::APP_FREEZE) {
        condTotal = condLeft;
    } else if (faultType == FaultLogType::JS_CRASH) {
        condTotal = condRight;
    } else {
        condTotal = condLeft.Or(condRight);
    }
    (*query).Select(QUERY_ITEMS).Where(condTotal).Order("time_", false);
    if (id != 0) {
        query->And("MODULE", EventStore::Op::EQ, module);
    }

    if (faultType != 0) {
        query->And("FAULT_TYPE", EventStore::Op::EQ, faultType);
    }

    EventStore::ResultSet resultSet = query->Execute(maxNum);
    while (resultSet.HasNext()) {
        auto it = resultSet.Next();
        FaultLogInfo info;
        if (!ParseFaultLogInfoFromJson(it->jsonExtraInfo_, info)) {
            HIVIEW_LOGI("Failed to parse FaultLogInfo from queryResult.");
            continue;
        }
        queryResult.push_back(info);
    }
    return queryResult;
}

bool FaultLogDatabase::IsFaultExist(int32_t pid, int32_t uid, int32_t faultType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto query = EventStore::SysEventDao::BuildQuery(EventStore::StoreType::FAULT);
    EventStore::Cond pidUpperCond("PID", EventStore::Op::EQ, pid);
    EventStore::Cond pidLowerCond("pid_", EventStore::Op::EQ, pid);
    EventStore::Cond uidUpperCond("UID", EventStore::Op::EQ, uid);
    EventStore::Cond uidLowerCond("uid_", EventStore::Op::EQ, uid);
    EventStore::Cond hiviewCond("uid_", EventStore::Op::EQ, static_cast<int64_t>(getuid()));
    EventStore::Cond typeCond("FAULT_TYPE", EventStore::Op::EQ, faultType);
    EventStore::Cond condLeft = hiviewCond.And(pidUpperCond).And(uidUpperCond).And(typeCond);
    EventStore::Cond condRight = pidLowerCond.And(uidLowerCond).And(typeCond);
    EventStore::Cond condTotal;
    if (faultType == FaultLogType::CPP_CRASH || faultType == FaultLogType::APP_FREEZE) {
        condTotal = condLeft;
    } else if (faultType == FaultLogType::JS_CRASH) {
        condTotal = condRight;
    } else {
        condTotal = condLeft.Or(condRight);
    }
    (*query).Select(QUERY_ITEMS).Where(condTotal).Order("time_", false);
    return query->Execute(1).HasNext();
}
}  // namespace HiviewDFX
}  // namespace OHOS
