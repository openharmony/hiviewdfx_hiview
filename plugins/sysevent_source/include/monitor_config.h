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

#ifndef HIVIEW_CORE_MONITOR_CONFIG_H
#define HIVIEW_CORE_MONITOR_CONFIG_H
#include <cstdint>
#include <map>
#include <string>

namespace OHOS {
namespace HiviewDFX {
class MonitorConfig {
public:
    explicit MonitorConfig(const std::string& configPath): configPath_(configPath) {}
    ~MonitorConfig() {}

    bool Parse();
    void ReadParam(const std::string &name, uint32_t &value);
private:
    std::string configPath_;
    std::map<std::string, uint32_t> configs_;
}; // MonitorConfig
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_CORE_MONITOR_CONFIG_H