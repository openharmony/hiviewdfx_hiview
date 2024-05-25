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

#ifndef UTILITY_HIVIEW_ZIP_UTIL_H
#define UTILITY_HIVIEW_ZIP_UTIL_H

#include <contrib/minizip/unzip.h>
#include <contrib/minizip/zip.h>
#include <string>

namespace OHOS {
namespace HiviewDFX {
enum class ZipFileLevel {
    KEEP_NONE_PARENT_PATH,
    KEEP_ONE_PARENT_PATH
};

class HiviewZipUnit {
public:
    HiviewZipUnit(const std::string& zipPath, int32_t zipMode = APPEND_STATUS_CREATE);
    ~HiviewZipUnit();

    bool isValid() const { return zipFile_ != nullptr; }
    int32_t AddFileInZip(const std::string& srcFile, ZipFileLevel zipFileLevel);

private:
    std::string GetDstFilePath(const std::string& srcFile, ZipFileLevel zipFileLevel);
    FILE* GetFileHandle(const std::string& file, std::string& realPath);

private:
    zipFile zipFile_ { nullptr };
};

class HiviewUnzipUnit {
public:
    HiviewUnzipUnit(const std::string& zipPath, const std::string& dstDir);
    ~HiviewUnzipUnit();

    bool UnzipFile() const;

private:
    bool DoUnzipFile(const std::string& path, int& fileSize) const;

private:
    unzFile zipFile_ { nullptr };
    std::string dstDir_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // UTILITY_HIVIEW_ZIP_UTIL_H