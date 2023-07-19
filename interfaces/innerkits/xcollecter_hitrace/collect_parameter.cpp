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
#include <algorithm>
#include "collect_parameter.h"

namespace OHOS {
namespace HiviewDFX {
CollectParameter::CollectParameter(const std::string &caller): caller_(caller)
{}

CollectParameter::~CollectParameter()
{
    items_.clear();
    itemIntConfigs_.clear();
    itemStrConfigs_.clear();
}

void CollectParameter::SetRuntime(const std::string &second, const std::string &minute, const std::string &hour)
{
    second_ = second;
    minute_ = minute;
    hour_ = hour;
}

void CollectParameter::SetExecTimes(uint32_t times)
{
    times_ = times;
}

void CollectParameter::SetCollectItems(const std::vector<std::string> &items)
{
    for (auto it = items.begin(); it != items.end(); it++) {
        auto itFinded = std::find(items_.begin(), items_.end(), *it);
        if (itFinded != items_.end()) {
            continue;
        }

        items_.emplace_back(*it);
    }
}

void CollectParameter::SetConfig(const std::string &collectModule, const std::string &config, const std::string value)
{
    auto moduleConfig = itemStrConfigs_.find(collectModule);
    if (moduleConfig == itemStrConfigs_.end()) {
        itemStrConfigs_.insert(std::make_pair(collectModule, std::map<std::string, std::string>()));
        moduleConfig = itemStrConfigs_.find(collectModule);
    }
    moduleConfig->second.insert(std::make_pair(config, value));
}

void CollectParameter::SetConfig(const std::string &collectModule, const std::string &config, uint32_t value)
{
    auto moduleConfig = itemIntConfigs_.find(collectModule);
    if (moduleConfig == itemIntConfigs_.end()) {
        itemIntConfigs_.insert(std::make_pair(collectModule, std::map<std::string, uint32_t>()));
        moduleConfig = itemIntConfigs_.find(collectModule);
    }
    moduleConfig->second.insert(std::make_pair(config, value));
}
} // namespace HiviewDFX
} // namespace OHOS
