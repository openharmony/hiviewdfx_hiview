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
#include "hiview_logger.h"

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
        if (event->len <= 0) {
            eventSize = sizeof(*event);
            res -= eventSize;
            eventPos += eventSize;
            continue;
        }
        if (strcpy_s(filename, PATH_MAX,  event->name) != EOK) {
            HIVIEW_LOGI("try to copy the file name error, continue");
            continue;
        }

        // Check the full path of log file.
        if (event->mask & NOTIFY_MASK) {
            *sfilename = filename;

            if (event->wd == gAsanWd) {
                strSanLogPath = std::string(ASAN_LOG_PATH);
                type = ASAN_LOG_RPT;
            }

            ret = 0;
            std::string strFileName(filename);
            std::string fullPath = strSanLogPath + "/" + strFileName;

            HIVIEW_LOGI("recv filename is:[%{public}s]\n", fullPath.c_str());
            if (gCallback != nullptr) {
                gCallback(type, strFileName);
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
    gNfds = 1;
    gUfds = reinterpret_cast<pollfd*>(calloc(1, sizeof(gUfds[0])));
    if (!gUfds) {
        HIVIEW_LOGI("Memory allocation failed for gUfds.");
        return 1;
    }
    gUfds[0].fd = inotify_init();
    if (gUfds[0].fd < 0) {
        HIVIEW_LOGI("inotify_init failed: %{public}d-%{public}s.", gUfds[0].fd, strerror(errno));
        Uninit();
        return 1;
    }

    gUfds[0].events = POLLIN;
    gAsanWd = inotify_add_watch(gUfds[0].fd, asanLogPath.c_str(), NOTIFY_MASK);
    if (gAsanWd < 0) {
        HIVIEW_LOGI("add watch %{public}s failed: %{public}d-%{public}s.", asanLogPath.c_str(), gUfds[0].fd,
            strerror(errno));
        Uninit();
        return 1;
    } else {
        HIVIEW_LOGI("add watch %{public}s successfully: %{public}d.", asanLogPath.c_str(), gUfds[0].fd);
    }

    gCallback = pcb;
    return 0;
}

void SanitizerdMonitor::Uninit()
{
    for (int i = 0; i < gNfds; i++) {
        if (gUfds[i].fd >= 0) {
            close(gUfds[i].fd);
        }
    }

    if (gUfds != nullptr) {
        free(gUfds);
        gUfds = nullptr;
    }
    gNfds = 0;
    gAsanWd = -1;
}

int SanitizerdMonitor::RunMonitor(std::string *filename, int timeout)
{
    int pollres;

    while (true) {
        pollres = poll(gUfds, gNfds, timeout);
        if (pollres == 0) {
            return 1;
        }

        if (gUfds[0].revents & POLLIN) {
            ReadNotify(filename, gUfds[0].fd);
        }
    }
    return 0;
}
} // namespace HiviewDFX
} // namespace OHOS
