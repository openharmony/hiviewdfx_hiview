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

#include "faultlogger_client.h"
#include "file_util.h"
#include "logger.h"
#include "string_util.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("FreezeDetector");
bool Vendor::ReduceRelevanceEvents(std::list<WatchPoint>& list, const FreezeResult& result) const
{
    HIVIEW_LOGI("before size=%{public}zu", list.size());
    if (freezeCommon_ == nullptr) {
        return false;
    }
    if (freezeCommon_->IsSystemResult(result) == false && freezeCommon_->IsApplicationResult(result) == false) {
        list.clear();
        return false;
    }

    // erase if not system event
    if (freezeCommon_->IsSystemResult(result)) {
        std::list<WatchPoint>::iterator watchPoint;
        for (watchPoint = list.begin(); watchPoint != list.end();) {
            if (freezeCommon_->IsSystemEvent(watchPoint->GetDomain(), watchPoint->GetStringId())) {
                watchPoint++;
            } else {
                watchPoint = list.erase(watchPoint);
            }
        }
    }

    // erase if not application event
    if (freezeCommon_->IsApplicationResult(result)) {
        std::list<WatchPoint>::iterator watchPoint;
        for (watchPoint = list.begin(); watchPoint != list.end();) {
            if (freezeCommon_->IsApplicationEvent(watchPoint->GetDomain(), watchPoint->GetStringId())) {
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

std::string Vendor::GetTimeString(unsigned long long timestamp) const
{
    struct tm tm;
    time_t ts;
    ts = timestamp / FreezeCommon::MILLISECOND; // ms
    localtime_r(&ts, &tm);
    char buf[TIME_STRING_LEN] = {0};

    strftime(buf, TIME_STRING_LEN - 1, "%Y%m%d%H%M%S", &tm);
    return std::string(buf, strlen(buf));
}

std::string Vendor::SendFaultLog(const WatchPoint &watchPoint, const std::string& logPath,
    const std::string& logName) const
{
    if (freezeCommon_ == nullptr) {
        return "";
    }
    std::string packageName = StringUtil::TrimStr(watchPoint.GetPackageName());
    std::string processName = StringUtil::TrimStr(watchPoint.GetProcessName());
    std::string stringId = watchPoint.GetStringId();
    
    std::string type = freezeCommon_->IsApplicationEvent(watchPoint.GetDomain(), watchPoint.GetStringId())
        ? APPFREEZE : SYSFREEZE;
    if (type == SYSFREEZE) {
        processName = stringId;
    } else if (processName == "" && packageName != "") {
        processName = packageName;
    }
    if (processName == "" && packageName == "") {
        processName = stringId;
    }

    FaultLogInfoInner info;
    info.time = watchPoint.GetTimestamp();
    info.id = watchPoint.GetUid();
    info.pid = watchPoint.GetPid();
    info.faultLogType = freezeCommon_->IsApplicationEvent(watchPoint.GetDomain(), watchPoint.GetStringId())
        ? FaultLogType::APP_FREEZE : FaultLogType::SYS_FREEZE;
    info.module = processName;
    info.reason = stringId;
    info.summary = type + ": " + processName + " " + stringId
        + " at " + GetTimeString(watchPoint.GetTimestamp()) + "\n";
    info.logPath = logPath;
    AddFaultLog(info);
    return logPath;
}

void Vendor::DumpEventInfo(std::ostringstream& oss, const std::string& header, const WatchPoint& watchPoint) const
{
    uint64_t timestamp = watchPoint.GetTimestamp() / TimeUtil::SEC_TO_MILLISEC;
    oss << header << std::endl;
    oss << FreezeCommon::EVENT_DOMAIN << FreezeCommon::COLON << watchPoint.GetDomain() << std::endl;
    oss << FreezeCommon::EVENT_STRINGID << FreezeCommon::COLON << watchPoint.GetStringId() << std::endl;
    oss << FreezeCommon::EVENT_TIMESTAMP << FreezeCommon::COLON <<
        TimeUtil::TimestampFormatToDate(timestamp, "%Y/%m/%d-%H:%M:%S") <<
        ":" << watchPoint.GetTimestamp() % TimeUtil::SEC_TO_MILLISEC << std::endl;
    oss << FreezeCommon::EVENT_PID << FreezeCommon::COLON << watchPoint.GetPid() << std::endl;
    oss << FreezeCommon::EVENT_UID << FreezeCommon::COLON << watchPoint.GetUid() << std::endl;
    oss << FreezeCommon::EVENT_PACKAGE_NAME << FreezeCommon::COLON << watchPoint.GetPackageName() << std::endl;
    oss << FreezeCommon::EVENT_PROCESS_NAME << FreezeCommon::COLON << watchPoint.GetProcessName() << std::endl;
    oss << FreezeCommon::EVENT_MSG << FreezeCommon::COLON << watchPoint.GetMsg() << std::endl;
}

std::string Vendor::MergeEventLog(
    const WatchPoint &watchPoint, const std::vector<WatchPoint>& list,
    const std::vector<FreezeResult>& result) const
{
    if (freezeCommon_ == nullptr) {
        return "";
    }

    std::string domain = watchPoint.GetDomain();
    std::string stringId = watchPoint.GetStringId();
    std::string timestamp = GetTimeString(watchPoint.GetTimestamp());
    long uid = watchPoint.GetUid();
    std::string packageName = StringUtil::TrimStr(watchPoint.GetPackageName());
    std::string processName = StringUtil::TrimStr(watchPoint.GetProcessName());
    std::string msg = watchPoint.GetMsg();

    std::string type = freezeCommon_->IsApplicationEvent(watchPoint.GetDomain(), watchPoint.GetStringId())
        ? APPFREEZE : SYSFREEZE;
    if (type == SYSFREEZE) {
        processName = stringId;
    } else if (processName == "" && packageName != "") {
        processName = packageName;
    }
    if (processName == "" && packageName == "") {
        processName = stringId;
    }

    std::string retPath;
    std::string logPath;
    std::string logName;
    if (freezeCommon_->IsApplicationEvent(watchPoint.GetDomain(), watchPoint.GetStringId())) {
        retPath = FAULT_LOGGER_PATH + APPFREEZE + HYPHEN + processName
            + HYPHEN + std::to_string(uid) + HYPHEN + timestamp;
        logPath = FREEZE_DETECTOR_PATH + APPFREEZE + HYPHEN + processName
            + HYPHEN + std::to_string(uid) + HYPHEN + timestamp + POSTFIX;
        logName = APPFREEZE + HYPHEN + processName + HYPHEN + std::to_string(uid) + HYPHEN + timestamp + POSTFIX;
    } else {
        retPath = FAULT_LOGGER_PATH + SYSFREEZE + HYPHEN + processName
            + HYPHEN + std::to_string(uid) + HYPHEN + timestamp;
        logPath = FREEZE_DETECTOR_PATH + SYSFREEZE + HYPHEN + processName
            + HYPHEN + std::to_string(uid) + HYPHEN + timestamp + POSTFIX;
        logName = SYSFREEZE + HYPHEN + processName + HYPHEN + std::to_string(uid) + HYPHEN + timestamp + POSTFIX;
    }

    if (FileUtil::FileExists(retPath)) {
        HIVIEW_LOGW("filename: %{public}s is existed, direct use.", retPath.c_str());
        return retPath;
    }

    std::ostringstream header;
    DumpEventInfo(header, TRIGGER_HEADER, watchPoint);

    HIVIEW_LOGI("merging list size %{public}zu", list.size());
    std::ostringstream body;
    for (auto node : list) {
        std::string filePath = node.GetLogPath();
        HIVIEW_LOGI("merging file:%{public}s.", filePath.c_str());

        if (filePath == "nolog" || filePath == "" || FileUtil::FileExists(filePath) == false) {
            HIVIEW_LOGI("only header, no content:[%{public}s, %{public}s]",
                node.GetDomain().c_str(), node.GetStringId().c_str());
            DumpEventInfo(body, HEADER, node);
            continue;
        }

        std::ifstream ifs(filePath, std::ios::in);
        if (!ifs.is_open()) {
            HIVIEW_LOGE("cannot open log file for reading:%{public}s.", filePath.c_str());
            DumpEventInfo(body, HEADER, node);
            continue;
        }

        body << HEADER << std::endl;
        if (node.GetDomain() == "RELIABILITY" && node.GetStringId() == "STACK") {
            body << FreezeCommon::EVENT_DOMAIN << "=" << node.GetDomain() << std::endl;
            body << FreezeCommon::EVENT_STRINGID << "=" << node.GetStringId() << std::endl;
            body << FreezeCommon::EVENT_TIMESTAMP << "=" << node.GetTimestamp() << std::endl;
            body << FreezeCommon::EVENT_PID << "=" << watchPoint.GetPid() << std::endl;
            body << FreezeCommon::EVENT_UID << "=" << watchPoint.GetUid() << std::endl;
            body << FreezeCommon::EVENT_PACKAGE_NAME << "=" << watchPoint.GetPackageName() << std::endl;
            body << FreezeCommon::EVENT_PROCESS_NAME << "=" << watchPoint.GetProcessName() << std::endl;
            body << FreezeCommon::EVENT_MSG << "=" << node.GetMsg() << std::endl;
            body << std::endl;
        }
        body << ifs.rdbuf();
        ifs.close();
    }

    int fd = logStore_->CreateLogFile(logName);
    if (fd < 0) {
        HIVIEW_LOGE("failed to create log file %{public}s.", logPath.c_str());
        return "";
    }

    FileUtil::SaveStringToFd(fd, header.str());
    FileUtil::SaveStringToFd(fd, body.str());
    close(fd);
    return SendFaultLog(watchPoint, logPath, logName);
}

bool Vendor::Init()
{
    if (freezeCommon_ == nullptr) {
        return false;
    }
    logStore_ = std::make_unique<LogStoreEx>(FREEZE_DETECTOR_PATH, true);
    logStore_->SetMaxSize(MAX_FOLDER_SIZE);
    logStore_->SetMinKeepingFileNumber(MAX_FILE_NUM);
    logStore_->Init();
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
