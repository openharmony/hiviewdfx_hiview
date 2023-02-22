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

#ifndef SANITIZERD_LOG_H
#define SANITIZERD_LOG_H

#include <cstdio>
#include <cstring>

#include "reporter.h"
#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
constexpr static char ASAN_LOG_PATH[] = "/dev/asanlog";

static constexpr OHOS::HiviewDFX::HiLogLabel SANITIZERD_LABEL = {LOG_CORE, 0xD002D12, "Sanitizer"};

#define FILE_NAME(x) (strrchr(x, '/') ? (strrchr(x, '/') + 1) : (x))

#define SANITIZERD_DECORATOR_HILOG(op, fmt, args...)                                                    \
do {                                                                                                        \
    op(SANITIZERD_LABEL, "[%{public}s:%{public}d] " fmt, FILE_NAME(__FILE__), __LINE__, ##args);         \
} while (0)

#define SANITIZERD_LOGF(fmt, args...) SANITIZERD_DECORATOR_HILOG(OHOS::HiviewDFX::HiLog::Fatal, fmt, ##args)
#define SANITIZERD_LOGE(fmt, args...) SANITIZERD_DECORATOR_HILOG(OHOS::HiviewDFX::HiLog::Error, fmt, ##args)
#define SANITIZERD_LOGW(fmt, args...) SANITIZERD_DECORATOR_HILOG(OHOS::HiviewDFX::HiLog::Warn, fmt, ##args)
#define SANITIZERD_LOGI(fmt, args...) SANITIZERD_DECORATOR_HILOG(OHOS::HiviewDFX::HiLog::Info, fmt, ##args)

} // namespace HiviewDFX
} // namespace OHOS
#endif // SANITIZERD_LOG_H
