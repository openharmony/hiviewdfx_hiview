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

#include "hiview_app_context.h"

#include <string>

#include "application_context.h"
#include "hiview_logger.h"
#include "storage_acl.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiViewAppContext");
namespace AppConext {
bool GrantBaseDirAccessToHiView(std::string &baseDirOut)
{
    std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> context =
        OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        HIVIEW_LOGE("context is null.");
        return false;
    }

    std::string baseDir = context->GetBaseDir();
    if (baseDir.empty()) {
        HIVIEW_LOGE("base dir is empty.");
        return false;
    }

    int aclBaseRet = OHOS::StorageDaemon::AclSetAccess(baseDir, "g:1201:x");
    if (aclBaseRet != 0) {
        HIVIEW_LOGE("set acl access for base dir failed.");
        return false;
    }
    baseDirOut = baseDir;
    return true;
}

bool GrantCacheDirAccessToHiView(std::string &cacheDirOut)
{
    std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> context =
        OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        HIVIEW_LOGE("context is null.");
        return false;
    }

    std::string cacheDir = context->GetCacheDir();
    if (cacheDir.empty()) {
        HIVIEW_LOGE("cache dir is empty.");
        return false;
    }

    cacheDirOut = context->GetCacheDir();
    int aclCacheRet = OHOS::StorageDaemon::AclSetAccess(cacheDir, "g:1201:x");
    if (aclCacheRet != 0) {
        HIVIEW_LOGE("set acl access for cache dir failed.");
        return false;
    }
    return true;
}
} // namespace AppConext
} // namespace HiviewDFX
} // namespace OHOS