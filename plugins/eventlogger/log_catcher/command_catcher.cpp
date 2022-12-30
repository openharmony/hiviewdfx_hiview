/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "command_catcher.h"

#include "dump_client_main.h"

#include "common_utils.h"
#include "log_catcher_utils.h"
namespace OHOS {
namespace HiviewDFX {
CommandCatcher::CommandCatcher() : EventLogCatcher()
{
    name_ = "CommandCatcher";
}

void CommandCatcher::AddCmd(const std::vector<std::string>& cmd)
{
    cmdString_.push_back(cmd);
}

bool CommandCatcher::Initialize(const std::string& packageNam, int pid, int intParam)
{
    if (pid <= 0 && packageNam.length() == 0) {
        description_ = "CommandCatcher -- pid is null, packageName is null\n";
        return false;
    }
    pid_ = pid;
    packageName_ = packageNam;

    if (pid_ <= 0) {
        description_ = "CommandCatcher -- packageName is " + packageName_ + " pid is null\n";
        return false;
    }

    if (packageName_.length() == 0) {
        packageName_ = "(null)";
    }

    description_ = "CommandCatcher -- pid==" + std::to_string(pid_) + " packageName is " + packageName_ + "\n";
    return EventLogCatcher::Initialize(packageNam, pid, intParam);
}

int CommandCatcher::Catch(int fd)
{
    if (pid_ <= 0) {
        return -1;
    }
    auto originSize = GetFdSize(fd);

    for (auto& args : cmdString_) {
        HiDumper(fd, args);
    }

    logSize_ = GetFdSize(fd) - originSize;
    return logSize_;
}

int CommandCatcher::HiDumper(int fd, const std::vector<std::string> &args)
{
    int argc = args.size();

    std::vector<char *> argv;
    for (const auto &arg : args) {
        argv.push_back(const_cast<char *>(arg.c_str()));
    }

    return DumpClientMain::GetInstance().Main(argc, &argv[0], fd);
}
} // namespace HiviewDFX
} // namespace OHOS