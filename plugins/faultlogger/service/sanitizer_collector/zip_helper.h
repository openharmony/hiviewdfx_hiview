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

#ifndef SANITIZERD_ZIPHELPER_H
#define SANITIZERD_ZIPHELPER_H
#include <algorithm>
#include <string>
#include <vector>

#include "reporter.h"

namespace OHOS {
namespace HiviewDFX {
constexpr unsigned SL_BUF_LEN = 512;
constexpr unsigned GZ_BUF_SIZE = 4096;
constexpr char GZ_SUFFIX[] = ".gz";
constexpr unsigned BUF_SIZE = 1024;
constexpr unsigned MAX_PROCESS_PATH = 1024;
constexpr unsigned HASH_FACTOR = 16127;
constexpr unsigned MILLISEC_OF_PER_SEC = 1000;
constexpr unsigned MICROSEC_OF_PER_MILLISEC = 1000;
constexpr int MIN_APP_USERID = 10000;
constexpr uint32_t MAX_NAME_LENGTH = 4096;
constexpr int AID_ROOT   = 0;
constexpr int AID_SYSTEM = 1000;
constexpr mode_t DEFAULT_LOG_FILE_MODE = 0666;
constexpr mode_t DEFAULT_LOG_DIR_MODE = 0775;
constexpr char CUSTOM_SANITIZER_LOG_PATH[] = "/data/log/faultlog/faultlogger/";
constexpr char ROOT_FAULTLOG_LOG_PATH[] = "/data/log/faultlog/";
const std::string DEVICE_OHOS_VERSION_PARAM = "hw_sc.build.os.version";
const std::string EMPTY_PARAM = "";

bool IsLinkFile(const std::string& filename);
bool GetRealPath(const std::string& fn, std::string& out);
bool ReadNormalFileToString(const std::string& path, std::string& content);
bool ReadFileToString(const std::string& path, std::string& out);
std::vector<std::string> SplitString(const std::string& input, const std::string& regex);
unsigned HashString(const std::string& input);
bool GetNameByPid(pid_t pid, const char procName[]);
bool IsModuleNameValid(const std::string& name);
bool GetNameByPid(pid_t pid, const char *procName);
std::string RegulateModuleNameIfNeed(const std::string& name);
std::string GetApplicationNameById(int32_t uid);
std::string GetApplicationVersion(int32_t uid, const std::string& bundleName);
void WriteCollectedData(T_SANITIZERD_PARAMS *params);
} // namespace HiviewDFX
} // namespace OHOS

#endif // SANITIZERD_ZIPHELPER_H

