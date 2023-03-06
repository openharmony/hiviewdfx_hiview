/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include "dmesg_catcher.h"

#include <string>
#include <sys/klog.h>
#include <unistd.h>

#include "logger.h"
#include "log_catcher_utils.h"
#include "common_utils.h"
#include "securec.h"
namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_LABEL(0xD002D01, "EventLogger-DmesgCatcher");
namespace {
    constexpr int SYSLOG_ACTION_READ_ALL = 3;
    constexpr int SYSLOG_ACTION_SIZE_BUFFER = 10;
}
DmesgCatcher::DmesgCatcher() : EventLogCatcher()
{
    name_ = "DmesgCatcher";
}

bool DmesgCatcher::Initialize(const std::string& packageNam  __UNUSED,
    int pid  __UNUSED, int intParam)
{
    needWriteSysrq = intParam;
    description_ = needWriteSysrq ? "catcher: SysrqCatcher\n" : "catcher: DmesgCatcher\n";
    return true;
}

int DmesgCatcher::DumpDmesgLog(int fd)
{
    if (fd < 0) {
        return -1;
    }
    int size = klogctl(SYSLOG_ACTION_SIZE_BUFFER, 0, 0);
    if (size <= 0) {
        return -1;
    }
    char *data = (char *)malloc(size + 1);
    if (data == nullptr) {
        return -1;
    }

    memset_s(data, size + 1, 0, size + 1);
    int readSize = klogctl(SYSLOG_ACTION_READ_ALL, data, size);
    if (readSize < 0) {
        free(data);
        return -1;
    }
    bool res = FileUtil::SaveStringToFd(fd, data);
    free(data);
    return res;
}

bool DmesgCatcher::WriteSysrq()
{
    FILE* file = fopen("/proc/sysrq-trigger", "w");
    if (file == nullptr) {
        HIVIEW_LOGE("Can't read sysrq,errno: %{public}d", errno);
        return false;
    }
    fwrite("w", 1, 1, file);
    fflush(file);

    fwrite("l", 1, 1, file);
    sleep(1);
    return true;
}

int DmesgCatcher::Catch(int fd)
{
    auto originSize = GetFdSize(fd);
    if (needWriteSysrq && !WriteSysrq()) {
        return -1;
    }
    DumpDmesgLog(fd);
    logSize_ = GetFdSize(fd) - originSize;
    return logSize_;
}
} // namespace HiviewDFX
} // namespace OHOS
