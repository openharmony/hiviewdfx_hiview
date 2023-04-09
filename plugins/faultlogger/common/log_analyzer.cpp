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
#include "log_analyzer.h"

#include "faultlog_util.h"
#include "file_util.h"
#include "smart_parser.h"
#include "string_util.h"
#include "tbox.h"

namespace OHOS {
namespace HiviewDFX {
bool AnalysisFaultlog(const FaultLogInfo& info, std::map<std::string, std::string>& eventInfos)
{
    std::string fingerPrint;
    bool needDelete = false;
    std::string logPath = info.logPath;
    auto eventType = GetFaultNameByType(info.faultLogType, false);
    if (eventType == "JS_ERROR" && !FileUtil::FileExists(info.logPath) && !info.summary.empty()) {
        logPath = info.logPath + "tmp";
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
    fingerPrint = Tbox::CalcFingerPrint(info.module + StringUtil::GetLeftSubstr(info.reason, "@") +
        eventInfos["FIRST_FRAME"] + eventInfos["SECOND_FRAME"] + eventInfos["LAST_FRAME"] +
        ((eventType == "JS_ERROR") ? eventInfos["SUBREASON"] : ""), 0, FP_BUFFER);
    eventInfos["fingerPrint"] = fingerPrint;

    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
