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

#include "vendor.h"

#include "db_helper.h"
#include "faultlogger_client.h"
#include "file_util.h"
#include "hisysevent.h"
#include "logger.h"
#include "plugin.h"
#include "resolver.h"
#include "string_util.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("FreezeDetector");

const std::vector<std::pair<std::string, std::string>> Vendor::applicationPairs_ = {
    {"APPEXECFWK", "UI_BLOCK_3S"},
    {"APPEXECFWK", "UI_BLOCK_6S"},
};

const std::vector<std::pair<std::string, std::string>> Vendor::systemPairs_ = {
    {"HUNGTASK", "HUNGTASK"},
};

bool Vendor::IsFreezeEvent(const std::string& domain, const std::string& stringId) const
{
    for (auto const pair : applicationPairs_) {
        if (domain == pair.first && stringId == pair.second) {
            return true;
        }
    }
    for (auto const pair : systemPairs_) {
        if (domain == pair.first && stringId == pair.second) {
            return true;
        }
    }
    return false;
}

bool Vendor::IsApplicationEvent(const std::string& domain, const std::string& stringId) const
{
    for (auto const pair : applicationPairs_) {
        if (domain == pair.first && stringId == pair.second) {
            return true;
        }
    }
    return false;
}

bool Vendor::IsSystemEvent(const std::string& domain, const std::string& stringId) const
{
    for (auto const pair : systemPairs_) {
        if (domain == pair.first && stringId == pair.second) {
            return true;
        }
    }
    return false;
}

bool Vendor::IsSystemResult(const FreezeResult& result) const
{
    return result.GetId() == SYSTEM_RESULT_ID;
}

bool Vendor::IsApplicationResult(const FreezeResult& result) const
{
    return result.GetId() == APPLICATION_RESULT_ID;
}

bool Vendor::IsBetaVersion() const
{
    return true;
}

std::set<std::string> Vendor::GetFreezeStringIds() const
{
    std::set<std::string> set;

    for (auto const pair : applicationPairs_) {
        set.insert(pair.second);
    }
    for (auto const pair : systemPairs_) {
        set.insert(pair.second);
    }

    return set;
}

bool Vendor::GetMatchString(const std::string& src, std::string& dst, const std::string& pattern) const
{
    std::regex reg(pattern);
    std::smatch result;
    if (std::regex_search(src, result, reg)) {
        dst = StringUtil::TrimStr(result[1], '\n');
        return true;
    }
    return false;
}

bool Vendor::ReduceRelevanceEvents(std::list<WatchPoint>& list, const FreezeResult& result) const
{
    HIVIEW_LOGI("before size=%{public}zu", list.size());
    if (IsSystemResult(result) == false && IsApplicationResult(result) == false) {
        list.clear();
        return false;
    }

    // erase if not system event
    if (IsSystemResult(result)) {
        std::list<WatchPoint>::iterator watchPoint;
        for (watchPoint = list.begin(); watchPoint != list.end();) {
            if (IsSystemEvent(watchPoint->GetDomain(), watchPoint->GetStringId())) {
                watchPoint++;
            } else {
                watchPoint = list.erase(watchPoint);
            }
        }
    }

    // erase if not application event
    if (IsApplicationResult(result)) {
        std::list<WatchPoint>::iterator watchPoint;
        for (watchPoint = list.begin(); watchPoint != list.end();) {
            if (IsApplicationEvent(watchPoint->GetDomain(), watchPoint->GetStringId())) {
                watchPoint++;
            } else {
                watchPoint = list.erase(watchPoint);
            }
        }
    }

    list.sort();
    list.unique();
    HIVIEW_LOGI("after size=%{public}zu", list.size());
    return list.size() != 0;
}

std::string Vendor::GetTimeString(unsigned long timestamp) const
{
    struct tm tm;
    time_t ts;
    ts = timestamp / FreezeResolver::MILLISECOND; // ms
    localtime_r(&ts, &tm);
    char buf[TIME_STRING_LEN] = {0};

    strftime(buf, TIME_STRING_LEN - 1, "%Y%m%d%H%M%S", &tm);
    return std::string(buf, strlen(buf));
}

