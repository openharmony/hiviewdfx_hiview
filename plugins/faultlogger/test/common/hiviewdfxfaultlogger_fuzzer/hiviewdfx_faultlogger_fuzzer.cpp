/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include <fstream>
#include <sstream>

#include "faultlogger_client.h"
#include "hiviewdfx_faultlogger_fuzzer.h"

namespace OHOS {
std::vector<std::string> g_cmdRet;
std::string g_faultloggerCmd = "dumpsys hiviewdfx_faultlogger ";

int ExecCmdWithRet(std::string cmd, std::vector<std::string> &resvec)
{
    resvec.clear();
    FILE *pp = popen(cmd.c_str(), "r");
    if (pp == nullptr) {
        return -1;
    }
    char tmp[1024];
    while (fgets(tmp, sizeof(tmp), pp) != nullptr) {
        if (tmp[strlen(tmp) - 1] == '\n') {
            tmp[strlen(tmp) - 1] = '\0';
        }
        resvec.push_back(tmp);
    }
    pclose(pp);
    return resvec.size();
}

void FuzzFaultlogger(const uint8_t* data, size_t size)
{
    std::string rawString(reinterpret_cast<const char*>(data), size);
    rawString = g_faultloggerCmd + rawString;
    ExecCmdWithRet(rawString.c_str(), g_cmdRet);
}

void FuzzFaultloggerOptD(const uint8_t* data, size_t size)
{
    std::string rawString(reinterpret_cast<const char*>(data), size);
    rawString = g_faultloggerCmd + "-d" + rawString;
    ExecCmdWithRet(rawString.c_str(), g_cmdRet);
}

void FuzzFaultloggerOptF(const uint8_t* data, size_t size)
{
    std::string rawString(reinterpret_cast<const char*>(data), size);
    rawString = g_faultloggerCmd + "-f" + rawString;
    ExecCmdWithRet(rawString.c_str(), g_cmdRet);
}

void FuzzFaultloggerOptL(const uint8_t* data, size_t size)
{
    std::string rawString(reinterpret_cast<const char*>(data), size);
    rawString = g_faultloggerCmd + "-l" + rawString;
    ExecCmdWithRet(rawString.c_str(), g_cmdRet);
}

void FuzzFaultloggerOptM(const uint8_t* data, size_t size)
{
    std::string rawString(reinterpret_cast<const char*>(data), size);
    rawString = g_faultloggerCmd + "-m" + rawString;
    ExecCmdWithRet(rawString.c_str(), g_cmdRet);
}

void FuzzFaultloggerOptT(const uint8_t* data, size_t size)
{
    std::string rawString(reinterpret_cast<const char*>(data), size);
    rawString = g_faultloggerCmd + "-t" + rawString;
    ExecCmdWithRet(rawString.c_str(), g_cmdRet);
}

void FuzzInterfaceAddFaultLog(const uint8_t* data, size_t size)
{
    FaultLogInfoInner inner;
    inner.time = static_cast<int64_t>(*data);
    inner.id = static_cast<int32_t>(*data);
    inner.pid = static_cast<int32_t>(*data);
    inner.faultLogType = static_cast<HiviewDFX::FaultLogType>(*data);
    inner.module = std::string(reinterpret_cast<const char*>(data), size);
    inner.summary = std::string(reinterpret_cast<const char*>(data), size);
    inner.module = std::string(reinterpret_cast<const char*>(data), size);
    inner.logPath = std::string(reinterpret_cast<const char*>(data), size);
    HiviewDFX::AddFaultLog(inner);
    HiviewDFX::AddFaultLog(inner.time, inner.faultLogType, inner.module, inner.summary);
}

void FuzzInterfaceQuerySelfFaultLog(const uint8_t* data, size_t size)
{
    auto type = static_cast<HiviewDFX::FaultLogType>(*data);
    auto count = static_cast<int32_t>(*data);
    auto result = HiviewDFX::QuerySelfFaultLog(type, count);
    if (result != nullptr) {
        while (result->HasNext()) {
            result->Next();
        }
    }
}

void FuzzFaultloggerDumpCmds(const uint8_t* data, size_t size)
{
    FuzzFaultlogger(data, size);
    FuzzFaultloggerOptD(data, size);
    FuzzFaultloggerOptF(data, size);
    FuzzFaultloggerOptL(data, size);
    FuzzFaultloggerOptM(data, size);
    FuzzFaultloggerOptT(data, size);
}

void FuzzFaultloggerClientInterface(const uint8_t* data, size_t size)
{
    FuzzInterfaceAddFaultLog(data, size);
    FuzzInterfaceQuerySelfFaultLog(data, size);
}
}

// Fuzzer entry point.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    // OHOS::FuzzFaultloggerDumpCmds(data, size);
    OHOS::FuzzFaultloggerClientInterface(data, size);
    return 0;
}