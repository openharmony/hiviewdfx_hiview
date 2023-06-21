/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "base/raw_data_base_def.h"

#include <vector>

namespace OHOS {
namespace HiviewDFX {
namespace EventRaw {
namespace {
constexpr unsigned int DEFAULT_TZ_POS = 14; // default "+0000"

static std::vector<std::string> ALL_TIME_ZONES {
    "-0100", "-0200", "-0300", "-0330", "-0400", "-0500", "-0600",
    "-0700", "-0800", "-0900", "-0930", "-1000", "-1100", "-1200",
    "+0000", "+0100", "+0200", "+0300", "+0330", "+0400", "+0430",
    "+0500", "+0530", "+0545", "+0600", "+0630", "+0700", "+0800",
    "+0845", "+0900", "+0930", "+1000", "+1030", "+1100", "+1200",
    "+1245", "+1300", "+1400"
};
}
int ParseTimeZone(const std::string& tzStr)
{
    int ret = DEFAULT_TZ_POS;
    for (auto iter = ALL_TIME_ZONES.begin(); iter < ALL_TIME_ZONES.end(); ++iter) {
        if (*iter == tzStr) {
            ret = (iter - ALL_TIME_ZONES.begin());
            break;
        }
    }
    return ret;
}

std::string ParseTimeZone(const uint8_t tzVal)
{
    if (tzVal >= ALL_TIME_ZONES.size()) {
        return ALL_TIME_ZONES.at(DEFAULT_TZ_POS);
    }
    return ALL_TIME_ZONES.at(tzVal);
}

size_t GetValidDataMinimumByteCount()
{
    return sizeof(int32_t) + sizeof(struct EventRaw::HiSysEventHeader);
}
} // namespace EventRaw
} // namespace HiviewDFX
} // namespace OHOS