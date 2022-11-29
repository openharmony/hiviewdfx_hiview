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
#include "smart_parser.h"
#include "tbox.h"

namespace OHOS {
namespace HiviewDFX {
bool AnalysisFaultlog(const FaultLogInfo& info, std::map<std::string, std::string>& eventInfos)
{
    auto eventType = GetFaultNameByType(info.faultLogType, false);
    eventInfos = SmartParser::Analysis(info.logPath, SMART_PARSER_PATH, eventType);
    if (eventInfos.empty()) {
        eventInfos.insert(std::make_pair("fingerPrint", Tbox::CalcFingerPrint(info.module + info.reason +
                                                                              info.summary, 0, FP_BUFFER)));
        return false;
    }

    Tbox::FilterTrace(eventInfos);
    std::string fingerPrint = Tbox::CalcFingerPrint(info.module + info.reason + eventInfos["FIRST_FRAME"] +
        eventInfos["SECOND_FRAME"] + eventInfos["LAST_FRAME"], 0, FP_BUFFER);
    eventInfos["fingerPrint"] = fingerPrint;
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
