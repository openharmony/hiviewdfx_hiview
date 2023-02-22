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

#include "sanitizerd_monitor.h"

#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "securec.h"

#include "sanitizerd_log.h"
#include "logger.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr uint32_t NOTIFY_MASK = IN_CLOSE_WRITE | IN_MOVED_TO;
constexpr int EVENT_BUF_LEN = 512;
DEFINE_LOG_TAG("Faultlogger");
}

int SanitizerdMonitor::ReadNotify(std::string *sfilename, int nfd)
{
    size_t res;
    char filename[PATH_MAX];
    char eventBuf[EVENT_BUF_LEN];
    int eventSize;
    int eventPos = 0;
    int ret = 1;
    struct inotify_event *event = nullptr;
    std::string strSanLogPath;
    SanitizerdType type;

    res = read(nfd, eventBuf, sizeof(eventBuf));
    if (res < sizeof(*event)) {
        HIVIEW_LOGI("could not get notify events, %s\n", strerror(errno));
        return ret;
    }

    while (res >= sizeof(*event)) {
        event = reinterpret_cast<struct inotify_event*>(eventBuf + eventPos);
        if (event->len) {
            if (strcpy_s(filename, PATH_MAX,  event->name) != EOK) {
                HIVIEW_LOGI("try to copy the file name error, continue");
            }

            // Check the full path of log file.
            if (event->mask & NOTIFY_MASK) {
                *sfilename = filename;

                if (event->wd == g_asanWd) {
                    strSanLogPath = std::string(ASAN_LOG_PATH);
                    type = ASAN_LOG_RPT;
                }

                ret = 0;
                std::string strFileName(filename);
                std::string fullPath = strSanLogPath + "/" + strFileName;

                HIVIEW_LOGI("recv filename is:[%{public}s]\n", fullPath.c_str());
                if (g_callback != nullptr) {
                    g_callback(type, strFileName);
                }
            }
        }
        eventSize = sizeof(*event) + event->len;
        res -= eventSize;
        eventPos += eventSize;
    }
    return ret;
}

int SanitizerdMonitor::Init(SANITIZERD_NOTIFY_CALLBACK pcb)
{
    const std::string asanLogPath = std::string(ASAN_LOG_PATH);
    g_nfds = 1;
    g_ufds = reinterpret_cast<pollfd*>(calloc(1, sizeof(g_ufds[0])));
    g_ufds[0].fd = inotify_init();
    if (g_ufds[0].fd < 0) {
        HIVIEW_LOGI("inotify_init failed: %{public}d-%{public}s.", g_ufds[0].fd, strerror(errno));
        Uninit();
        return 1;
    }

    g_ufds[0].events = POLLIN;
    g_asanWd = inotify_add_watch(g_ufds[0].fd, asanLogPath.c_str(), NOTIFY_MASK);
    if (g_asanWd < 0) {
        HIVIEW_LOGI("add watch %{public}s failed: %{public}d-%{public}s.", asanLogPath.c_str(), g_ufds[0].fd,
            strerror(errno));
        Uninit();
        return 1;
    } else {
        HIVIEW_LOGI("add watch %{public}s successfully: %{public}d.", asanLogPath.c_str(), g_ufds[0].fd);
    }

    g_callback = pcb;
    return 0;
}

void SanitizerdMonitor::Uninit()
{
    for (int i = 0; i < g_nfds; i++) {
        if (g_ufds[i].fd >= 0) {
            close(g_ufds[i].fd);
        }
    }

    if (g_ufds != nullptr) {
        free(g_ufds);
        g_ufds = nullptr;
    }
    g_nfds = 0;
    g_asanWd = -1;
}

int SanitizerdMonitor::RunMonitor(std::string *filename, int timeout)
{
    int pollres;

    while (true) {
        pollres = poll(g_ufds, g_nfds, timeout);
        if (pollres == 0) {
            return 1;
        }

        if (g_ufds[0].revents & POLLIN) {
            ReadNotify(filename, g_ufds[0].fd);
        }
    }
    return 0;
}
} // namespace HiviewDFX
} // namespace OHOS
