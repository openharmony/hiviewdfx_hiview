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
#ifndef FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_ZIP_UTILS_H
#define FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_ZIP_UTILS_H
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include "hilog/log.h"
#include <iostream>
#include "securec.h"
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <zip.h>

#include "file_util.h"

namespace OHOS {
namespace HiviewDFX {
const int READ_MORE_LENGTH = 102400;
constexpr int32_t ERR_CODE = -1;

zipFile CreateZipFile(const std::string& zipPath);
void CloseZipFile(zipFile& zipfile);
FILE* GetFileHandle(const std::string& file);
int32_t AddFileInZip(zipFile& zipfile, const std::string& srcFile);
bool PackFiles(const std::string& fileName, const std::string& zipFileName);
} // HiViewDFX
} // OHOS
#endif // FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_ZIP_UTILS_H