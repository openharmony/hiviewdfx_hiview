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
#include "zip_utils.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D03, "Hiview-ZipUtils" };

zipFile CreateZipFile(const std::string& zipPath)
{
    return zipOpen(zipPath.c_str(), APPEND_STATUS_CREATE);
}

void CloseZipFile(zipFile& zipfile)
{
    zipClose(zipfile, nullptr);
}

FILE* GetFileHandle(const std::string& file)
{
    std::string realPath;
    if (!FileUtil::PathToRealPath(file, realPath)) {
        return nullptr;
    }
    return fopen(realPath.c_str(), "rb");
}

int32_t AddFileInZip(zipFile& zipfile, const std::string& srcFile)
{
    zip_fileinfo zipInfo;
    errno_t result = memset_s(&zipInfo, sizeof(zipInfo), 0, sizeof(zipInfo));
    if (result != EOK) {
        HiLog::Error(LABEL, "AddFileInZip memset_s error, file:%{public}s.", srcFile.c_str());
        return ERR_CODE;
    }
    FILE *srcFp = GetFileHandle(srcFile);
    if (srcFp == nullptr) {
        HiLog::Error(LABEL, "get file handle failed:%{public}s, errno: %{public}d.", srcFile.c_str(), errno);
        return ERR_CODE;
    }

    zipOpenNewFileInZip(zipfile, srcFile.c_str(), &zipInfo,
        nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION);

    int errcode = 0;
    char buf[READ_MORE_LENGTH] = {0};
    while (!feof(srcFp)) {
        size_t numBytes = fread(buf, 1, sizeof(buf), srcFp);
        if (numBytes == 0) {
            HiLog::Error(LABEL, "zip file failed, size is zero.");
            errcode = ERR_CODE;
            break;
        }
        zipWriteInFileInZip(zipfile, buf, static_cast<unsigned int>(numBytes));
        if (ferror(srcFp)) {
            HiLog::Error(LABEL, "zip file failed:%{public}s, errno: %{public}d.", srcFile.c_str(), errno);
            errcode = ERR_CODE;
            break;
        }
    }
    (void)fclose(srcFp);
    zipCloseFileInZip(zipfile);
    return errcode;
}

bool PackFiles(const std::string& fileName, const std::string& zipFileName)
{
    HiLog::Info(LABEL, "start pack file %{public}s to %{public}s.", fileName.c_str(), zipFileName.c_str());
    zipFile compressZip = CreateZipFile(zipFileName);
    if (compressZip == nullptr) {
        HiLog::Error(LABEL, "create zip file failed.");
        return false;
    }
    AddFileInZip(compressZip, fileName);
    CloseZipFile(compressZip);
    return true;
}
} // HiViewDFX
} // OHOS
