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

#include "monitor_config.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

#include "logger.h"
#include "string_util.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-MonitorCfg");
bool MonitorConfig::Parse()
{
    std::ifstream in;
    in.open(configPath_);
    if (!in.is_open()) {
        HIVIEW_LOGW("fail to open monitor config file.");
        return false;
    }

    std::string buf = "";
    const int configItemField = 1;
    const int configValueField = 2;
    while (getline(in, buf)) {
        std::string strTmp = StringUtil::TrimStr(buf);
        if (strTmp.empty()) {
            continue;
        }
        std::smatch result;
        if (!regex_search(strTmp, result, std::regex("(collectPeriod|reportPeriod|totalSizeBenchMark|"
            "realTimeBenchMark|processTimeBenchMark)\\s*=\\s*(\\d+)"))) {
            HIVIEW_LOGW("match field failed %{public}s", strTmp.c_str());
            continue;
        }

        std::string configItem = StringUtil::TrimStr(result[configItemField]);
        uint32_t configValue = static_cast<uint32_t>(atol(std::string(result[configValueField]).c_str()));
        configs_[configItem] = configValue;
        HIVIEW_LOGD("config=%{public}s value=%{public}u", configItem.c_str(), configValue);
    }
    in.close();
    return true;
}

void MonitorConfig::ReadParam(const std::string &name, uint32_t &value)
{
    auto it = configs_.find(name);
    if (it == configs_.end()) {
        return;
    }
    if (it->second == 0) {
        return;
    }
    value = it->second;
}
} // namespace HiviewDFX
} // namespace OHOS