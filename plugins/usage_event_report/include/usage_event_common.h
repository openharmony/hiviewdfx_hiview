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

#ifndef HIVIEW_PLUGINS_USAGE_EVENT_REPORT_SREVICE_USAGE_EVENT_COMMON_H
#define HIVIEW_PLUGINS_USAGE_EVENT_REPORT_SREVICE_USAGE_EVENT_COMMON_H

#include <string>

#include "hiview_event_common.h"

namespace OHOS {
namespace HiviewDFX {
namespace AppUsageEventSpace {
const std::string EVENT_NAME = "APP_USAGE";
const std::string KEY_OF_PACKAGE = "PACKAGE";
const std::string KEY_OF_VERSION = "VERSION";
const std::string KEY_OF_USAGE = "USAGE";
const std::string KEY_OF_DATE = "DATE";
}

namespace SysUsageEventSpace {
const std::string EVENT_NAME = "SYS_USAGE";
const std::string KEY_OF_START = "START";
const std::string KEY_OF_END = "END";
const std::string KEY_OF_POWER = "POWER";
const std::string KEY_OF_SCREEN = "SCREEN";
const std::string KEY_OF_RUNNING = "RUNNING";
}

namespace SysUsageDbSpace {
const std::string SYS_USAGE_COLL = "sys_usage";
const std::string LAST_SYS_USAGE_COLL = "last_sys_usage";
}
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_USAGE_EVENT_REPORT_SREVICE_USAGE_EVENT_COMMON_H
