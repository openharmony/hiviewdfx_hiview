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

#include "hiretrieval_ani.h"

#include <cinttypes>

#include "hilog/log.h"
#include "hiretrieval_ani_util.h"
#include "hiretrieval_common_util.h"
#include "hiretrieval_mgr.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D10

#undef LOG_TAG
#define LOG_TAG "HIRETRIEVAL_ANI"

using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::HiRetrieval;

namespace OHOS::HiviewDFX {
void HiRetrievalAni::Init(ani_env* env)
{
    auto ret = HiRetrievalMgr::GetInstance().Init();
    HiRetrievalAniUtil::CheckRetAndThrowError(env, ret);
}

void HiRetrievalAni::Participate(ani_env* env, ani_object config)
{
    HiRetrievalMgr::Config cfg;
    HiRetrievalAniUtil::ParseJsHiRetrievalConfig(env, config, cfg);
    auto ret = HiRetrievalMgr::GetInstance().Participate(cfg);
    HiRetrievalAniUtil::CheckRetAndThrowError(env, ret);
}

void HiRetrievalAni::Quit(ani_env* env)
{
    auto ret = HiRetrievalMgr::GetInstance().Quit();
    HiRetrievalAniUtil::CheckRetAndThrowError(env, ret);
}

ani_boolean HiRetrievalAni::IsParticipant(ani_env* env)
{
    bool isParticipant = HiRetrievalMgr::GetInstance().IsParticipant();
    return static_cast<ani_boolean>(isParticipant);
}

ani_long HiRetrievalAni::GetLastParticipationTs(ani_env* env)
{
    auto lastParticipationTs = HiRetrievalMgr::GetInstance().GetLastParticipationTs();
    return static_cast<ani_long>(lastParticipationTs);
}

void HiRetrievalAni::Run(ani_env* env)
{
    auto ret = HiRetrievalMgr::GetInstance().Run();
    HiRetrievalAniUtil::CheckRetAndThrowError(env, ret);
}

ani_object HiRetrievalAni::GetCurrentConfig(ani_env* env)
{
    auto curConfig = HiRetrievalMgr::GetInstance().GetCurrentConfig();
    ani_object val {};
    HiRetrievalAniUtil::CreateJsHiRetrievalConfig(env, curConfig, val);
    return val;
}
}; // namespace OHOS::HiviewDFX

ANI_EXPORT ani_status ANI_Constructor(ani_vm* vm, uint32_t* result)
{
    ani_env* env;
    if (vm->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "failed to get ani envrionment.");
        return ANI_ERROR;
    }
    ani_namespace ns {};
    if (env->FindNamespace("@ohos.hiviewdfx.hiRetrieval.hiRetrieval", &ns) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "failed to find name space of hiRetrieval.");
        return ANI_INVALID_ARGS;
    }
    std::array methods = {
        ani_native_function {ApiDef::INIT, nullptr, reinterpret_cast<void *>(OHOS::HiviewDFX::HiRetrievalAni::Init)},
        ani_native_function {ApiDef::PARTICIPATE, nullptr,
            reinterpret_cast<void *>(OHOS::HiviewDFX::HiRetrievalAni::Participate)},
        ani_native_function {ApiDef::QUIT, nullptr, reinterpret_cast<void *>(OHOS::HiviewDFX::HiRetrievalAni::Quit)},
        ani_native_function {ApiDef::IS_PARTICIPANT, nullptr,
            reinterpret_cast<void *>(OHOS::HiviewDFX::HiRetrievalAni::IsParticipant)},
        ani_native_function {ApiDef::GET_LAST_PARTICIPATION_TS, nullptr,
            reinterpret_cast<void *>(OHOS::HiviewDFX::HiRetrievalAni::GetLastParticipationTs)},
        ani_native_function {ApiDef::RUN, nullptr, reinterpret_cast<void *>(OHOS::HiviewDFX::HiRetrievalAni::Run)},
        ani_native_function {ApiDef::GET_CUR_CFG, nullptr,
            reinterpret_cast<void *>(OHOS::HiviewDFX::HiRetrievalAni::GetCurrentConfig)},
    };
    if (env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size()) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "failed to bind native functions of hiRetrieval.");
        return ANI_INVALID_TYPE;
    };
    *result = ANI_VERSION_1;
    return ANI_OK;
}