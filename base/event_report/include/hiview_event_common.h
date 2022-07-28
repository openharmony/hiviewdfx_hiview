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

#ifndef HIVIEW_BASE_EVENT_REPORT_HIVIEW_EVENT_COMMON_H
#define HIVIEW_BASE_EVENT_REPORT_HIVIEW_EVENT_COMMON_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
constexpr unsigned int LABEL_DOMAIN = 0xD002D10;
const std::string EVENT_DOMAIN = "HIVIEWDFX";

namespace BaseEventSpace {
const std::string KEY_OF_DOMAIN = "domain_";
const std::string KEY_OF_NAME = "name_";
const std::string KEY_OF_TYPE = "type_";
}

namespace PluginEventSpace {
const std::string LOAD_EVENT_NAME = "PLUGIN_LOAD";
const std::string UNLOAD_EVENT_NAME = "PLUGIN_UNLOAD";
const std::string KEY_OF_PLUGIN_NAME = "NAME";
const std::string KEY_OF_RESULT = "RESULT";
constexpr uint32_t LOAD_SUCCESS = 0;
constexpr uint32_t LOAD_DUPLICATE_NAME = 1;
constexpr uint32_t LOAD_UNREGISTERED = 2;
constexpr uint32_t UNLOAD_SUCCESS = 0;
constexpr uint32_t UNLOAD_INVALID = 1;
constexpr uint32_t UNLOAD_NOT_FOUND = 2;
constexpr uint32_t UNLOAD_IN_USE = 3;
}

namespace PluginFaultEventSpace {
const std::string EVENT_NAME = "PLUGIN_FAULT";
const std::string KEY_OF_PLUGIN_NAME = "NAME";
const std::string KEY_OF_REASON = "REASON";
}

namespace PluginStatsEventSpace {
const std::string EVENT_NAME = "PLUGIN_STATS";
const std::string KEY_OF_PLUGIN_NAME = "NAME";
const std::string KEY_OF_AVG_TIME = "AVG_TIME";
const std::string KEY_OF_TOP_K_TIME = "TOP_K_TIME";
const std::string KEY_OF_TOP_K_EVENT = "TOP_K_EVENT";
const std::string KEY_OF_TOTAL = "TOTAL";
const std::string KEY_OF_PROC_NAME = "PROC_NAME";
const std::string KEY_OF_PROC_TIME = "PROC_TIME";
const std::string KEY_OF_TOTAL_TIME = "TOTAL_TIME";
}
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_BASE_EVENT_REPORT_HIVIEW_EVENT_COMMON_H
