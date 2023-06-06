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

#ifndef OHOS_HIVIEWDFX_DATA_PUBLISHER_SYS_EVENT_CALLBACK_H
#define OHOS_HIVIEWDFX_DATA_PUBLISHER_SYS_EVENT_CALLBACK_H

#include <string>
#include <vector>

#include "iquery_base_callback.h"

namespace OHOS {
namespace HiviewDFX {
class DataPublisherSysEventCallback : public IQueryBaseCallback {

public:
    DataPublisherSysEventCallback(std::string srcPath, std::string destPath,
        int32_t fileIndex, int32_t totalJsonSize): srcPath_(srcPath),
        destPath_(destPath), fileIndex_(fileIndex), totalJsonSize_(totalJsonSize){};
    ~DataPublisherSysEventCallback() {};
    void OnQuery(const std::vector<std::u16string>& sysEvent, const std::vector<int64_t>& seq) override;
    void OnComplete(int32_t reason, int32_t total, int64_t seq) override;
    void HandleEventFile(const std::string &srcPath, const std::string &desPath);

private:
    std::string srcPath_;
    std::string destPath_;
    std::string lastDestFilePath_;
    int32_t fileIndex_;
    int32_t totalJsonSize_;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_DATA_PUBLISHER_SYS_EVENT_CALLBACK_H

