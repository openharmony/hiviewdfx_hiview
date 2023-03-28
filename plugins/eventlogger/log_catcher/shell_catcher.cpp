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
    description_ = "catcher cmd: " + catcherCmd + " ";
    return true;
}

void ShellCatcher::SetCmdArgument(const char* arg[], size_t argSize)
{
    if (arg == nullptr || argSize == 0) {
        HIVIEW_LOGE("SetCmdArgument fail");
	return;
    }
    cmdArgument = arg;
}

void ShellCatcher::DoChildProcess(int &inFd, int &outFd)
{
    close(outFd);
    if (dup2(inFd, 1) == -1) {
        HIVIEW_LOGE("dup2 inFd fail");
    }
    close(inFd);

    int ret = execv(catcherCmd.c_str(), (char * const *)cmdArgument);
    HIVIEW_LOGE("execv %{public}d, errno: %{public}d", ret, errno);
}

void ShellCatcher::DoFatherProcess(int &inFd, int &outFd, int childPid, int writeFd)
{
    char buf[BUF_SIZE_4096] = {0};
    size_t restNum = 0;
    time_t startTime = time(nullptr);
    constexpr int timeSpaceLimit = 5;

    close(inFd);
    while (true) {
        restNum = read(outFd, buf, sizeof(buf));
        if (restNum == 0 || !FileUtil::WriteBufferToFd(writeFd, buf, restNum)) {
            HIVIEW_LOGE("WriteBufferToFd fail, resNum: %{public}d", restNum);
            break;
        }

	if (time(nullptr) > startTime + timeSpaceLimit) {
            HIVIEW_LOGE("timeout limit");
            break;
	}
    }
    close(outFd);
}

bool ShellCatcher::ReadShellToFile(int writeFd, const std::string& cmd)
{
    constexpr int pipeCnt = 2;
    int pipes[pipeCnt];
    if (pipe(pipes) != 0) {
        HIVIEW_LOGE("pipe creat fail");
	return false;
    }

    int outFd = pipes[0];
    int inFd = pipes[1];

    int childPid = fork();
    if (childPid < 0) {
	HIVIEW_LOGE("fork fail");
	return false;
    } else if (childPid == 0) {
	DoChildProcess(inFd, outFd);
    } else {
	sleep(shellWaitTime);
	DoFatherProcess(inFd, outFd, childPid, writeFd);
    }
    return true;

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
