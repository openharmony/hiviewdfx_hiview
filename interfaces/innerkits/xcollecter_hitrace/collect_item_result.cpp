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
#include <string>
#include <iostream>
#include "collect_item_result.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
CollectItemResult::CollectItemResult()
{}

CollectItemResult::~CollectItemResult()
{
    itemValues_.clear();
}

void CollectItemResult::SetCollectItemValue(const std::string& item, std::string &value)
{
    if (item.rfind("/hitrace/") != 0) {
        return;
    }
    CollectItemValue ciValue = CollectItemValue();
    ciValue.strValue_ = value;
    itemValues_.emplace(item, ciValue);
}

void CollectItemResult::GetCollectItemValue(const std::string& item, std::string &value)
{
    auto it = itemValues_.find(item);
    if (it == itemValues_.end()) {
        value.clear();
        return;
    }

    value = it->second.strValue_;
}

void CollectItemResult::GetCollectItemValue(const std::string& item, std::string &value, std::string &unit)
{
    auto it = itemValues_.find(item);
    if (it == itemValues_.end()) {
        value.clear();
        unit.clear();
        return;
    }

    value = it->second.strValue_;
    unit = it->second.unit_;
}

void CollectItemResult::GetCollectItemValue(const std::string& item, int32_t &value)
{
    auto it = itemValues_.find(item);
    if (it == itemValues_.end()) {
        value = 0;
        return;
    }

    value = it->second.intValue_;
}

void CollectItemResult::GetCollectItemValue(const std::string& item, int32_t &value, std::string &unit)
{
    auto it = itemValues_.find(item);
    if (it == itemValues_.end()) {
        value = 0;
        unit.clear();
        return;
    }

    value = it->second.intValue_;
    unit = it->second.unit_;
}
} // namespace HiviewDFX
} // namespace OHOS
