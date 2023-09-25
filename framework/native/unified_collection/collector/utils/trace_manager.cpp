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
#include "hilog/log.h"
#include "hitrace_dump.h"
#include "logger.h"
#include "parameter_ex.h"
#include "trace_manager.h"

#include <mutex>

DEFINE_LOG_TAG("UCollectUtil");

namespace OHOS {
namespace HiviewDFX {
namespace {
    std::mutex lock_;
}

int32_t TraceManager::OpenSnapshotTrace(const std::vector<std::string> &tagGroups)
{
    std::lock_guard<std::mutex> lock(lock_);
    // service mode
    if (OHOS::HiviewDFX::Hitrace::GetTraceMode() ==
        OHOS::HiviewDFX::Hitrace::TraceMode::SERVICE_MODE) {
        HIVIEW_LOGE("Service status.");
        return OHOS::HiviewDFX::Hitrace::TraceErrorCode::CALL_ERROR;
    }

    // recording mode
    if (OHOS::HiviewDFX::Hitrace::GetTraceMode() ==
        OHOS::HiviewDFX::Hitrace::TraceMode::CMD_MODE) {
        HIVIEW_LOGE("Recording status, open snapshot failed.");
        return OHOS::HiviewDFX::Hitrace::TraceErrorCode::TRACE_IS_OCCUPIED;
    }

    return OHOS::HiviewDFX::Hitrace::OpenTrace(tagGroups);
}

int32_t TraceManager::OpenRecordingTrace(const std::string &args)
{
    std::lock_guard<std::mutex> lock(lock_);
    // recording mode
    if (OHOS::HiviewDFX::Hitrace::GetTraceMode() ==
        OHOS::HiviewDFX::Hitrace::TraceMode::CMD_MODE) {
        HIVIEW_LOGE("Recording status, open recording failed.");
        return OHOS::HiviewDFX::Hitrace::TraceErrorCode::TRACE_IS_OCCUPIED;
    }

    // service mode
    if (OHOS::HiviewDFX::Hitrace::GetTraceMode() ==
        OHOS::HiviewDFX::Hitrace::TraceMode::SERVICE_MODE) {
        HIVIEW_LOGI("TraceMode is switching: snapshot close, open recording.");
        OHOS::HiviewDFX::Hitrace::CloseTrace();
    }

    return OHOS::HiviewDFX::Hitrace::OpenTrace(args);
}

int32_t TraceManager::CloseTrace()
{
    std::lock_guard<std::mutex> lock(lock_);
    HIVIEW_LOGI("TraceManager: close trace.");
    return OHOS::HiviewDFX::Hitrace::CloseTrace();
}

int32_t TraceManager::RecoverTrace()
{
    std::lock_guard<std::mutex> lock(lock_);
    int32_t ret = OHOS::HiviewDFX::Hitrace::CloseTrace();

    if (Parameter::IsBetaVersion()) {
        HIVIEW_LOGI("TraceManager: RecoverTrace to Snapshot.");
        const std::vector<std::string> tagGroups = {"scene_performance"};
        return OHOS::HiviewDFX::Hitrace::OpenTrace(tagGroups);
    }

    HIVIEW_LOGI("TraceManager: RecoverTrace to Close.");
    return ret;
}
} // HiviewDFX
} // OHOS
