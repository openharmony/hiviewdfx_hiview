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
#include "shell_catcher.h"
#include <unistd.h>
#include "dump_client_main.h"
#include "logger.h"
#include "common_utils.h"
#include "log_catcher_utils.h"
#include "securec.h"
namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_LABEL(0xD002D01, "EventLogger-ShellCatcher");
ShellCatcher::ShellCatcher() : EventLogCatcher()
{
    name_ = "ShellCatcher";
}

bool ShellCatcher::Initialize(const std::string& cmd, int time, int intParam __UNUSED)
{
    catcherCmd = cmd;
    shellWaitTime = time;
    return true;
}

bool ShellCatcher::ReadShellToFile(int fd, const std::string& cmd)
{
    FILE *fp = nullptr;
    fp = popen(cmd.c_str(), "r");
    if (fp == nullptr) {
        HIVIEW_LOGE("popen %{public}s fail, errno: %{public}d", cmd.c_str(), errno);
        return false;
    }

    if (shellWaitTime > 0) {
        sleep(shellWaitTime);
    }

    char buf[BUF_SIZE_4096] = {0};
    size_t restNum = 0;
    while (!feof(fp)) {
        restNum = fread(buf, 1, BUF_SIZE_4096 - 1, fp);
        if (FileUtil::WriteBufferToFd(fd, buf, restNum)) {
            HIVIEW_LOGE("WriteBufferToFd fail, resNum: %{public}d", restNum);
            break;
        }
    }
    return pclose(fp);
}

int ShellCatcher::Catch(int fd)
{
    auto originSize = GetFdSize(fd);
    if (catcherCmd.empty()) {
        HIVIEW_LOGE("catcherCmd empty");
        return -1;
    }

    ReadShellToFile(fd, catcherCmd);
    logSize_ = GetFdSize(fd) - originSize;
    return logSize_;
}
} // namespace HiviewDFX
} // namespace OHOS
