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

#ifndef HIVIEW_SERVICE_AGENT_H
#define HIVIEW_SERVICE_AGENT_H

#include <string>
#include <vector>

#include "hiview_file_info.h"
#include "iremote_broker.h"

namespace OHOS {
namespace HiviewDFX {
class HiviewServiceAgent {
public:
    static int32_t List(const std::string& logType, std::vector<HiviewFileInfo>& fileInfos);
    static int32_t Copy(const std::string& logType, const std::string& logName, const std::string& dest);
    static int32_t Move(const std::string& logType, const std::string& logName, const std::string& dest);
    static int32_t Remove(const std::string& logType, const std::string& logName);

private:
    HiviewServiceAgent() = default;
    ~HiviewServiceAgent() = default;

    static int32_t CopyOrMoveFile(
        const std::string& logType, const std::string& logName, const std::string& dest, bool isMove);
    static bool CheckAndCreateHiviewDir(const std::string& destDir);
    static bool CreateDestDirs(const std::string& rootDir, const std::string& destDir);
    static bool CreateAndGrantAclPermission(const std::string& dirPath);
    static sptr<IRemoteObject> GetRemoteService();
};
} // namespace HiviewDFX
} // namespace OHOS
#endif