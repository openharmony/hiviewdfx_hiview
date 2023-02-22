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

#include "reporter.h"
#include "hisysevent.h"
#include "sanitizerd_log.h"
#include "zip_helper.h"
#include "logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("Faultlogger");

int Init(SanitizerdType type)
{
    switch (type) {
        case ASAN_LOG_RPT:
        case UBSAN_LOG_RPT:
        case KASAN_LOG_RPT:
        case LSAN_LOG_RPT:
            break;
        default:
            return -1;
    }

    return 0;
}

void Upload(T_SANITIZERD_PARAMS *params)
{
    if (!params || Init(params->type) < 0) {
        HIVIEW_LOGI("Error Params...");
        return;
    }

    WriteCollectedData(params);
    return;
}
} // namespace HiviewDFX
} // namespace OHOS
