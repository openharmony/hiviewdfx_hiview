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

#include "hiview_zip_util.h"

#include "file_util.h"
#include "hiview_logger.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
DEFINE_LOG_TAG("HiView-ZipUtil");
constexpr uLong MAX_ENTRY_NUM = 10000;
constexpr int MAX_UNZIP_SIZE = 300 * 1024 * 1024;
constexpr uint32_t BUFFER_SIZE = 100 * 1024;
constexpr size_t BLOCK_COUNT = 1;
constexpr int32_t ERROR_GET_HANDLE = 1001;
constexpr int32_t ERROR_READ_BYTES = 1002;
constexpr int32_t ERROR_INVALID_FILE = 1003;
constexpr int32_t ERROR_OPEN_NEW_FILE = 1004;
constexpr int32_t ERROR_CREATE_ZIP = 1005;
}

HiviewZipUnit::HiviewZipUnit(const std::string& zipPath, int32_t zipMode)
{
    zipFile_ = zipOpen(zipPath.c_str(), zipMode);
}

HiviewZipUnit::~HiviewZipUnit()
{
    if (zipFile_ != nullptr) {
        zipClose(zipFile_, nullptr);
        zipFile_ = nullptr;
    }
}

int32_t HiviewZipUnit::AddFileInZip(const std::string& srcFile, ZipFileLevel zipFileLevel)
{
    if (zipFile_ == nullptr) {
        return ERROR_CREATE_ZIP;
    }

    std::string realPath;
    FILE* srcFp = GetFileHandle(srcFile, realPath);
    if (srcFp == nullptr) {
        HIVIEW_LOGE("get file handle failed: %{public}s, errno: %{public}d", srcFile.c_str(), errno);
        return ERROR_GET_HANDLE;
    }

    std::string srcFileName(GetDstFilePath(realPath, zipFileLevel));
    if (srcFileName.empty()) {
        HIVIEW_LOGW("get target path failed.");
        (void)fclose(srcFp);
        return ERROR_INVALID_FILE;
    }
    if (zipOpenNewFileInZip(zipFile_, srcFileName.c_str(),
        nullptr, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK) {
        HIVIEW_LOGW("open new file in zip failed.");
        (void)fclose(srcFp);
        return ERROR_OPEN_NEW_FILE;
    }

    int32_t errCode = 0;
    char buf[BUFFER_SIZE] = { 0 };
    while (!feof(srcFp)) {
        size_t numBytes = fread(buf, 1, sizeof(buf), srcFp);
        if (numBytes < 0) {
            HIVIEW_LOGE("zip file failed, size is invalid.");
            errCode = ERROR_READ_BYTES;
            break;
        }
        zipWriteInFileInZip(zipFile_, buf, static_cast<unsigned int>(numBytes));
        if (ferror(srcFp)) {
            HIVIEW_LOGE("zip file failed, file: %{public}s, errno: %{public}d", srcFile.c_str(), errno);
            errCode = errno;
            break;
        }
    }
    (void)fclose(srcFp);
    zipCloseFileInZip(zipFile_);
    return errCode;
}

FILE* HiviewZipUnit::GetFileHandle(const std::string& file, std::string& realPath)
{
    if (!FileUtil::PathToRealPath(file, realPath)) {
        return nullptr;
    }
    return fopen(realPath.c_str(), "rb");
}

std::string HiviewZipUnit::GetDstFilePath(const std::string& srcFile, ZipFileLevel zipFileLevel)
{
    if (zipFileLevel == ZipFileLevel::KEEP_NONE_PARENT_PATH) {
        return FileUtil::ExtractFileName(srcFile);
    }

    std::string::size_type lastSlashPos = srcFile.rfind("/");
    if (lastSlashPos == std::string::npos) {
        return "";
    }
    std::string::size_type secondSlashPos = srcFile.substr(0, lastSlashPos).rfind("/");
    if (secondSlashPos == std::string::npos) {
        return "";
    }
    return srcFile.substr(secondSlashPos + 1);
}

HiviewUnzipUnit::HiviewUnzipUnit(const std::string& zipPath, const std::string& dstDir) : dstDir_(dstDir)
{
    zipFile_ = unzOpen(zipPath.c_str());
}

HiviewUnzipUnit::~HiviewUnzipUnit()
{
    if (zipFile_ != nullptr) {
        unzClose(zipFile_);
        zipFile_ = nullptr;
    }
}

bool HiviewUnzipUnit::UnzipFile() const
{
    if (zipFile_ == nullptr) {
        return false;
    }
    // Get info about the zip file
    unz_global_info globalInfo;
    if (unzGetGlobalInfo(zipFile_, &globalInfo) != UNZ_OK || globalInfo.number_entry > MAX_ENTRY_NUM) {
        HIVIEW_LOGW("entry num exceeds: %{public}lu", globalInfo.number_entry);
        return false;
    }

    int totalUnzipSize = 0;
    for (uLong index = 0; index < globalInfo.number_entry; ++index) {
        // Get info about current file
        unz_file_info fileInfo;
        char fileName[NAME_MAX] = { 0 };
        if (unzGetCurrentFileInfo(zipFile_, &fileInfo, fileName, sizeof(fileName), NULL, 0, NULL, 0) != UNZ_OK
            || fileInfo.size_filename >= NAME_MAX) {
            HIVIEW_LOGE("file numbers exceeded: %{public}d", static_cast<int>(fileInfo.size_filename));
            return false;
        }
        std::string path(dstDir_ + fileName);
        if (path.back() == '/') {
            // entry is a directory, so create it
            if (!FileUtil::ForceCreateDirectory(path)) {
                HIVIEW_LOGW("create dir failed.");
                return false;
            }
        } else {
            std::string filePath(FileUtil::ExtractFilePath(path));
            if (!FileUtil::ForceCreateDirectory(filePath)) {
                HIVIEW_LOGW("create dir for file failed.");
                return false;
            }
            int fileSize = 0;
            if (!DoUnzipFile(path, fileSize)) {
                HIVIEW_LOGW("unzip file failed.");
                return false;
            }
            totalUnzipSize += fileSize;
            if (totalUnzipSize > MAX_UNZIP_SIZE) {
                HIVIEW_LOGW("unzip file size exceeded.");
                return false;
            }
        }
        // go to the next entry listed in the zip file
        if ((index + 1) < globalInfo.number_entry && unzGoToNextFile(zipFile_) != UNZ_OK) {
            return false;
        }
    }
    return true;
}

bool HiviewUnzipUnit::DoUnzipFile(const std::string& path, int& fileSize) const
{
    if (unzOpenCurrentFile(zipFile_) != UNZ_OK) {
        HIVIEW_LOGE("unzip current file failed.");
        return false;
    }

    FILE* outFile = fopen(path.c_str(), "wb");
    if (outFile == nullptr) {
        HIVIEW_LOGW("create zip sub file failed.");
        unzCloseCurrentFile(zipFile_);
        return false;
    }

    int readSize = 0;
    bool isSuccess = true;
    char readBuffer[BUFFER_SIZE] = { 0 };
    do {
        readSize = unzReadCurrentFile(zipFile_, readBuffer, sizeof(readBuffer));
        if (readSize < 0) {
            HIVIEW_LOGE("read file from zip failed.");
            isSuccess = false;
            break;
        }
        if (readSize > 0) {
            if (fwrite(readBuffer, readSize, BLOCK_COUNT, outFile) != BLOCK_COUNT) {
                HIVIEW_LOGE("write date to file failed.");
                isSuccess = false;
                break;
            }
            fileSize += readSize;
            if (fileSize > MAX_UNZIP_SIZE) {
                HIVIEW_LOGE("unzip readSize exceeded.");
                isSuccess = false;
                break;
            }
        }
    } while (readSize > 0);
    if (fclose(outFile) != 0) {
        HIVIEW_LOGE("close file failed.");
        isSuccess = false;
    }
    unzCloseCurrentFile(zipFile_);
    return isSuccess;
}
} // HiviewDFX
} // OHOS