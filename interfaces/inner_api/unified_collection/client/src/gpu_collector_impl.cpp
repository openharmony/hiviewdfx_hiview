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
#include "gpu_collector.h"

using namespace OHOS::HiviewDFX::UCollect;

namespace OHOS {
namespace HiviewDFX {
namespace UCollectClient {
class GpuCollectorImpl : public GpuCollector {
public:
    GpuCollectorImpl() = default;
    virtual ~GpuCollectorImpl() = default;

public:
    virtual CollectResult<GpuFreq> CollectGpuFrequency() override;
    virtual CollectResult<SysGpuLoad> CollectSysGpuLoad() override;
};

std::shared_ptr<GpuCollector> GpuCollector::Create()
{
    return std::make_shared<GpuCollectorImpl>();
}

CollectResult<GpuFreq> GpuCollectorImpl::CollectGpuFrequency()
{
    CollectResult<GpuFreq> result;
    return result;
}

CollectResult<SysGpuLoad> GpuCollectorImpl::CollectSysGpuLoad()
{
    CollectResult<SysGpuLoad> result;
    return result;
}
} // UCollectClient
} // HiViewDFX
} // OHOS