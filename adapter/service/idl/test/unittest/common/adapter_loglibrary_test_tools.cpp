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

#include "adapter_loglibrary_test_tools.h"

#include <string>
#include <vector>

#include "accesstoken_kit.h"
#include "file_util.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace OHOS {
namespace HiviewDFX {
void AdapterLoglibraryTestTools::CreateDir(const std::string& dir)
{
    if (!FileUtil::FileExists(dir)) {
        FileUtil::ForceCreateDirectory(dir, FileUtil::FILE_PERM_770);
    }
}

void AdapterLoglibraryTestTools::NativeToken(const char* permList[], int permSize)
{
    uint64_t tokenId;
    NativeTokenInfoParams tokenInfo = {
        .dcapsNum = 0,
        .permsNum = permSize,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = permList,
        .acls = nullptr,
        .aplStr = "system_basic",
    };

    tokenInfo.processName = "AdapterLoglibraryIdlTest";
    tokenId = GetAccessTokenId(&tokenInfo);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void AdapterLoglibraryTestTools::ApplyPermissionAccess()
{
    const char* permList[] = {
        "ohos.permission.WRITE_HIVIEW_SYSTEM",
        "ohos.permission.READ_HIVIEW_SYSTEM",
    };
    constexpr int permSize = 2;
    NativeToken(permList, permSize); // 2 is the size of the array which consists of required permissions.
}
}
}