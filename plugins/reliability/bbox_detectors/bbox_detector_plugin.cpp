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
#include "logger.h"
#include "plugin_factory.h"
#include "sys_event_dao.h"
#include "smart_parser.h"
#include "string_util.h"
#include "tbox.h"
#include "time_util.h"


namespace OHOS {
namespace HiviewDFX {
using namespace std;
REGISTER(BBoxDetectorPlugin)
DEFINE_LOG_TAG("BBoxDetectorPlugin");

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

    auto eventloop = GetHiviewContext()->GetSharedWorkLoop();
    if (eventloop != nullptr) {
        eventloop->AddTimerEvent(nullptr, nullptr, [&]() {
            StartBootScan();
        }, SECONDS, false); // delay 10s
    }
}

void BBoxDetectorPlugin::OnUnload()
{
    HIVIEW_LOGI("BBoxDetectorPlugin OnUnload");
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
    std::string event = sysEvent->GetEventValue("REASON");
    std::string module = sysEvent->GetEventValue("MODULE");
    std::string timeStr = sysEvent->GetEventValue("SUB_LOG_PATH");
    std::string LOG_PATH = sysEvent->GetEventValue("LOG_PATH");
    std::string name = sysEvent->GetEventValue("name_");

    std::string dynamicPaths = ((!LOG_PATH.empty() && LOG_PATH[LOG_PATH.size() - 1] == '/') ?
                                  LOG_PATH : LOG_PATH + '/') + timeStr;
    if (IsEventProcessed(name, "LOG_PATH", dynamicPaths)) {
        HIVIEW_LOGE("HandleBBoxEvent is processed event path is %{public}s", dynamicPaths.c_str());
        return;
    }

    auto times = static_cast<int64_t>(TimeUtil::StrToTimeStamp(StringUtil::GetRleftSubstr(timeStr, "-"),
                                                               "%Y%m%d%H%M%S"));
    sysEvent->SetEventValue("HAPPEN_TIME", times * MILLSECONDS);

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
            string name = hisiHistoryPath ? StringUtil::GetMidSubstr(line, "category [", "]") :
                StringUtil::GetMidSubstr(line, "category[", "]");
            if (name.empty() || name == "NORMALBOOT") {
                continue;
            }
            string module = hisiHistoryPath ? StringUtil::GetMidSubstr(line, "core [", "]") :
                StringUtil::GetMidSubstr(line, "module[", "]");
            string reason = hisiHistoryPath ? StringUtil::GetMidSubstr(line, "reason [", "]") :
                StringUtil::GetMidSubstr(line, "event[", "]");
            string bootup_keypoint = hisiHistoryPath ? StringUtil::GetMidSubstr(line, "bootup_keypoint [", "]") :
                StringUtil::GetMidSubstr(line, "errdesc[", "]");
            string dynamicPaths = hisiHistoryPath ? HISIPATH + StringUtil::GetMidSubstr(line, "time [", "]") :
                BBOXPATH + StringUtil::GetMidSubstr(line, "time[", "]");
            auto time_now = static_cast<int64_t>(TimeUtil::GetMilliseconds());
            auto time_event = hisiHistoryPath ?
                static_cast<int64_t>(TimeUtil::StrToTimeStamp(StringUtil::GetMidSubstr(line, "time [", "-"),
                "%Y%m%d%H%M%S")) * MILLSECONDS :
                static_cast<int64_t>(TimeUtil::StrToTimeStamp(StringUtil::GetMidSubstr(line, "time[", "-"),
                "%Y%m%d%H%M%S")) * MILLSECONDS;
            if (abs(time_now - abs(time_event)) > ONE_DAY  || IsEventProcessed(name, "LOG_PATH", dynamicPaths)) {
                continue;
            }
            string logPath = hisiHistoryPath ? HISIPATH : BBOXPATH;
            string subLogPath = hisiHistoryPath ? StringUtil::GetMidSubstr(line, "time [", "]") :
                StringUtil::GetMidSubstr(line, "time[", "]");
            int res = HiSysEventWrite(DOMAIN, name, HiSysEvent::EventType::FAULT,
                "MODULE", module,
                "REASON", reason,
                "LOG_PATH", logPath,
                "SUB_LOG_PATH", subLogPath,
                "SUMMARY", "bootup_keypoint:" + bootup_keypoint);
            HIVIEW_LOGI("BBox write history line is %{public}s write result =  %{public}d", line.c_str(), res);
        }
    }
}

bool BBoxDetectorPlugin::IsEventProcessed (const std::string& name, const std::string& key, const std::string& value)
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
}
}
