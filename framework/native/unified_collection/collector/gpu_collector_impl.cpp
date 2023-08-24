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
#include "common_util.h"
#include "logger.h"
#include "file_util.h"

DEFINE_LOG_TAG("UCollectUtil");

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
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

inline int32_t getValue(std::string filename)
{
    std::string content;
    FileUtil::LoadStringFromFile(filename, content);
    return std::stoi(content);
}

CollectResult<GpuFreq> GpuCollectorImpl::CollectGpuFrequency()
{
    CollectResult<GpuFreq> result;
    GpuFreq& gpuFreq = result.data;
    gpuFreq.curFeq = getValue(GPU_CUR_FREQ);
    HIVIEW_LOGD("curFeq=%{public}d", gpuFreq.curFeq);
    gpuFreq.maxFeq = getValue(GPU_MAX_FREQ);
    HIVIEW_LOGD("maxFeq=%{public}d", gpuFreq.maxFeq);
    gpuFreq.minFeq = getValue(GPU_MIN_FREQ);
    HIVIEW_LOGD("minFeq=%{public}d", gpuFreq.minFeq);
    result.retCode = UcError::SUCCESS;
    return result;
}

CollectResult<SysGpuLoad> GpuCollectorImpl::CollectSysGpuLoad()
{
    CollectResult<SysGpuLoad> result;
    SysGpuLoad& sysGpuLoad = result.data;
    sysGpuLoad.gpuLoad = getValue(GPU_LOAD);
    HIVIEW_LOGD("gpuLoad=%{public}f", sysGpuLoad.gpuLoad);
    result.retCode = UcError::SUCCESS;
    return result;
}
} // UCollectUtil
} // HiViewDFX
} // OHOS