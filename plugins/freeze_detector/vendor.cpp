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

#include "display_power_mgr_client.h"
#include "faultlogger_client.h"
#include "file_util.h"
#include "freeze_json_util.h"
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
    time_t ts = static_cast<long long>(timestamp) / FreezeCommon::MILLISECOND; // ms
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
    
    std::string type = freezeCommon_->IsApplicationEvent(watchPoint.GetDomain(), watchPoint.GetStringId()) ?
        APPFREEZE : SYSFREEZE;
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
    info.faultLogType = freezeCommon_->IsApplicationEvent(watchPoint.GetDomain(), watchPoint.GetStringId()) ?
        FaultLogType::APP_FREEZE : FaultLogType::SYS_FREEZE;
    info.module = processName;
    info.reason = stringId;
    std::string disPlayPowerInfo = GetDisPlayPowerInfo();
    info.summary = type + ": " + processName + " " + stringId +
        " at " + GetTimeString(watchPoint.GetTimestamp()) + "\n";
    info.summary += FreezeCommon::DISPLAY_POWER_INFO + disPlayPowerInfo;
    info.logPath = logPath;
    info.sectionMaps[FreezeCommon::HIREACE_TIME] = watchPoint.GetHitraceTime();
    info.sectionMaps[FreezeCommon::SYSRQ_TIME] = watchPoint.GetSysrqTime();
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
}

void Vendor::MergeFreezeJsonFile(const WatchPoint &watchPoint, const std::vector<WatchPoint>& list) const
{
    std::ostringstream oss;
    for (auto node : list) {
        std::string filePath = FreezeJsonUtil::GetFilePath(node.GetPid(), node.GetUid(), node.GetTimestamp());
        if (!FileUtil::FileExists(filePath)) {
            continue;
        }
        std::ifstream ifs(filePath, std::ios::in);
        if (ifs.is_open()) {
            oss << ifs.rdbuf();
            ifs.close();
        }
        FreezeJsonUtil::DelFile(filePath);
    }

    std::string mergeFilePath = FreezeJsonUtil::GetFilePath(watchPoint.GetPid(),
        watchPoint.GetUid(), watchPoint.GetTimestamp());
    int jsonFd = FreezeJsonUtil::GetFd(mergeFilePath);
    if (jsonFd < 0) {
        HIVIEW_LOGE("fail to open FreezeJsonFile! jsonFd: %{public}d", jsonFd);
        return;
    } else {
        HIVIEW_LOGI("success to open FreezeJsonFile! jsonFd: %{public}d", jsonFd);
    }
    HIVIEW_LOGI("MergeFreezeJsonFile oss size: %{public}zu.", oss.str().size());
    FileUtil::SaveStringToFd(jsonFd, oss.str());
    FreezeJsonUtil::WriteKeyValue(jsonFd, "domain", watchPoint.GetDomain());
    FreezeJsonUtil::WriteKeyValue(jsonFd, "stringId", watchPoint.GetStringId());
    FreezeJsonUtil::WriteKeyValue(jsonFd, "timestamp", watchPoint.GetTimestamp());
    FreezeJsonUtil::WriteKeyValue(jsonFd, "pid", watchPoint.GetPid());
    FreezeJsonUtil::WriteKeyValue(jsonFd, "uid", watchPoint.GetUid());
    FreezeJsonUtil::WriteKeyValue(jsonFd, "package_name", watchPoint.GetPackageName());
    FreezeJsonUtil::WriteKeyValue(jsonFd, "process_name", watchPoint.GetProcessName());
    close(jsonFd);
    HIVIEW_LOGI("success to merge FreezeJsonFiles!");
}

void Vendor::InitLogInfo(const WatchPoint& watchPoint, std::string& type, std::string& retPath,
    std::string& logPath, std::string& logName) const
{
    std::string stringId = watchPoint.GetStringId();
    std::string timestamp = GetTimeString(watchPoint.GetTimestamp());
    long uid = watchPoint.GetUid();
    std::string packageName = StringUtil::TrimStr(watchPoint.GetPackageName());
    std::string processName = StringUtil::TrimStr(watchPoint.GetProcessName());

    type = freezeCommon_->IsApplicationEvent(watchPoint.GetDomain(), watchPoint.GetStringId()) ?
        APPFREEZE : SYSFREEZE;
    if (type == SYSFREEZE) {
        processName = stringId;
    } else if (processName == "" && packageName != "") {
        processName = packageName;
    }
    if (processName == "" && packageName == "") {
        processName = stringId;
    }

    if (freezeCommon_->IsApplicationEvent(watchPoint.GetDomain(), watchPoint.GetStringId())) {
        retPath = FAULT_LOGGER_PATH + APPFREEZE + HYPHEN + processName +
            HYPHEN + std::to_string(uid) + HYPHEN + timestamp;
        logPath = FREEZE_DETECTOR_PATH + APPFREEZE + HYPHEN + processName +
            HYPHEN + std::to_string(uid) + HYPHEN + timestamp + POSTFIX;
        logName = APPFREEZE + HYPHEN + processName + HYPHEN + std::to_string(uid) + HYPHEN + timestamp + POSTFIX;
    } else {
        retPath = FAULT_LOGGER_PATH + SYSFREEZE + HYPHEN + processName +
            HYPHEN + std::to_string(uid) + HYPHEN + timestamp;
        logPath = FREEZE_DETECTOR_PATH + SYSFREEZE + HYPHEN + processName +
            HYPHEN + std::to_string(uid) + HYPHEN + timestamp + POSTFIX;
        logName = SYSFREEZE + HYPHEN + processName + HYPHEN + std::to_string(uid) + HYPHEN + timestamp + POSTFIX;
    }
}

