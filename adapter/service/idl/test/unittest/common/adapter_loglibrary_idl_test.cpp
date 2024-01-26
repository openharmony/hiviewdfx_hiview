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

#include "adapter_loglibrary_idl_test.h"

#include <string>
#include <vector>

#include "accesstoken_kit.h"
#include "file_util.h"
#include "hiview_service_ability_proxy.h"
#include "hiview_service_agent.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const std::string DEST_PATH = "";
const std::string SOURCE_PATH = "/data/log/logpack/remotelog/";
const std::string LOG_TYPE = "REMOTELOG";

void CreateDir(const std::string& dir)
{
    if (!FileUtil::FileExists(dir)) {
        FileUtil::ForceCreateDirectory(dir, FileUtil::FILE_PERM_770);
    }
}

void NativeToken(const char* permList[], int permSize)
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

void ApplyPermissionAccess()
{
    const char* permList[] = {
        "ohos.permission.WRITE_HIVIEW_SYSTEM",
        "ohos.permission.READ_HIVIEW_SYSTEM",
    };
    NativeToken(permList, 2); // 2 is the size of the array which consists of required permissions.
}
}
void AdapterLoglibraryIdlTest::SetUpTestCase() {}

void AdapterLoglibraryIdlTest::TearDownTestCase() {}

void AdapterLoglibraryIdlTest::SetUp()
{
    ApplyPermissionAccess();
    CreateDir(SOURCE_PATH);
    CreateDir(DEST_PATH);
}

void AdapterLoglibraryIdlTest::TearDown() {}

HWTEST_F(AdapterLoglibraryIdlTest, LoglibraryProxyListTest001, testing::ext::TestSize.Level1)
{
    std::string fileName = "List.txt";
    FileUtil::SaveStringToFile(SOURCE_PATH + fileName, "listtestcontent", true);
    std::vector<HiviewFileInfo> fileInfos;
    int32_t result = HiviewServiceAgent::GetInstance().List(LOG_TYPE, fileInfos);
    ASSERT_EQ(result, 0);
}

HWTEST_F(AdapterLoglibraryIdlTest, LoglibraryProxyCopyTest001, testing::ext::TestSize.Level1)
{
    std::string fileName = "Copy.txt";
    FileUtil::SaveStringToFile(SOURCE_PATH + fileName, "copytestcontent", true);
    int32_t result = HiviewServiceAgent::GetInstance().Copy(LOG_TYPE, fileName, DEST_PATH);
    ASSERT_EQ(result, -1);
}

HWTEST_F(AdapterLoglibraryIdlTest, LoglibraryProxyMoveTest001, testing::ext::TestSize.Level1)
{
    std::string fileName = "Move.txt";
    FileUtil::SaveStringToFile(SOURCE_PATH + fileName, "movetestcontent", true);
    int32_t result = HiviewServiceAgent::GetInstance().Move(LOG_TYPE, fileName, DEST_PATH);
    ASSERT_EQ(result, -1);
}

HWTEST_F(AdapterLoglibraryIdlTest, LoglibraryProxyRemoveTest001, testing::ext::TestSize.Level1)
{
    std::string fileName = "Reove.txt";
    FileUtil::SaveStringToFile(SOURCE_PATH + fileName, "removetestcontent", true);
    int32_t result = HiviewServiceAgent::GetInstance().Remove(LOG_TYPE, fileName);
    ASSERT_EQ(result, 0);
}
}
}