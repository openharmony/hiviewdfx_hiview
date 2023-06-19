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

#include "hiviewservice_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hiview_service.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
HiviewService g_hiviewService;
int g_fd = 0;

struct Initializer {
    Initializer()
    {
        g_fd = open("/dev/null", O_RDWR | O_CREAT | O_TRUNC, 0600); //0600 for file mode
        if (g_fd <= 0) {
            printf("failed to open file, exit\n");
            isInit = false;
            return;
        }
        isInit = true;
    }

    ~Initializer()
    {
        if (isInit) {
            (void)close(g_fd);
        }
    }

    bool isInit = false;
};
Initializer g_initializer;
}

static void HiviewServiceDumpTest(const uint8_t* data, size_t size)
{
    std::string strData = std::string(reinterpret_cast<const char*>(data), size);
    g_hiviewService.DumpRequestDispatcher(g_fd, {});
    g_hiviewService.DumpRequestDispatcher(g_fd, {strData});
}
} // namespace HiviewDFX
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (!OHOS::HiviewDFX::g_initializer.isInit) {
        printf("failed to init environment, exit\n");
        return 0;
    }
    OHOS::HiviewDFX::HiviewServiceDumpTest(data, size);
    return 0;
}
