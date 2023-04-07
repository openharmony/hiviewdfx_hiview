/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "eventservice_fuzzer.h"

#include <cinttypes>
#include <cstddef>
#include <cstdint>

#include "file_util.h"
#include "hiview_platform.h"
#include "query_sys_event_callback_stub.h"
#include "sys_event_service_ohos.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const std::string CONFIG_FILE_PATH = "/data/test/hiview_platform_config";
}

class TestQueryCallback : public QuerySysEventCallbackStub {
public:
    void OnQuery(const std::vector<std::u16string>& sysEvent, const std::vector<int64_t>& seq) override {}

    void OnComplete(int32_t reason, int32_t total, int64_t seq) override {}

    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
        MessageOption& option) override
    {
        return 0;
    }
};

bool CreateConfigFile()
{
    std::string content = "DEFAULT_PLUGIN_CONFIG_NAME = \"plugin_config\"\n" \
        "PLUGIN_CONFIG_FILE_DIR = \"/system/etc/hiview/\"\n" \
        "DYNAMIC_LIB_SEARCH_DIR = \"/system/lib/\"\n" \
        "DYNAMIC_LIB64_SEARCH_DIR = \"/system/lib64/\"\n" \
        "WORK_DIR = \"/data/test/hiview_fuzz/\"\n" \
        "COMMERCIAL_WORK_DIR = \"/log/LogService/\"\n" \
        "PERSIST_DIR = \"/log/hiview/\"";
    return FileUtil::SaveStringToFile(CONFIG_FILE_PATH, content);
}

bool InitEnvironment()
{
    if (!CreateConfigFile()) {
        printf("failed to create config file, exit\n");
        return false;
    }
    HiviewPlatform &platform = HiviewPlatform::GetInstance();
    return platform.InitEnvironment(CONFIG_FILE_PATH);
}

int32_t SysEventQueryTest(const uint8_t* data, size_t size)
{
    static OHOS::sptr<OHOS::HiviewDFX::IQuerySysEventCallback> callback = new(std::nothrow) TestQueryCallback();
    if (callback == nullptr) {
        printf("callback is null, exit.\n");
        return -1;
    }
    int64_t int64Data = static_cast<int64_t>(*data);
    int32_t int32Data = static_cast<int32_t>(*data);
    QueryArgument arg(int64Data, int64Data, int32Data);
    std::string strData = std::string(reinterpret_cast<const char*>(data), size);
    SysEventQueryRuleGroupOhos rules = { SysEventQueryRule(strData, { strData }) };
    auto service = SysEventServiceOhos::GetInstance();
    if (service == nullptr) {
        printf("SysEventServiceOhos service is null.\n");
        return -1;
    }
    return service->Query(arg,  rules, callback);
}
} // namespace HiviewDFX
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    static bool initialized = OHOS::HiviewDFX::InitEnvironment();
    if (!initialized) {
        printf("failed to init environment, exit\n");
        return -1;
    }
    (void)OHOS::HiviewDFX::SysEventQueryTest(data, size);
    return 0;
}

