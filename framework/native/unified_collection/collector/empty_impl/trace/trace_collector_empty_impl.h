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

#ifndef HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_TRACE_COLLECTOR_EMPTY_IMPL_H
#define HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_TRACE_COLLECTOR_EMPTY_IMPL_H

#include "trace_collector.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
class TraceCollectorEmptyImpl : public TraceCollector {
public:
    CollectResult<std::vector<std::string>> DumpTrace(UCollect::TraceCaller caller) override;
    CollectResult<std::vector<std::string>> DumpTraceWithDuration(
            UCollect::TraceCaller caller, uint32_t timeLimit, uint64_t happenTime) override;
    CollectResult<std::vector<std::string>> DumpTraceWithFilter(UCollect::TeleModule module,
        uint32_t timeLimit, uint64_t happenTime) override;
    CollectResult<int32_t> FilterTraceOn(UCollect::TeleModule module, uint64_t postTime) override;
    CollectResult<int32_t> FilterTraceOff(UCollect::TeleModule module) override;
    void RecoverTmpTrace() override;
};
} // namespace UCollectUtil
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_TRACE_COLLECTOR_EMPTY_IMPL_H
