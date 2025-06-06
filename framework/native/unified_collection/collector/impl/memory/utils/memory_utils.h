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
#ifndef FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_MEMORY_UTILS_H
#define FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_MEMORY_UTILS_H

#include "memory.h"

namespace OHOS {
namespace HiviewDFX {
bool ParseSmaps(int32_t pid, const std::string& smapsPath, ProcessMemoryDetail& processMemoryDetail,
    GraphicMemOption option);
} // HiViewDFX
} // OHOS
#endif // FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_MEMORY_UTILS_H
