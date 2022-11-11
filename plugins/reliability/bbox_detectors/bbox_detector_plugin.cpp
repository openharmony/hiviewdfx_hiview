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

#include "event.h"
#include "file_util.h"
#include "common_defines.h"
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

void BBoxDetectorPlugin::OnLoad()
{
    SetName("BBoxDetectorPlugin");
    SetVersion("BBoxDetector1.0");
}

void BBoxDetectorPlugin::OnUnload()
{
    HIVIEW_LOGI("BBoxDetectorPlugin OnUnload");
}

bool BBoxDetectorPlugin::OnEvent(std::shared_ptr<Event> &event)
{
    if (!CanProcessEvent(event)) {
        return false;
    }
    auto sysEvent = Event::DownCastTo<SysEvent>(event);
    HandleBBoxEvent(sysEvent);
    return true;
}

bool BBoxDetectorPlugin::CanProcessEvent(std::shared_ptr<Event> event)
{
    if (event == nullptr || event->domain_ != "KERNEL_VENDOR") {
        return false;
    }

    auto sysEvent = Event::DownCastTo<SysEvent>(event);
    auto subEventType = sysEvent->GetEventValue("name_");
    if (subEventType != "PANIC") {
        HIVIEW_LOGI("sub event type is %{public}s, not care", subEventType.c_str());
        return false;
    }
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
    std::string dynamicPaths = LOG_PATH + timeStr;
    auto times = static_cast<int64_t>(TimeUtil::StrToTimeStamp(StringUtil::GetRleftSubstr(timeStr, "-"),
                                                               "%Y%m%d%H%M%S"));
    sysEvent->SetEventValue("HAPPEN_TIME", times);

    WaitForLogs(dynamicPaths);
    auto eventInfos = SmartParser::Analysis(dynamicPaths, logParseConfig_, sysEvent->GetEventValue("name_"));
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
    if (sysEvent->GetSeq() != 0 && EventStore::SysEventDao::Update(sysEvent, false) != 0) {
        HIVIEW_LOGE("update failed, event: %{public}s", sysEvent->eventName_.c_str());
    }
}
}
}