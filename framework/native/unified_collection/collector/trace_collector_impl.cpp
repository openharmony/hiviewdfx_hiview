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
#include <mutex>

#include "logger.h"
#include "trace_collector.h"
#include "trace_manager.h"
#include "trace_utils.h"

using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::Hitrace;
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace OHOS::HiviewDFX::UCollect;

DEFINE_LOG_TAG("UCollectUtil");

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
namespace {
std::mutex g_dumpTraceMutex;
}

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
    std::lock_guard<std::mutex> lock(g_dumpTraceMutex);
    CollectResult<std::vector<std::string>> result;
    TraceRetInfo ret = OHOS::HiviewDFX::Hitrace::DumpTrace();
    if (ret.errorCode == TraceErrorCode::SUCCESS) {
        if (caller == TraceCollector::Caller::DEVELOP) {
            result.data = ret.outputFiles;
        } else {
            std::vector<std::string> outputFiles = GetUnifiedFiles(ret, caller);
            result.data = outputFiles;
        }
    }
    result.retCode = TransCodeToUcError(ret.errorCode);
    HIVIEW_LOGI("DumpTrace, ret = %{public}d, data.size = %{public}d.", result.retCode, result.data.size());
    return result;
}

CollectResult<int32_t> TraceCollectorImpl::TraceOn()
{
    CollectResult<int32_t> result;
    TraceErrorCode ret = OHOS::HiviewDFX::Hitrace::DumpTraceOn();
    result.retCode = TransCodeToUcError(ret);
    HIVIEW_LOGI("TraceOn, ret = %{public}d.", result.retCode);
    return result;
}

CollectResult<std::vector<std::string>> TraceCollectorImpl::TraceOff()
{
    CollectResult<std::vector<std::string>> result;
    TraceRetInfo ret = OHOS::HiviewDFX::Hitrace::DumpTraceOff();
    if (ret.errorCode == TraceErrorCode::SUCCESS) {
        result.data = ret.outputFiles;
    }
    result.retCode = TransCodeToUcError(ret.errorCode);
    HIVIEW_LOGI("TraceOff, ret = %{public}d, data.size = %{public}d.", result.retCode, result.data.size());
    return result;
}
} // UCollectUtil
} // HiViewDFX
} // OHOS
