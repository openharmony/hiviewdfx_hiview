/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "common_utils.h"

#include <cstdint>
#include <cstdio>
#include <dirent.h>
#include <fcntl.h>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <vector>

#include "securec.h"
#include "time_util.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
namespace CommonUtils {
int ExecCommand(const std::string &cmd, const std::vector<std::string> &args)
{
    pid_t pid = fork();
    if (pid < 0) {
        return -1;
    } else if (pid == 0) {
        // Redirect the stdout to /dev/null
        int fd = open("/dev/null", O_WRONLY);
        // follow standard, although dup2 may handle the case of invalid oldfd
        if (fd >= 0) {
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        std::vector<char *> argv;
        argv.push_back(const_cast<char *>(cmd.c_str()));
        for (const auto &arg : args) {
            argv.push_back(const_cast<char *>(arg.c_str()));
        }
        argv.push_back(0);
        execv(argv[0], &argv[0]);
    }
    constexpr uint64_t maxWaitingTime = 60; // 60 seconds
    uint64_t remainedTime = maxWaitingTime * NS_PER_SECOND;
    while (remainedTime > 0) {
        uint64_t startTime = TimeUtil::GetNanoTime();
        int status = 0;
        waitpid(pid, &status, WNOHANG);
        if (WIFEXITED(status)) {
            return 0;
        }
        sleep(1);
        uint64_t duration = TimeUtil::GetNanoTime() - startTime;
        remainedTime = (remainedTime > duration) ? (remainedTime - duration) : 0;
    }

    return -1;
}

std::string GetProcNameByPid(pid_t pid)
{
    std::string result;
    char buf[BUF_SIZE_256] = {0};
    (void)snprintf_s(buf, BUF_SIZE_256, BUF_SIZE_256 - 1, "/proc/%d/cmdline", pid);
    FileUtil::LoadStringFromFile(std::string(buf, strlen(buf)), result);
    auto pos = result.find_last_not_of(" \n\r\t");
    if (pos == std::string::npos) {
        return result;
    }
    result.erase(pos + 1);
    return result.c_str();
}

pid_t GetPidByName(const std::string& processName)
{
    pid_t pid = -1;
    std::string cmd = "pidof " + processName;

    char buffer[BUF_SIZE_256] = {'\0'};
    FILE* fp = popen(cmd.c_str(), "r");
    if (fp != nullptr) {
        while (fgets(buffer, sizeof(buffer) - 1, fp) != nullptr) {}
        std::istringstream istr(buffer);
        istr >> pid;
        pclose(fp);
    }
    return pid;
}

bool IsTheProcessExist(pid_t pid)
{
    int ret = syscall(SYS_tgkill, pid, pid, 0);
    if (ret != 0 && errno == ESRCH) {
         return false;
    }
    return true;
}

bool IsSpecificCmdExist(const std::string& fullPath)
{
    return access(fullPath.c_str(), X_OK) == 0;
}

bool WriteCommandResultToFile(int fd, const std::string& cmd)
{
    if (cmd.empty()) {
        return false;
    }
    char buffer[BUF_SIZE_256];
    FILE* fp = popen(cmd.c_str(), "r");
    if (fp != nullptr) {
        while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
            FileUtil::SaveStringToFd(fd, buffer);
        }
        pclose(fp);
        return true;
    }
    return false;
}
}
} // namespace HiviewDFX
} // namespace OHOS
