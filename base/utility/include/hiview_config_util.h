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

#ifndef UTILITY_HIVIEW_CFG_UTIL_H
#define UTILITY_HIVIEW_CFG_UTIL_H

#include <functional>
#include <string>

namespace OHOS {
namespace HiviewDFX {
namespace HiViewConfigUtil {
// fileHandleRet(srcConfigDir, destConfigDir, configFileName)
using ConfigFileHandler = std::function<bool(const std::string&, const std::string&, const std::string&)>;

std::string GetConfigFilePath(const std::string& configFileName);
std::string GetConfigFilePath(const std::string& configZipFileName, const std::string& configDir,
    const std::string configFileName);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // UTILITY_HIVIEW_CFG_UTIL_H