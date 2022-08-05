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

#include "running_status_log_util.h"

#include "running_status_logger.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr char LOG_DETAIL_CONCAT[] = " ";
constexpr char RULE_ITEM_CONCAT[] = ",";
}

void RunningStatusLogUtil::LogTooManyWatchRules(const std::vector<SysEventRule>& rules)
{
    std::string info = RunningStatusLogger::GetInstance().FormatTimeStamp();
    info.append(LOG_DETAIL_CONCAT).append("TOOMANYWATCHRULES ");
    info.append(std::to_string(rules.size())).append(LOG_DETAIL_CONCAT);
    info.append("RULES=[");
    for (auto& rule : rules) {
        info.append("{");
        info.append(rule.domain).append(RULE_ITEM_CONCAT);
        info.append(rule.eventName).append(RULE_ITEM_CONCAT);
        if (!rule.tag.empty()) {
            info.append(rule.tag).append(RULE_ITEM_CONCAT);
        }
        info.append(std::to_string(rule.ruleType));
        info.append("},");
    }
    info.erase(info.end() - 1);
    info.append("]");
    RunningStatusLogger::GetInstance().Log(info);
}

void RunningStatusLogUtil::LogTooManyWatchers()
{
    std::string info = RunningStatusLogger::GetInstance().FormatTimeStamp();
    info.append(LOG_DETAIL_CONCAT).append("TOOMANYWATCHERS COUNT > 30");
    RunningStatusLogger::GetInstance().Log(info);
}
} // namespace HiviewDFX
} // namespace OHOS