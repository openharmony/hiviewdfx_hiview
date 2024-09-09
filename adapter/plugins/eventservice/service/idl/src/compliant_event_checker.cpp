/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "compliant_event_checker.h"

#include <list>
#include <unordered_map>

#include "hiview_logger.h"
#include "parameter_ex.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
DEFINE_LOG_TAG("CompliantEventChecker");

constexpr int64_t SECURE_DISBALED_VAL = 0;
constexpr int64_t SECURE_ENABALED_VAL = 1;

std::unordered_map<std::string, std::list<std::string>> COMPLIANT_EVENT_CONFIGS {
    {"AAFWK", { "START_ABILITY", "ABILITY_ONFOREGROUND", "ABILITY_ONBACKGROUND", "ABILITY_ONACTIVE",
        "ABILITY_ONINACTIVE", "TERMINATE_ABILITY"}},
    {"ACE", {"INTERACTION_COMPLETED_LATENCY"}},
    {"AV_CODEC", {"CODEC_START_INFO"}},
    {"GRAPHIC", {"INTERACTION_COMPLETED_LATENCY"}},
    {"LOCATION", {"GNSS_STATE"}},
    {"PERFORMANCE", {}},
    {"RELIBILITY", {}},
    {"STABILITY", {"JS_ERROR", "CPP_CRASH", "APP_FREEZE", "MEMORY_LEAK", "FD_LEAK", "THREAD_LEAK", "ADDR_SANITIZER"}},
    {"WORKSCHEDULER", {"WORK_ADD", "WORK_REMOVE", "WORK_START", "WORK_STOP"}},
};
}

CompliantEventChecker::CompliantEventChecker()
{
    secureVal_ = Parameter::GetInteger("const.secure", SECURE_ENABALED_VAL);
    HIVIEW_LOGD("value of const.secure is %{public}" PRId64 "", secureVal_);
}

bool CompliantEventChecker::IsInCompliantEvent(const std::string& domain, const std::string& eventName)
{
    if (secureVal_ == SECURE_DISBALED_VAL) {
        return false;
    }
    for (const auto& compliantConfig : COMPLIANT_EVENT_CONFIGS) {
        if (compliantConfig.first != domain) {
            continue;
        }
        if (compliantConfig.second.empty()) {
            HIVIEW_LOGD("event with domain [%{public}s] is compliant", domain.c_str());
            return false;
        }
        auto findRet = std::find(compliantConfig.second.begin(), compliantConfig.second.end(), eventName);
        if (findRet != compliantConfig.second.end()) {
            HIVIEW_LOGD("event [%{public}s|%{public}s] is compliant", domain.c_str(), eventName.c_str());
            return false;
        }
        return true;
    }
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS