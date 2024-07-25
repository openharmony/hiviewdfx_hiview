/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "bbox_detector_plugin.h"

#include <fstream>
#include <securec.h>

#include "common_defines.h"
#include "event.h"
#include "event_loop.h"
#include "file_util.h"
#include "hisysevent.h"
#include "hiview_logger.h"
#include "panic_report_recovery.h"
#include "plugin_factory.h"
#include "sys_event_dao.h"
#include "smart_parser.h"
#include "string_util.h"
#include "tbox.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
using namespace std;
REGISTER(BBoxDetectorPlugin);
DEFINE_LOG_LABEL(0xD002D11, "BBoxDetectorPlugin");

namespace {
    const std::string HISIPATH = "/data/hisi_logs/";
    const std::string BBOXPATH = "/data/log/bbox/";
    const std::string LOGPARSECONFIG = "/system/etc/hiview";
    const std::vector<std::string> HISTORYLOGLIST = {
        "/data/hisi_logs/history.log",
        "/data/log/bbox/history.log"
    };
}

void BBoxDetectorPlugin::OnLoad()
{
    SetName("BBoxDetectorPlugin");
    SetVersion("BBoxDetector1.0");
#ifndef UNITTEST
    eventLoop_ = GetHiviewContext()->GetSharedWorkLoop();
    if (eventLoop_ != nullptr) {
        eventLoop_->AddTimerEvent(nullptr, nullptr, [&]() {
            StartBootScan();
        }, SECONDS, false); // delay 60s
    }
    InitPanicReporter();
#endif
}

void BBoxDetectorPlugin::OnUnload()
{
    HIVIEW_LOGI("BBoxDetectorPlugin OnUnload");
    RemoveDetectBootCompletedTask();
}

bool BBoxDetectorPlugin::OnEvent(std::shared_ptr<Event> &event)
{
    if (event == nullptr || event->domain_ != "KERNEL_VENDOR") {
        return false;
    }
    auto sysEvent = Event::DownCastTo<SysEvent>(event);
    HandleBBoxEvent(sysEvent);
    return true;
}

void BBoxDetectorPlugin::WaitForLogs(const std::string& logDir)
{
    std::string doneFile = logDir + "/DONE";
    if (!Tbox::WaitForDoneFile(doneFile, 60)) { // 60s
        HIVIEW_LOGE("can not find file: %{public}s", doneFile.c_str());
    }
}

void BBoxDetectorPlugin::HandleBBoxEvent(std::shared_ptr<SysEvent> &sysEvent)
{
    if (PanicReport::IsRecoveryPanicEvent(sysEvent)) {
        return;
    }
    std::string event = sysEvent->GetEventValue("REASON");
    std::string module = sysEvent->GetEventValue("MODULE");
    std::string timeStr = sysEvent->GetEventValue("SUB_LOG_PATH");
    std::string LOG_PATH = sysEvent->GetEventValue("LOG_PATH");
    std::string name = sysEvent->GetEventValue("name_");

    std::string dynamicPaths = ((!LOG_PATH.empty() && LOG_PATH[LOG_PATH.size() - 1] == '/') ?
                                  LOG_PATH : LOG_PATH + '/') + timeStr;
#ifndef UNITTEST
    if (IsEventProcessed(name, "LOG_PATH", dynamicPaths)) {
        HIVIEW_LOGE("HandleBBoxEvent is processed event path is %{public}s", dynamicPaths.c_str());
        return;
    }
#endif
    if (name == "PANIC" && PanicReport::IsLastShortStartUp()) {
        PanicReport::CompressAndCopyLogFiles(dynamicPaths, timeStr);
    }
    if ((module == "AP") && (event == "BFM_S_NATIVE_DATA_FAIL")) {
        sysEvent->OnFinish();
    }
    auto happenTime_ = static_cast<uint64_t>(Tbox::GetHappenTime(StringUtil::GetRleftSubstr(timeStr, "-"),
        "(\\d{4})(\\d{2})(\\d{2})(\\d{2})(\\d{2})(\\d{2})"));
    sysEvent->SetEventValue("HAPPEN_TIME", happenTime_);

    WaitForLogs(dynamicPaths);
    auto eventInfos = SmartParser::Analysis(dynamicPaths, LOGPARSECONFIG, name);
    Tbox::FilterTrace(eventInfos);

    sysEvent->SetEventValue("FIRST_FRAME", eventInfos["FIRST_FRAME"].empty() ? "/" :
                                StringUtil::EscapeJsonStringValue(eventInfos["FIRST_FRAME"]));
    sysEvent->SetEventValue("SECOND_FRAME", eventInfos["SECOND_FRAME"].empty() ? "/" :
                                StringUtil::EscapeJsonStringValue(eventInfos["SECOND_FRAME"]));
    sysEvent->SetEventValue("LAST_FRAME", eventInfos["LAST_FRAME"].empty() ? "/ " :
                                StringUtil::EscapeJsonStringValue(eventInfos["LAST_FRAME"]));
    sysEvent->SetEventValue("FINGERPRINT", Tbox::CalcFingerPrint(event + module + eventInfos["FIRST_FRAME"] +
        eventInfos["SECOND_FRAME"] + eventInfos["LAST_FRAME"], 0, FP_BUFFER));
    sysEvent->SetEventValue("LOG_PATH", dynamicPaths);
    HIVIEW_LOGI("HandleBBoxEvent event: %{public}s is success ", name.c_str());
}