void Vendor::InitLogBody(const std::vector<WatchPoint>& list, std::ostringstream& body,
    bool& isFileExists) const
{
    HIVIEW_LOGI("merging list size %{public}zu", list.size());
    for (auto node : list) {
        std::string filePath = node.GetLogPath();
        if (filePath == "nolog" || filePath == "") {
            HIVIEW_LOGI("only header, no content:[%{public}s, %{public}s]",
                node.GetDomain().c_str(), node.GetStringId().c_str());
            DumpEventInfo(body, HEADER, node);
            continue;
        }

        if (FileUtil::FileExists(filePath) == false) {
            isFileExists = false;
            HIVIEW_LOGE("[%{public}s, %{public}s] File:%{public}s does not exist",
                node.GetDomain().c_str(), node.GetStringId().c_str(), filePath.c_str());
            return;
        }

        HIVIEW_LOGI("merging file:%{public}s.", filePath.c_str());

        std::ifstream ifs(filePath, std::ios::in);
        if (!ifs.is_open()) {
            HIVIEW_LOGE("cannot open log file for reading:%{public}s.", filePath.c_str());
            DumpEventInfo(body, HEADER, node);
            continue;
        }

        body << HEADER << std::endl;
        body << ifs.rdbuf();
        ifs.close();
    }
}

std::string Vendor::MergeEventLog(
    const WatchPoint &watchPoint, const std::vector<WatchPoint>& list,
    const std::vector<FreezeResult>& result) const
{
    if (freezeCommon_ == nullptr) {
        return "";
    }

    std::string type;
    std::string retPath;
    std::string logPath;
    std::string logName;
    InitLogInfo(watchPoint, type, retPath, logPath, logName);

    if (FileUtil::FileExists(retPath)) {
        HIVIEW_LOGW("filename: %{public}s is existed, direct use.", retPath.c_str());
        return retPath;
    }

    std::ostringstream header;
    DumpEventInfo(header, TRIGGER_HEADER, watchPoint);

    std::ostringstream body;
    bool isFileExists = true;
    InitLogBody(list, body, isFileExists);
    HIVIEW_LOGI("After Init --body size: %{public}zu.", body.str().size());
    if (!isFileExists) {
        HIVIEW_LOGE("Failed to open the file.");
        return "";
    }

    if (type == APPFREEZE) {
        MergeFreezeJsonFile(watchPoint, list);
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

std::string Vendor::GetDisPlayPowerInfo()
{
    std::string disPlayPowerInfo;
    uint32_t brightness = OHOS::DisplayPowerMgr::DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    disPlayPowerInfo =  "brightness:" + StringUtil::ToString(brightness);

    OHOS::PowerMgr::PowerState powerState = OHOS::PowerMgr::PowerMgrClient::GetInstance().GetState();
    disPlayPowerInfo += ", powerState:" + GetPowerStateString(powerState) + "\n";
    return disPlayPowerInfo;
}

std::string Vendor::GetPowerStateString(OHOS::PowerMgr::PowerState state)
{
    switch (state) {
        case OHOS::PowerMgr::PowerState::AWAKE:
            return std::string("AWAKE");
        case OHOS::PowerMgr::PowerState::FREEZE:
            return std::string("FREEZE");
        case OHOS::PowerMgr::PowerState::INACTIVE:
            return std::string("INACTIVE");
        case OHOS::PowerMgr::PowerState::STAND_BY:
            return std::string("STAND_BY");
        case OHOS::PowerMgr::PowerState::DOZE:
            return std::string("DOZE");
        case OHOS::PowerMgr::PowerState::SLEEP:
            return std::string("SLEEP");
        case OHOS::PowerMgr::PowerState::HIBERNATE:
            return std::string("HIBERNATE");
        case OHOS::PowerMgr::PowerState::SHUTDOWN:
            return std::string("SHUTDOWN");
        case OHOS::PowerMgr::PowerState::UNKNOWN:
            return std::string("UNKNOWN");
        default:
            break;
    }
    return std::string("UNKNOWN");
}
} // namespace HiviewDFX
} // namespace OHOS
