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
#ifndef INTERFACES_INNER_API_UNIFIED_COLLECTION_UTILITY_TRACE_COLLECTOR_H
#define INTERFACES_INNER_API_UNIFIED_COLLECTION_UTILITY_TRACE_COLLECTOR_H
#include <cinttypes>
#include <memory>
#include <string>

#include "collect_result.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
class TraceCollector {
public:
    TraceCollector() = default;
    virtual ~TraceCollector() = default;

public:
    virtual CollectResult<std::string> DumpTrace(const std::string &tag) = 0;
    virtual CollectResult<int32_t> TraceOn() = 0;
    virtual CollectResult<std::string> TraceOff() = 0;
    static std::shared_ptr<TraceCollector> Create();
}; // TraceCollector
} // UCollectUtil
} // HiviewDFX
} // OHOS
#endif // INTERFACES_INNER_API_UNIFIED_COLLECTION_UTILITY_TRACE_COLLECTOR_H