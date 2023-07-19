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
#ifndef UNIFIED_COLLECTION_XCOLLECTER_H
#define UNIFIED_COLLECTION_XCOLLECTER_H
#include <cinttypes>
#include <memory>
#include "collect_callback.h"
#include "collect_parameter.h"
#include "collect_task_result.h"

namespace OHOS {
namespace HiviewDFX {
class Xcollecter {
public:
    // 同步获取采集数据
    std::shared_ptr<CollectTaskResult> SubmitTask(std::shared_ptr<CollectParameter> collectParameter);
    // 异步获取采集的数据
    uint64_t SubmitTask(std::shared_ptr<CollectParameter> collectParameter, std::shared_ptr<CollectCallback> callback);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // UNIFIED_COLLECTION_XCOLLECTER_H
