/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "log_analyzer.h"

#include <securec.h>

#include "common_utils.h"
#include "constants.h"
#include "faultlog_util.h"
#include "file_util.h"
#include "smart_parser.h"
#include "string_util.h"
#include "tbox.h"

namespace OHOS {
namespace HiviewDFX {
static void GetFingerRawString(std::string& fingerRawString, const FaultLogInfo& info,
                               std::map<std::string, std::string>& eventInfos)
{
    if (info.reason.compare("SERVICE_BLOCK") == 0) {
        static uint64_t serviceBlockNum = 0;
        fingerRawString = std::to_string(++serviceBlockNum);
        return;
    }

    if (info.reason.compare("SERVICE_TIMEOUT") == 0) {
        auto it = info.sectionMap.find("MODULE_NAME");
        if (it != info.sectionMap.end()) {
            fingerRawString = it->second;
            return;
        }
    }

    if (info.reason.compare("APP_HICOLLIE") == 0) {
        if (!eventInfos["TIME_OUT"].empty()) {
            fingerRawString = eventInfos["TIME_OUT"];
            return;
        }
    }

    auto eventType = GetFaultNameByType(info.faultLogType, false);
    fingerRawString = info.module + StringUtil::GetLeftSubstr(info.reason, "@") +
        eventInfos["FIRST_FRAME"] + eventInfos["SECOND_FRAME"] + eventInfos["LAST_FRAME"] +
        ((eventType == "JS_ERROR") ? eventInfos["SUBREASON"] : "");
}

bool AnalysisFaultlog(const FaultLogInfo& info, std::map<std::string, std::string>& eventInfos)
{
    bool needDelete = false;
    std::string logPath = info.logPath;
    auto eventType = GetFaultNameByType(info.faultLogType, false);
    if ((eventType == "JS_ERROR" || eventType == "CPP_CRASH") &&
        !FileUtil::FileExists(info.logPath) && !info.summary.empty()) {
        logPath = std::string(FaultLogger::FAULTLOG_BASE_FOLDER) + eventType + std::to_string(info.time);
        FileUtil::SaveStringToFile(logPath, info.summary);
        needDelete = true;
    }

    eventInfos = SmartParser::Analysis(logPath, SMART_PARSER_PATH, eventType);
    if (needDelete) {
        FileUtil::RemoveFile(logPath);
    }
    if (eventInfos.empty()) {
        eventInfos.insert(std::make_pair("fingerPrint", Tbox::CalcFingerPrint(info.module + info.reason +
            info.summary, 0, FP_BUFFER)));
        return false;
    }
    Tbox::FilterTrace(eventInfos, eventType);
    std::string fingerRawString;
    GetFingerRawString(fingerRawString, info, eventInfos);
    eventInfos["fingerPrint"] = Tbox::CalcFingerPrint(fingerRawString, 0, FP_BUFFER);

    if (eventType == "APP_FREEZE" && !(eventInfos["TRACER_PID"].empty())) {
        int32_t pid = 0;
        if (sscanf_s(eventInfos["TRACER_PID"].c_str(), "%d", &pid) == 1 && pid > 0) {
            eventInfos["LAST_FRAME"] += ("(Tracer Process Name:" + CommonUtils::GetProcNameByPid(pid) + ")");
        }
    }

    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
