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

#include "file_helper.h"

#include <fstream>
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002699

#undef LOG_TAG
#define LOG_TAG "file_helper"

namespace OHOS {
namespace HiviewDFX {
namespace Graphic {

bool FileHelper::ReadFileByLine(const std::string &filePath, const DataHandler &func)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        HILOG_ERROR(LOG_CORE, "file[%{public}s] open failed.", filePath.c_str());
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (func(line)) {
            break;
        }
    }
    file.close();
    return true;
}

} // namespace Graphic
} // namespace HiviewDFX
} // namespace OHOS