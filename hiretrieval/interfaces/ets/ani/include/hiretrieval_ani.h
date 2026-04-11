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


#ifndef HIRETRIEVAL_ANI_INCLUDE_H
#define HIRETRIEVAL_ANI_INCLUDE_H

#include <ani.h>

namespace OHOS::HiviewDFX {
class HiRetrievalAni {
public:
    static void Init(ani_env* env);
    static void Participate(ani_env* env, ani_object config);
    static void Quit(ani_env* env);
    static ani_boolean IsParticipant(ani_env* env);
    static ani_long GetLastParticipationTs(ani_env* env);
    static void Run(ani_env* env);
    static ani_object GetCurrentConfig(ani_env* env);
};
}; // namespace OHOS::HiviewDFX

#endif // HIRETRIEVAL_ANI_INCLUDE_H