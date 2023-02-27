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

#include "plugin.h"

#include <fstream>
#include <regex>

#include "file_util.h"
#include "hisysevent.h"
#include "logger.h"
#include "parameter_ex.h"
#include "plugin_factory.h"
#include "sys_event_dao.h"
#include "string_util.h"
#include "tbox.h"

namespace OHOS {
namespace HiviewDFX {
REGISTER(HiCollieCollector);
DEFINE_LOG_TAG("HiCollieCollector");

std::string HiCollieCollector::GetListenerName()
{
    return name_;
}

bool HiCollieCollector::ReadyToLoad()
{
    return true;
}

bool HiCollieCollector::CanProcessEvent(std::shared_ptr<Event> event)
{   
    return false;
}

void HiCollieCollector::OnLoad()
{
    SetName("HiCollieCollector");
    SetVersion("HiCollieCollector 1.0");
    HIVIEW_LOGI("OnLoad.");
    AddListenerInfo(Event::MessageType::SYS_EVENT, STRINGID_SERVICE_TIMEOUT);
    AddListenerInfo(Event::MessageType::SYS_EVENT, STRINGID_SERVICE_BLOCK);
    GetHiviewContext()->RegisterUnorderedEventListener(
        std::static_pointer_cast<HiCollieCollector>(shared_from_this()));
}

void HiCollieCollector::OnUnload()
{
    HIVIEW_LOGI("OnUnload.");
}

bool HiCollieCollector::OnEvent(std::shared_ptr<Event> &event)
{
    return true;
}

void HiCollieCollector::OnUnorderedEvent(const Event &event)
{
    HIVIEW_LOGI("received event domain=%{public}s, stringid=%{public}s.\n",
        event.domain_.c_str(), event.eventName_.c_str());
    if (GetHiviewContext() == nullptr) {
        HIVIEW_LOGE("failed to get context.");
        return;
    }

    if (event.eventName_ != STRINGID_SERVICE_TIMEOUT && event.eventName_ != STRINGID_SERVICE_BLOCK) {
        HIVIEW_LOGE("invalid stringid=%{public}s.\n", event.eventName_.c_str());
        return;
    }

    Event& eventRef = const_cast<Event&>(event);
    SysEvent& sysEvent = static_cast<SysEvent&>(eventRef);
    ProcessHiCollieEvent(sysEvent);
}

std::string HiCollieCollector::SelectEventFromDB(
    int32_t pid, const std::string& processName, const std::string& moduleName) const
{
    std::string ret;

    auto eventQuery = EventStore::SysEventDao::BuildQuery(EventStore::StoreType::FAULT);
    std::vector<std::string> selections { EventStore::EventCol::TS };
    (*eventQuery).Select(selections)
        .And(EventStore::EventCol::DOMAIN, EventStore::Op::EQ, "FRAMEWORK")
        .And(EventStore::EventCol::NAME, EventStore::Op::EQ, "SERVICE_WARNING")
        .And(EventStore::EventCol::PID, EventStore::Op::EQ, pid)
        .And("PROCESS_NAME", EventStore::Op::EQ, processName)
        .And("MODULE_NAME", EventStore::Op::EQ, moduleName);

    EventStore::ResultSet set = eventQuery->Execute();
    if (set.GetErrCode() != 0) {
        HIVIEW_LOGE("failed to select event from db, error:%{public}d.", set.GetErrCode());
        return "";
    }

    while (set.HasNext()) {
        auto record = set.Next();

        std::string info = record->GetEventValue(EventStore::EventCol::INFO);
        std::regex reg("logPath:([^,]+)");
        std::smatch result;
        if (std::regex_search(info, result, reg)) {
            ret = result[1];
            break;
        }
    }
    return ret;
}

std::string HiCollieCollector::GetTimeString(unsigned long long timestamp) const
{
    struct tm tm;
    time_t ts;
    constexpr int TIME_STRING_LEN = 16;
    ts = timestamp / 1000; // 1000 ms to s
    localtime_r(&ts, &tm);
    char buf[TIME_STRING_LEN] = {0};

    auto ret = strftime(buf, TIME_STRING_LEN - 1, "%Y%m%d%H%M%S", &tm);
    if (ret) {
        HIVIEW_LOGE("strftime error");
    }
    return std::string(buf, strlen(buf));
}

void HiCollieCollector::SaveToFile(SysEvent &sysEvent, int32_t pid, const std::string& processName,
    const std::string& path, const std::string& desPath) const
{
    std::string moduleName = StringUtil::TrimStr(sysEvent.GetEventValue("MODULE_NAME"));
    std::ostringstream desText;
    desText << "DEVICE_INFO:" << Parameter::GetString("const.product.name", "Unknown") << std::endl;
    desText << "BUILD_INFO:" << Parameter::GetString("const.product.software.version", "Unknown") << std::endl;
    desText << "MODULE:" << processName << "-" << moduleName << std::endl;
    desText << "SUMMARY:" << sysEvent.GetEventValue("MSG") << std::endl;
    desText << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
    std::ifstream ifs(path, std::ios::in);
    if (!ifs.is_open()) {
        return;
    }
    desText << ifs.rdbuf();
    ifs.close();
    if (sysEvent.eventName_ == STRINGID_SERVICE_BLOCK) {
        std::string log = SelectEventFromDB(pid, processName, moduleName);
        if (log != "" && FileUtil::FileExists(log)) {
            std::ifstream fin(log, std::ios::in);
            if (!fin.is_open()) {
                return;
            }
            desText << "\n*******************************************\n";
            desText << fin.rdbuf();
            fin.close();
        }
    }
    if (!FileUtil::SaveStringToFile(desPath, desText.str())) {
        HIVIEW_LOGE("failed to save log file %{public}s.", desPath.c_str());
    }
}

void HiCollieCollector::ProcessHiCollieEvent(SysEvent &sysEvent)
{
    std::string path = "";
    int32_t pid = static_cast<int32_t>(sysEvent.GetEventIntValue("PID"));
    pid = pid ? pid : sysEvent.GetPid();
    int32_t uid = sysEvent.GetEventIntValue("UID");
    uid = uid ? uid : sysEvent.GetUid();

    std::string timestamp = GetTimeString(sysEvent.happenTime_);
    std::string processName = StringUtil::TrimStr(sysEvent.GetEventValue("PROCESS_NAME"));
    if (processName == "") {
        processName = std::to_string(pid);
    }

    std::string desFile = "";
    if (sysEvent.eventName_ == STRINGID_SERVICE_BLOCK) {
        desFile = "watchdog-" + processName + "-" + std::to_string(uid) + "-" + timestamp;
    } else if (sysEvent.eventName_ == STRINGID_SERVICE_TIMEOUT) {
        desFile = "timeout-" + processName + "-" + std::to_string(uid) + "-" + timestamp;
    }

    if (desFile == "") {
        desFile = FileUtil::ExtractFileName(path);
    }
    std::string desPath = FAULT_LOG_PATH + desFile;

    std::string info = sysEvent.GetEventValue(EventStore::EventCol::INFO);
    std::regex reg("logPath:([^,]+)");
    std::smatch result;
    if (!std::regex_search(info, result, reg)) {
        return;
    }
    path = result[1].str();
    SaveToFile(sysEvent, pid, processName, path, desPath);

    std::vector<std::string> paths = {desPath};
    HiSysEvent::Write("RELIABILITY", sysEvent.eventName_ + "_REPORT", HiSysEvent::FAULT,
        "HIVIEW_LOG_FILE_PATHS", paths,
        "PID", sysEvent.GetEventValue(EVENT_PID),
        "TGID", sysEvent.GetEventValue(EVENT_TGID),
        "MSG", sysEvent.GetEventValue(EVENT_MSG));
    ReportSysFreezeIfNeed(sysEvent, timestamp, processName, desPath);
    HIVIEW_LOGI("send event [%{public}s, %{public}s] msg:%{public}s path:%{public}s",
        "RELIABILITY", sysEvent.eventName_.c_str(), sysEvent.GetEventValue(EVENT_MSG).c_str(), desPath.c_str());
}

bool HiCollieCollector::ShouldReportSysFreeze(const std::string& processName)
{
    if ((processName.find("foundation") == std::string::npos) &&
        (processName.find("render_service") == std::string::npos)) {
        return false;
    }

    return true;
}

void HiCollieCollector::ReportSysFreezeIfNeed(SysEvent &sysEvent, const std::string& timestamp, const std::string& processName,
    const std::string& path)
{
    if (!ShouldReportSysFreeze(processName)) {
        return;
    }

    int32_t pid = static_cast<int32_t>(sysEvent.GetEventIntValue("PID"));
    pid = pid ? pid : sysEvent.GetPid();
    int32_t uid = sysEvent.GetEventIntValue("UID");
    uid = uid ? uid : sysEvent.GetUid();
    std::string fingerPrint = Tbox::CalcFingerPrint(processName, 0, FP_BUFFER);
    HiSysEvent::Write("RELIABILITY", "SYS_FREEZE", HiSysEvent::FAULT,
        "LOG_PATH", path,
        "PID", pid,
        "UID", uid,
        "MODULE", processName,
        "REASON", "Watchdog",
        "EVENT_TIME", timestamp,
        "FINGERPRINT", fingerPrint,
        "FIRST_FRAME", "/",
        "SECOND_FRAME", "/",
        "LAST_FRAME", "/",
        "SUMMARY", sysEvent.GetEventValue(EVENT_MSG));
}
} // namespace HiviewDFX
} // namespace OHOS
