/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "memory_util.h"

#ifdef USE_JEMALLOC_DFX_INTF
#include <malloc.h>
#endif

namespace OHOS {
namespace HiviewDFX {
namespace MemoryUtil {
int DisableThreadCache()
{
#ifdef USE_JEMALLOC_DFX_INTF
    return mallopt(M_SET_THREAD_CACHE, M_THREAD_CACHE_DISABLE) == 1 ? E_MEM_OK : E_MEM_ERR;
#else
    return E_MEM_OK;
#endif
}

int DisableDelayFree()
{
#ifdef USE_JEMALLOC_DFX_INTF
    return mallopt(M_DELAYED_FREE, M_DELAYED_FREE_DISABLE) == 1 ? E_MEM_OK : E_MEM_ERR;
#else
    return E_MEM_OK;
#endif
}
} // namespace MemoryUtil
} // namespace HiviewDFX
} // namespace OHOS