std::string Vendor::MergeEventLog(
    const WatchPoint &watchPoint, const std::list<WatchPoint>& list,
    const FreezeResult& result, std::string& digest) const
{
    std::string domain = watchPoint.GetDomain();
    std::string stringId = watchPoint.GetStringId();
    std::string timestamp = GetTimeString(watchPoint.GetTimestamp());
    long pid = watchPoint.GetPid();
    long uid = watchPoint.GetUid();
    std::string packageName = StringUtil::TrimStr(watchPoint.GetPackageName());
    std::string processName = StringUtil::TrimStr(watchPoint.GetProcessName());
    std::string msg = watchPoint.GetMsg();
    if (packageName == "" && processName != "") {
        packageName = processName;
    }
    if (packageName == "" && processName == "") {
        packageName = stringId;
    }

    std::string retPath = FAULT_LOGGER_PATH + APPFREEZE + HYPHEN + packageName + HYPHEN + std::to_string(uid) + HYPHEN + timestamp;
    std::string logPath = FAULT_LOGGER_PATH + APPFREEZE + HYPHEN + packageName + HYPHEN + std::to_string(uid) + HYPHEN + timestamp + POSTFIX;

    std::ofstream output(logPath, std::ios::out);
    if (!output.is_open()) {
        HIVIEW_LOGE("cannot open log file for writing:%{public}s.\n", logPath.c_str());
        return "";
    }
    output << HEADER << std::endl;
    output << FreezeDetectorPlugin::EVENT_DOMAIN << FreezeDetectorPlugin::COLON << domain << std::endl;
    output << FreezeDetectorPlugin::EVENT_STRINGID << FreezeDetectorPlugin::COLON << stringId << std::endl;
    output << FreezeDetectorPlugin::EVENT_TIMESTAMP << FreezeDetectorPlugin::COLON <<
        watchPoint.GetTimestamp() << std::endl;
    output << FreezeDetectorPlugin::EVENT_PID << FreezeDetectorPlugin::COLON << pid << std::endl;
    output << FreezeDetectorPlugin::EVENT_UID << FreezeDetectorPlugin::COLON << uid << std::endl;
    output << FreezeDetectorPlugin::EVENT_PACKAGE_NAME << FreezeDetectorPlugin::COLON << packageName << std::endl;
    output << FreezeDetectorPlugin::EVENT_PROCESS_NAME << FreezeDetectorPlugin::COLON << processName << std::endl;
    output << FreezeDetectorPlugin::EVENT_MSG << FreezeDetectorPlugin::COLON << msg << std::endl;
    output.flush();
    output.close();

    HIVIEW_LOGI("merging list size %{public}zu", list.size());
    std::ostringstream body;
    for (auto node : list) {
        std::string filePath = node.GetLogPath();
        HIVIEW_LOGI("merging file:%{public}s.\n", filePath.c_str());
        if (filePath == "" || filePath == "nolog" || FileUtil::FileExists(filePath) == false) {
            continue;
        }

        std::ifstream ifs(filePath, std::ios::in);
        if (!ifs.is_open()) {
            HIVIEW_LOGE("cannot open log file for reading:%{public}s.\n", filePath.c_str());
            continue;
        }

        std::ofstream ofs(logPath, std::ios::out | std::ios::app);
        if (!ofs.is_open()) {
            ifs.close();
            HIVIEW_LOGE("cannot open log file for writing:%{public}s.\n", logPath.c_str());
            continue;
        }

        ofs << HEADER << std::endl;
        ofs << ifs.rdbuf();

        ofs.flush();
        ofs.close();
        ifs.close();
    }

    std::string type = IsSystemResult(result) ? SP_SYSTEMHUNGFAULT : SP_APPFREEZE;
    auto eventInfos = SmartParser::Analysis(logPath, SMART_PARSER_PATH, type);
    digest = eventInfos[SP_ENDSTACK];
    std::string summary = eventInfos[SP_ENDSTACK];
    summary = EVENT_SUMMARY + FreezeDetectorPlugin::COLON + NEW_LINE + summary;

    FaultLogInfoInner info;
    info.time = watchPoint.GetTimestamp() / FreezeResolver::MILLISECOND;
    info.id = uid;
    info.pid = pid;
    info.faultLogType = FaultLogType::APP_FREEZE;
    info.module = packageName;
    info.reason = stringId;
    info.summary = summary;
    info.logPath = logPath;
    AddFaultLog(info);

    HiSysEvent::Write("RELIABILITY", IsSystemResult(result) ? "SYSTEM_FREEZE" : "APP_FREEZE", HiSysEvent::FAULT,
        "SUB_EVENT_TYPE", stringId,
        "EVENT_TIME", timestamp,
        "MODULE", packageName,
        "PNAME", packageName,
        "REASON", stringId,
        "DIAG_INFO", summary,
        "STACK", summary);

    return retPath;
}

std::shared_ptr<PipelineEvent> Vendor::MakeEvent(
    const WatchPoint &watchPoint, const WatchPoint& matchedWatchPoint,
    const std::list<WatchPoint>& list, const FreezeResult& result,
    const std::string& logPath, const std::string& digest) const
{
    for (auto node : list) {
        DBHelper::UpdateEventIntoDB(node, result.GetId());
    }

    return nullptr;
}

Vendor::Vendor()
{
}

Vendor::~Vendor()
{
}
} // namespace HiviewDFX
} // namespace OHOS
