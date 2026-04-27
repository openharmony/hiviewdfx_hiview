/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef HIRETRIEVAL_DYNAMIC_LOADER_INCLUDE_H
#define HIRETRIEVAL_DYNAMIC_LOADER_INCLUDE_H

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "hiretrieval_base_def.h"

namespace OHOS::HiviewDFX {
#if defined(_WIN32)
using DllModule = HMODULE;
#define INVALID_DLL_MODULE NULL
#else
using DllModule = void*;
#define INVALID_DLL_MODULE nullptr
#endif

class HiRetrievalDynamicLoader {
public:
    HiRetrievalDynamicLoader();
    ~HiRetrievalDynamicLoader();

    int32_t Init();
    int32_t Participate();
    int32_t Quit();
    int32_t Run();

private:
    DllModule loadModule_ = INVALID_DLL_MODULE;
};
} // namespace OHOS::HiviewDFX

#endif // HIRETRIEVAL_DYNAMIC_LOADER_INCLUDE_H