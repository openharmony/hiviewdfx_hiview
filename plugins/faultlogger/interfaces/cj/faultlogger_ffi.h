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

#ifndef OHOS_FAULTLOGGER_FFI_H
#define OHOS_FAULTLOGGER_FFI_H

#include <cstdint>
#include "cj_common_ffi.h"
#include "ffi_remote_data.h"

constexpr int ERR_PARAM = 401;
constexpr int ERR_SERVICE_STATUS = 10600001;
constexpr int ERR_SYSTEM_CAPABILITY = 801;

extern "C" {
    struct CFaultLogInfo {
        int32_t pid = 0;
        int32_t uid = 0;
        int32_t faultLogType = 0;
        int64_t timestamp = 0;
        char* module;
        char* reason;
        char* summary;
        char* fullLog;
    };

    struct CArrFaultLogInfo {
        CFaultLogInfo* head;
        int64_t size;
    };

    FFI_EXPORT CArrFaultLogInfo FfiFaultLoggerQuery(int32_t faultType, int32_t &code);
}

#endif