void BBoxDetectorPlugin::StartBootScan()
{
    for (auto historyLog : HISTORYLOGLIST) {
        int num = READ_LINE_NUM;
        string line;

        if (FileUtil::FileExists(historyLog) && historyLog.find(HISIPATH) != std::string::npos) {
            hisiHistoryPath = true;
        }

        ifstream fin(historyLog, ios::ate);
        while (FileUtil::GetLastLine(fin, line) && num > 0) {
            num--;
            std::map<std::string, std::string> historyMap = GetValueFromHistory(line);
            string name = historyMap["category"];
            if (name.empty() || name == "NORMALBOOT") {
                continue;
            }
            if (name.find(":") != std::string::npos) {
                name = StringUtil::GetRleftSubstr(name, ":");
            }
            auto time_now = static_cast<int64_t>(TimeUtil::GetMilliseconds());
            auto time_event = hisiHistoryPath ?
                static_cast<int64_t>(TimeUtil::StrToTimeStamp(StringUtil::GetMidSubstr(line, "time [", "-"),
                "%Y%m%d%H%M%S")) * MILLSECONDS :
                static_cast<int64_t>(TimeUtil::StrToTimeStamp(StringUtil::GetMidSubstr(line, "time[", "-"),
                "%Y%m%d%H%M%S")) * MILLSECONDS;
            if (abs(time_now - abs(time_event)) > ONE_DAY  ||
                IsEventProcessed(name, "LOG_PATH", historyMap["dynamicPaths"])) {
                continue;
            }
            auto happenTime_ = GetHappenTime(line, hisiHistoryPath);
            int res = CheckAndHiSysEventWrite(name, historyMap, happenTime_);
            HIVIEW_LOGI("BBox write history line is %{public}s write result =  %{public}d", line.c_str(), res);
        }
    }
}

bool BBoxDetectorPlugin::IsEventProcessed(const std::string& name, const std::string& key, const std::string& value)
{
    auto sysEventQuery = EventStore::SysEventDao::BuildQuery("KERNEL_VENDOR", {name});
    std::vector<std::string> selections { EventStore::EventCol::TS };
    EventStore::ResultSet resultSet = sysEventQuery->Select(selections).
        Where(key, EventStore::Op::EQ, value).Execute();
    if (resultSet.HasNext()) {
        return true;
    }

    return false;
}

uint64_t BBoxDetectorPlugin::GetHappenTime(std::string& line, bool hisiHistoryPath)
{
    auto happenTime_ = hisiHistoryPath ?
        static_cast<uint64_t>(Tbox::GetHappenTime(StringUtil::GetMidSubstr(line, "time [", "-"),
            "(\\d{4})(\\d{2})(\\d{2})(\\d{2})(\\d{2})(\\d{2})")) :
        static_cast<uint64_t>(Tbox::GetHappenTime(StringUtil::GetMidSubstr(line, "time[", "-"),
            "(\\d{4})(\\d{2})(\\d{2})(\\d{2})(\\d{2})(\\d{2})"));

    return happenTime_;
}

