/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <string>
#include <list>

namespace OHOS {
namespace HiviewDFX {

typedef enum CmdType {
    CLEAR_DIR,
    KILL_ALL,
    PID_OF,
} CmdType;

class TestUtil {
public:
    static bool FileExists(const std::string &path);
    static std::string GetSmapsPath(const std::string &processName);
    static std::string GetSampleFile(const std::string &processName);
    static void KillProcess(const std::string &name);
    static void ClearDir(std::string name);
    static std::string BuildCommand(CmdType type, const std::string& target);
    static bool IsLeakingProcessAlive();
    static bool GetStatm(uint64_t &rss);
    static bool GetSelfStatm(uint64_t &rss);
    static bool RestartProcess(const std::string &name);
    static void CopyFile(const std::string &srcFile, const std::string &dstFile);
    static void WriteFile(const std::string &file, const std::string &line);
    static void DeleteFile(const std::string &file);

private:
    static std::string ExecCmd(CmdType type, const std::string &target);
    static std::string GetPidByProcess(const std::string &name);
    static bool IsValidTarget(const std::string &target);
    static bool IsSelfOverLimit(int leakTarget);
    static std::list<std::string> GetDightStrArr(const std::string &target);
    static pid_t leakProcessPid_;
};

} // namespace HiviewDFX
} // namespace OHOS

#endif // TEST_UTIL_H
