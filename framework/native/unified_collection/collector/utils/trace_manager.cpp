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
#include <map>
#include <mutex>

#include "hilog/log.h"
#include "hitrace_dump.h"
#include "logger.h"
#include "parameter_ex.h"
#include "trace_collector.h"
#include "trace_manager.h"
#include "trace_utils.h"

DEFINE_LOG_TAG("UCollectUtil-TraceCollector");

using OHOS::HiviewDFX::Hitrace::TraceErrorCode;
using OHOS::HiviewDFX::UCollect::UcError;

using OHOS::HiviewDFX::Hitrace::TraceErrorCode;
using OHOS::HiviewDFX::UCollect::UcError;

namespace OHOS {
namespace HiviewDFX {
namespace {
std::mutex g_traceLock;
}

int32_t TraceManager::OpenSnapshotTrace(const std::vector<std::string> &tagGroups)
{
    std::lock_guard<std::mutex> lock(g_traceLock);
    HIVIEW_LOGI("start to open snapshot trace.");
    // service mode
    if (OHOS::HiviewDFX::Hitrace::GetTraceMode() ==
        OHOS::HiviewDFX::Hitrace::TraceMode::SERVICE_MODE) {
        HIVIEW_LOGE("now is snapshot status, open snapshot failed.");
        return UcError::TRACE_CALL_ERROR;
    }

    // recording mode
    if (OHOS::HiviewDFX::Hitrace::GetTraceMode() ==
        OHOS::HiviewDFX::Hitrace::TraceMode::CMD_MODE) {
        HIVIEW_LOGE("now is recording status, open snapshot failed.");
        return UcError::TRACE_IS_OCCUPIED;
    }

    TraceErrorCode ret = OHOS::HiviewDFX::Hitrace::OpenTrace(tagGroups);
    return TransCodeToUcError(ret);
}

int32_t TraceManager::OpenRecordingTrace(const std::string &args)
{
    std::lock_guard<std::mutex> lock(g_traceLock);
    HIVIEW_LOGI("start to open recording trace.");
    // recording mode
    if (OHOS::HiviewDFX::Hitrace::GetTraceMode() ==
        OHOS::HiviewDFX::Hitrace::TraceMode::CMD_MODE) {
        HIVIEW_LOGE("now is recording status, open recording failed.");
        return UcError::TRACE_IS_OCCUPIED;
    }

    // service mode
    if (OHOS::HiviewDFX::Hitrace::GetTraceMode() ==
        OHOS::HiviewDFX::Hitrace::TraceMode::SERVICE_MODE) {
        HIVIEW_LOGI("TraceMode is switching: snapshot close, recording open.");
        OHOS::HiviewDFX::Hitrace::CloseTrace();
    }

    TraceErrorCode ret = OHOS::HiviewDFX::Hitrace::OpenTrace(args);
    return TransCodeToUcError(ret);
}

int32_t TraceManager::CloseTrace()
{
    std::lock_guard<std::mutex> lock(g_traceLock);
    HIVIEW_LOGI("start to close trace.");
    TraceErrorCode ret = OHOS::HiviewDFX::Hitrace::CloseTrace();
    return TransCodeToUcError(ret);
}

int32_t TraceManager::RecoverTrace()
{
    std::lock_guard<std::mutex> lock(g_traceLock);
    HIVIEW_LOGI("start to recover trace.");
    TraceErrorCode ret = OHOS::HiviewDFX::Hitrace::CloseTrace();

    if (Parameter::IsBetaVersion()) {
        HIVIEW_LOGI("recover trace to Snapshot.");
        const std::vector<std::string> tagGroups = {"scene_performance"};
        TraceErrorCode ret = OHOS::HiviewDFX::Hitrace::OpenTrace(tagGroups);
        return TransCodeToUcError(ret);
    }
    HIVIEW_LOGI("recover trace to close.");

    return TransCodeToUcError(ret);
}
} // HiviewDFX
} // OHOS
