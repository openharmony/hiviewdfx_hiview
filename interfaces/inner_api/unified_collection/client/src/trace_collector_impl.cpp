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
#include "trace_collector.h"

using namespace OHOS::HiviewDFX::UCollect;

namespace OHOS {
namespace HiviewDFX {
namespace UCollectClient {
class TraceCollectorImpl : public TraceCollector {
public:
    TraceCollectorImpl() = default;
    virtual ~TraceCollectorImpl() = default;

public:
    virtual CollectResult<std::vector<std::string>> DumpTrace(TraceCollector::Caller &caller) override;
    virtual CollectResult<int32_t> TraceOn() override;
    virtual CollectResult<std::vector<std::string>> TraceOff() override;
};

std::shared_ptr<TraceCollector> TraceCollector::Create()
{
    return std::make_shared<TraceCollectorImpl>();
}

CollectResult<std::vector<std::string>> TraceCollectorImpl::DumpTrace(TraceCollector::Caller &caller)
{
    CollectResult<std::vector<std::string>> result;
    return result;
}

CollectResult<int32_t> TraceCollectorImpl::TraceOn()
{
    CollectResult<int32_t> result;
    return result;
}

CollectResult<std::vector<std::string>> TraceCollectorImpl::TraceOff()
{
    CollectResult<std::vector<std::string>> result;
    return result;
}
} // UCollectClient
} // HiViewDFX
} // OHOS
