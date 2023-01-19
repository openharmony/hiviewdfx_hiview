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
#ifndef UTILITY_MEMORY_UTIL_H
#define UTILITY_MEMORY_UTIL_H

namespace OHOS {
namespace HiviewDFX {
namespace MemoryUtil {
/**
 * Disable the cache of the current thread.
 */
int DisableThreadCache();

/**
 * Disable the delayed free function of the current thread.
 */
int DisableDelayFree();
} // namespace MemoryUtil
} // namespace HiviewDFX
} // namespace OHOS
#endif // UTILITY_MEMORY_UTIL_H