int BBoxDetectorPlugin::CheckAndHiSysEventWrite(std::string& name, std::map<std::string, std::string>& historyMap,
    uint64_t& happenTime_)
{
    int res = HiSysEventWrite(DOMAIN, name, HiSysEvent::EventType::FAULT,
        "MODULE", historyMap["module"],
        "REASON", historyMap["reason"],
        "LOG_PATH", historyMap["logPath"],
        "SUB_LOG_PATH", historyMap["subLogPath"],
        "HAPPEN_TIME", happenTime_,
        "SUMMARY", "bootup_keypoint:" + historyMap["bootup_keypoint"]);
    if (res == 0 || (res < 0 && name.find("UNKNOWNS") != std::string::npos)) {
        return res;
    } else if (res < 0) {
        name = "UNKNOWNS";
        CheckAndHiSysEventWrite(name, historyMap, happenTime_);
    }

    return res;
}

std::map<std::string, std::string> BBoxDetectorPlugin::GetValueFromHistory(std::string& line)
{
    std::map<std::string, std::string> historyMap = {
        {"category", hisiHistoryPath ? StringUtil::GetMidSubstr(line, "category [", "]") :
            StringUtil::GetMidSubstr(line, "category[", "]")},
        {"module", hisiHistoryPath ? StringUtil::GetMidSubstr(line, "core [", "]") :
            StringUtil::GetMidSubstr(line, "module[", "]")},
        {"reason", hisiHistoryPath ? StringUtil::GetMidSubstr(line, "reason [", "]") :
            StringUtil::GetMidSubstr(line, "event[", "]")},
        {"bootup_keypoint", hisiHistoryPath ? StringUtil::GetMidSubstr(line, "bootup_keypoint [", "]") :
            StringUtil::GetMidSubstr(line, "errdesc[", "]")},
        {"dynamicPaths", hisiHistoryPath ? HISIPATH + StringUtil::GetMidSubstr(line, "time [", "]") :
            BBOXPATH + StringUtil::GetMidSubstr(line, "time[", "]")},
        {"logPath", hisiHistoryPath ? HISIPATH : BBOXPATH},
        {"subLogPath", hisiHistoryPath ? StringUtil::GetMidSubstr(line, "time [", "]") :
            StringUtil::GetMidSubstr(line, "time[", "]")}
    };

    return historyMap;
}

void BBoxDetectorPlugin::AddDetectBootCompletedTask()
{
    std::lock_guard<std::mutex> lock(lock_);
    if (eventLoop_ && !timeEventAdded_) {
        timeEventId_ = eventLoop_->AddTimerEvent(nullptr, nullptr, [this] {
            if (PanicReport::IsBootCompleted()) {
                NotifyBootCompleted();
            }
        }, 1, true);
        timeEventAdded_ = true;
    }
}

void BBoxDetectorPlugin::RemoveDetectBootCompletedTask()
{
    std::lock_guard<std::mutex> lock(lock_);
    if (eventLoop_ && timeEventAdded_) {
        eventLoop_->RemoveEvent(timeEventId_);
        timeEventId_ = 0;
        timeEventAdded_ = false;
    }
}

void BBoxDetectorPlugin::NotifyBootStable()
{
    if (PanicReport::TryToReportRecoveryPanicEvent()) {
        constexpr int timeout = 10; // 10s
        eventLoop_->AddTimerEvent(nullptr, nullptr, [] {
            PanicReport::NotifyReportFinished();
        }, timeout, false);
    }
}

void BBoxDetectorPlugin::NotifyBootCompleted()
{
    HIVIEW_LOGI("System boot completed, remove the task");
    RemoveDetectBootCompletedTask();
    constexpr int timeout = 60 * 10; // 10min
    eventLoop_->AddTimerEvent(nullptr, nullptr, [this] {
        NotifyBootStable();
    }, timeout, false);
}

void BBoxDetectorPlugin::InitPanicReporter()
{
    if (!PanicReport::InitPanicReport()) {
        HIVIEW_LOGE("Failed to init panic reporter");
        return;
    }
    AddDetectBootCompletedTask();
}
}
}
