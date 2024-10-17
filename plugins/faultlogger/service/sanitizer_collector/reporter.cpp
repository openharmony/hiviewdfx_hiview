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

#include "hisysevent.h"
#include "hiview_logger.h"
#include "reporter.h"
#include "sanitizerd_log.h"
#include "zip_helper.h"

namespace OHOS {
namespace HiviewDFX {

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
        HILOG_ERROR(LOG_CORE, "Error upload Params.");
        return;
    }

    WriteCollectedData(params);
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::RELIABILITY, "ADDR_SANITIZER",
                    OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
                    "MODULE", params->procName,
                    "VERSION", params->appVersion,
                    "REASON", params->errType,
                    "PID", params->pid,
                    "UID", params->uid,
                    "SUMMARY", params->description,
                    "FAULT_TYPE", params->errType,
                    "LOG_NAME", params->logName,
                    "FINGERPRINT", params->hash,
                    "HAPPEN_TIME", params->happenTime);
    return;
}
} // namespace HiviewDFX
} // namespace OHOS
