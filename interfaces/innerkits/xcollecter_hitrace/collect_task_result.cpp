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
#include "collect_item_result.h"
#include "collect_task_result.h"

namespace OHOS {
namespace HiviewDFX {
void CollectTaskResult::Add(std::shared_ptr<CollectItemResult> value)
{
    values_.emplace_back(value);
}

std::shared_ptr<CollectItemResult> CollectTaskResult::Next()
{
    if (values_.empty()) {
        return nullptr;
    }

    if (cur_ >= values_.size()) {
        return nullptr;
    }

    return values_[cur_++];
}
} // namespace HiviewDFX
} // namespace OHOS
