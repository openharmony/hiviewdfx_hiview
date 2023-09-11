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
#include "trace_manager.h"
#include <iostream>

DEFINE_LOG_TAG("UCollectUtil");

using namespace OHOS::HiviewDFX::Hitrace;
namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int32_t ERR_CODE = -1;
}

int32_t TraceManager::OpenSnapshotTrace(const std::vector<std::string> &tagGroups)
{
    // check trace manager status
    if (status_ != TraceStatus::STOP) {
        OHOS::HiviewDFX::Hitrace::CloseTrace();
        status_ = TraceStatus::STOP;
    }
    if (OHOS::HiviewDFX::Hitrace::OpenTrace(tagGroups) != TraceErrorCode::SUCCESS) {
        HIVIEW_LOGE("Service OpenTrace fail.");
        return ERR_CODE;
    }
    status_ = TraceStatus::SNAPSHOT;
    return 0;
}

int32_t TraceManager::OpenRecordingTrace(const std::string &args)
{
    // check trace manager status
    if (status_ != TraceStatus::STOP) {
        OHOS::HiviewDFX::Hitrace::CloseTrace();
        status_ = TraceStatus::STOP;
    }
    if (OHOS::HiviewDFX::Hitrace::OpenTrace(args) != TraceErrorCode::SUCCESS) {
        HIVIEW_LOGE("CMD OpenTrace fail.");
        return ERR_CODE;
    }
    status_ = TraceStatus::RECORDING;
    return 0;
}

int32_t TraceManager::CloseTrace()
{
    if (OHOS::HiviewDFX::Hitrace::CloseTrace() != TraceErrorCode::SUCCESS) {
        HIVIEW_LOGE("CloseTrace fail.");
        return ERR_CODE;
    }
    status_ = TraceStatus::STOP;
    return 0;
}
} // HiviewDFX
} // OHOS