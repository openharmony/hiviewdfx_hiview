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

#ifndef HIRETRIEVAL_ANI_UTIL_INCLUDE_H
#define HIRETRIEVAL_ANI_UTIL_INCLUDE_H

#include <ani.h>
#include <string>

#include "hiretrieval_base_def.h"
#include "hiretrieval_mgr.h"

namespace OHOS::HiviewDFX {
class HiRetrievalAniUtil {
public:
    static void CheckRetAndThrowError(ani_env* env, int32_t retCode);
    static void ParseJsHiRetrievalConfig(ani_env* env, ani_object& aniVal, HiRetrievalMgr::Config& cfg);
    static void CreateJsHiRetrievalConfig(ani_env* env, const HiRetrievalMgr::Config& cfg, ani_object& cfgObj);
};
}; // namespace OHOS::HiviewDFX

#endif // HIRETRIEVAL_ANI_UTIL_INCLUDE_H