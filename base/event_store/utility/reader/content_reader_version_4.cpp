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
 
#include "content_reader_version_4.h"

#include "base_def.h"
#include "content_reader_factory.h"
#include "base/raw_data_base_def.h"
#include "hiview_logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-ContentReaderVersion4");
REGISTER_CONTENT_READER(EventStore::EVENT_DATA_FORMATE_VERSION::VERSION4, ContentReaderVersion4);
int ContentReaderVersion4::ReadDocDetails(std::ifstream& docStream, EventStore::DocHeader& header,
    uint64_t& docHeaderSize, HeadExtraInfo& headExtra)
{
    if (!docStream.is_open()) {
        return DOC_STORE_ERROR_IO;
    }
    docStream.seekg(0, std::ios::beg);
    docStream.read(reinterpret_cast<char*>(&header), sizeof(EventStore::DocHeader));
    if (!GetDataString(docStream, headExtra.sysVersion)) {
        HIVIEW_LOGE("system version invalid.");
        return DOC_STORE_ERROR_INVALID;
    }

    if (!GetDataString(docStream, headExtra.patchVersion)) {
        HIVIEW_LOGE("patch version invalid.");
        return DOC_STORE_ERROR_INVALID;
    }
    HIVIEW_LOGD("systemVersion:%{public}s, patchVersion:%{public}s",
        headExtra.sysVersion.c_str(), headExtra.patchVersion.c_str());
    docHeaderSize = header.blockSize + sizeof(header.magicNum);
    return DOC_STORE_SUCCESS;
}
} // HiviewDFX
} // OHOS