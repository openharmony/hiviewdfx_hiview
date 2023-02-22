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

#ifndef SANITIZERD_MONITOR_H
#define SANITIZERD_MONITOR_H
#include <poll.h>
#include <string>

#include "reporter.h"

namespace OHOS {
namespace HiviewDFX {

typedef void (*SANITIZERD_NOTIFY_CALLBACK)(int32_t, const std::string&);

class SanitizerdMonitor {
public:
    int Init(SANITIZERD_NOTIFY_CALLBACK pcb);
    void Uninit();
    int RunMonitor(std::string *filename, int timeout);

private:
    int ReadNotify(std::string *sfilename, int nfd);
    struct pollfd* g_ufds = nullptr;
    int g_nfds = 0;
    SANITIZERD_NOTIFY_CALLBACK g_callback = nullptr;
    int g_asanWd = -1;
};
}
}
#endif // SANITIZERD_MONITOR_H

