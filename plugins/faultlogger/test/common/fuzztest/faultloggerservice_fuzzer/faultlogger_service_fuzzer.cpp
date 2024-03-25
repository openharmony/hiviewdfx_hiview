/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "faultlogger.h"
#include "faultlogger_service_ohos.h"
#include "faultlogger_service_fuzzer.h"
#include "hiview_platform.h"
#include "securec.h"
namespace OHOS {

std::shared_ptr<HiviewDFX::Faultlogger> CreateFaultloggerInstance()
{
    static std::unique_ptr<HiviewDFX::HiviewPlatform> platform = std::make_unique<HiviewDFX::HiviewPlatform>();
    auto plugin = std::make_shared<HiviewDFX::Faultlogger>();
    plugin->SetName("Faultlogger");
    plugin->SetHandle(nullptr);
    plugin->SetHiviewContext(platform.get());
    plugin->OnLoad();
    return plugin;
}

void FuzzServiceInterfaceDump(const uint8_t* data, size_t size)
{
    auto service = CreateFaultloggerInstance();
    HiviewDFX::FaultloggerServiceOhos serviceOhos;
    HiviewDFX::FaultloggerServiceOhos::StartService(service.get());
    if (HiviewDFX::FaultloggerServiceOhos::GetOrSetFaultlogger(nullptr) != service.get()) {
        printf("FaultloggerServiceOhos start service error.\n");
    }

    int32_t fd = static_cast<int32_t>(*data);
    std::vector<std::u16string> args;
    constexpr int maxLen = 20;
    char16_t arg[maxLen] = {0};
    errno_t err = strncpy_s(reinterpret_cast<char*>(arg), sizeof(arg), reinterpret_cast<const char*>(data), size);
    if (err != EOK) {
        std::cout << "strncpy_s arg failed" << std::endl;
        return;
    }
    args.push_back(arg);

    (void)serviceOhos.Dump(fd, args);
}

void FuzzServiceInterfaceQuerySelfFaultLog(const uint8_t* data, size_t size)
{
    auto service = CreateFaultloggerInstance();
    HiviewDFX::FaultloggerServiceOhos serviceOhos;
    HiviewDFX::FaultloggerServiceOhos::StartService(service.get());
    if (HiviewDFX::FaultloggerServiceOhos::GetOrSetFaultlogger(nullptr) != service.get()) {
        printf("FaultloggerServiceOhos start service error.\n");
    }
    int32_t faultType = static_cast<int32_t>(*data);
    int32_t maxNum = static_cast<int32_t>(*data);
    (void)serviceOhos.QuerySelfFaultLog(faultType, maxNum);
}

void FuzzFaultloggerServiceInterface(const uint8_t* data, size_t size)
{
    FuzzServiceInterfaceDump(data, size);
    FuzzServiceInterfaceQuerySelfFaultLog(data, size);
}
}

// Fuzzer entry point.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::FuzzFaultloggerServiceInterface(data, size);
    return 0;
}
