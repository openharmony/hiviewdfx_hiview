/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef PARAM_MANAGER_H
#define PARAM_MANAGER_H

#include <singleton.h>
#include <string>
#include <vector>

#include "file_util.h"
#include "param_const_common.h"
#include "param_reader.h"

namespace OHOS {
namespace HiviewDFX {
class ParamManager {
public:
    static void InitParam();
private:
    static void OnUpdateNotice(const std::string& localCfgPath, const std::string& cloudCfgPath);
    static bool IsFileNeedIgnore(const std::string& fileName);
    static void GetValidFiles(std::vector<std::string>& validFiles);
    static bool CopyConfigFiles(const std::vector<std::string>& files);
    static bool CopyFile(const std::string& srcFile, const std::string& dstFile);
};
}
}
#endif // PARAM_MANAGER_H
