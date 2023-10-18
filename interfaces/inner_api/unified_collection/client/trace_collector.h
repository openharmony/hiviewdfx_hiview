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
#ifndef INTERFACES_INNER_API_UNIFIED_COLLECTION_CLIENT_TRACE_COLLECTOR_H
#define INTERFACES_INNER_API_UNIFIED_COLLECTION_CLIENT_TRACE_COLLECTOR_H
#include <cinttypes>
#include <memory>
#include <string>
#include <vector>

#include "collect_result.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectClient {
class TraceCollector {
public:
    TraceCollector() = default;
    virtual ~TraceCollector() = default;
    enum Caller {
        RELIABILITY,
        XPERF,
        XPOWER,
        BETACLUB,
        DEVELOP,
        OTHER,
    };

public:
    virtual CollectResult<int32_t> OpenSnapshot(const std::vector<std::string>& tagGroups) = 0;
    virtual CollectResult<std::vector<std::string>> DumpSnapshot(Caller caller = Caller::OTHER) = 0;
    virtual CollectResult<int32_t> OpenRecording(const std::string& tags) = 0;
    virtual CollectResult<int32_t> RecordingOn() = 0;
    virtual CollectResult<std::vector<std::string>> RecordingOff() = 0;
    virtual CollectResult<int32_t> Close() = 0;
    virtual CollectResult<int32_t> Recover() = 0;

    static std::shared_ptr<TraceCollector> Create();
}; // TraceCollector
} // UCollectClient
} // HiviewDFX
} // OHOS
#endif // INTERFACES_INNER_API_UNIFIED_COLLECTION_CLIENT_TRACE_COLLECTOR_H
