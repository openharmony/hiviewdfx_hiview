/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "uc_native_process_observer.h"

#include "common_util.h"
#include "hiview_logger.h"
#include "process_status.h"
#include "string_util.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-UnifiedCollector");
using namespace OHOS::HiviewDFX::UCollectUtil;
namespace {
const std::string PID_SUFFIX = ".pid";
}

void UcNativeProcessObserver::OnParamChanged(const char* key, const char* value, void* context)
{
    if (key == nullptr || value == nullptr) {
        return;
    }

    int32_t pid = 0;
    if (!CommonUtil::EndWith(key, PID_SUFFIX) || !StringUtil::StrToInt(value, pid) || pid <= 0) {
        return;
    }
    HIVIEW_LOGD("process changed, key=%{public}s, value=%{public}s", key, value);
    // The id of the dead process in the cache needs to be deleted.
    ProcessStatus::GetInstance().NotifyProcessState(pid, DIED);
}
}  // namespace HiviewDFX
}  // namespace OHOS
