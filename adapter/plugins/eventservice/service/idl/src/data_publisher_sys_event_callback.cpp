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

#include "data_publisher_sys_event_callback.h"

#include <unistd.h>

#include "errors.h"
#include "hilog/log.h"

#include "data_share_common.h"
#include "data_share_util.h"
#include "file_util.h"
#include "string_util.h"

namespace OHOS {
namespace HiviewDFX {

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D10, "HiView-DataPublisherSysEventCallback" };
}  // namespace

void DataPublisherSysEventCallback::OnQuery(const std::vector<std::u16string>& sysEvent,
    const std::vector<int64_t>& seq)
{
    lastDestFilePath_ = destPath_;
    lastDestFilePath_.append("-")
        .append(SUCCESS_CODE)
        .append("-")
        .append(std::to_string(fileIndex_))
        .append(FILE_SUFFIX);
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    for (auto iter: sysEvent) {
        int32_t eventJsonSize = static_cast<int32_t>((iter.size() + 1) * sizeof(std::u16string));
        if (eventJsonSize + totalJsonSize_ > MAXIMUM_FILE_SIZE) {
            HandleEventFile(srcPath_, lastDestFilePath_);
            fileIndex_++;
            lastDestFilePath_ = destPath_;
            lastDestFilePath_.append("-")
                .append(SUCCESS_CODE)
                .append("-")
                .append(std::to_string(fileIndex_))
                .append(FILE_SUFFIX);
            totalJsonSize_ = 0;
        }
        std::string str = convert.to_bytes(iter);
        if (!FileUtil::SaveStringToFile(srcPath_, str + ",", false)) {
            HiLog::Error(LABEL, "failed to persist iter to file");
        }
        totalJsonSize_ += eventJsonSize;
    }
}

void DataPublisherSysEventCallback::OnComplete(int32_t reason, int32_t total, int64_t seq)
{
    if (totalJsonSize_ != 0) {
        HandleEventFile(srcPath_, lastDestFilePath_);
    }
}

void DataPublisherSysEventCallback::HandleEventFile(const std::string &srcPath, const std::string &desPath)
{
    auto res = OHOS::HiviewDFX::DataShareUtil::CopyFile(srcPath.c_str(), desPath.c_str());
    if (res == -1) {
        HiLog::Error(LABEL, "failed to move file to desPath.");
    }
    if (!FileUtil::RemoveFile(srcPath)) {
        HiLog::Error(LABEL, "failed to remove resourceFile.");
    }
    if (chmod(desPath.c_str(), FileUtil::FILE_PERM_666)) {
        HiLog::Error(LABEL, "Failed to chmod socket.");
    }
}
} // namespace HiviewDFX
} // namespace OHOS

