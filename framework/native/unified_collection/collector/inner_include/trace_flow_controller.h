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
#ifndef FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_TRACE_FLOW_CONTROLLER_H
#define FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_TRACE_FLOW_CONTROLLER_H

#include <memory>
#include <string>

#include "hitrace_dump.h"
#include "trace_collector.h"
#include "trace_storage.h"

using OHOS::HiviewDFX::Hitrace::TraceErrorCode;
using OHOS::HiviewDFX::Hitrace::TraceRetInfo;
using OHOS::HiviewDFX::UCollectUtil::TraceCollector;
using OHOS::HiviewDFX::UCollect::UcError;

namespace OHOS {
namespace HiviewDFX {
class TraceFlowController {
public:
    TraceFlowController();
    ~TraceFlowController() = default;
    bool NeedDump(TraceCollector::Caller &caller);
    bool NeedUpload(TraceCollector::Caller &caller, TraceRetInfo ret);
    void StoreDb();

private:
    void InitTraceDb();
    void InitTraceStorage();
    std::string GetDate();
    int64_t GetTraceSize(Hitrace::TraceRetInfo ret);
    bool IsLowerLimit(int64_t nowSize, int64_t traceSize, int64_t limitSize);
    UcollectionTraceStorage QueryDb();

private:
    struct UcollectionTraceStorage ucollectionTraceStorage_;
    std::shared_ptr<TraceStorage> traceStorage_;
};
} // HiViewDFX
} // OHOS
#endif // FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_TRACE_FLOW_CONTROLLER_H