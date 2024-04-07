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
 
#include "content_reader.h"

#include "logger.h"
#include "securec.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-ContentReader");
namespace {
constexpr uint32_t DATA_FORMAT_VERSION_OFFSET =
    sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint8_t); // magicNumber + blockSize + pageSize
}

uint8_t ContentReader::ReadFmtVersion(std::ifstream& docStream)
{
    if (!docStream.is_open()) {
        return EventStore::EVENT_DATA_FORMATE_VERSION::INVALID;
    }
    auto curPos = docStream.tellg();
    docStream.seekg(DATA_FORMAT_VERSION_OFFSET, std::ios::beg);
    uint8_t dataFmtVersion = EventStore::EVENT_DATA_FORMATE_VERSION::INVALID;
    docStream.read(reinterpret_cast<char*>(&dataFmtVersion), sizeof(uint8_t));
    docStream.seekg(curPos, std::ios::beg);
    return dataFmtVersion;
}

int ContentReader::WriteDomainName(const EventInfo& docInfo, uint8_t** event, uint32_t eventSize)
{
    if (memcpy_s(*event, eventSize, reinterpret_cast<char*>(&eventSize), BLOCK_SIZE) != EOK) {
        HIVIEW_LOGE("failed to copy block size to raw event");
        return DOC_STORE_ERROR_MEMORY;
    }
    uint32_t eventPos = BLOCK_SIZE;
    if (memcpy_s(*event + eventPos, eventSize - eventPos, docInfo.domain.c_str(),
        docInfo.domain.length() + 1) != EOK) {
        HIVIEW_LOGE("failed to copy domain to raw event");
        return DOC_STORE_ERROR_MEMORY;
    }
    eventPos += MAX_DOMAIN_LEN;
    if (memcpy_s(*event + eventPos, eventSize - eventPos, docInfo.name.c_str(),
        docInfo.name.length() + 1) != EOK) {
        HIVIEW_LOGE("failed to copy name to raw event");
        return DOC_STORE_ERROR_MEMORY;
    }
    return DOC_STORE_SUCCESS;
}

int ContentReader::ReadRawEvent(const EventInfo& docInfo, uint8_t** rawEvent, uint32_t& eventSize,
    uint8_t* content, uint32_t contentSize)
{
    EventStore::ContentHeader contentHeader;
    GetContentHeader(content, contentHeader);
    // the different size of event header between old and newer version.
    eventSize = contentSize - GetContentHeaderSize() + sizeof(EventStore::ContentHeader) -
        SEQ_SIZE + MAX_DOMAIN_LEN + MAX_EVENT_NAME_LEN;
    if (eventSize > MAX_NEW_SIZE) {
        HIVIEW_LOGE("invalid new event size=%{public}u", eventSize);
        return DOC_STORE_ERROR_MEMORY;
    }
    uint8_t* event = new(std::nothrow) uint8_t[eventSize];
    if (event == nullptr) {
        HIVIEW_LOGE("failed to allocate new memory for raw event, size=%{public}u", eventSize);
        return DOC_STORE_ERROR_MEMORY;
    }
    int ret = WriteDomainName(docInfo, &event, eventSize);
    if (ret != DOC_STORE_SUCCESS) {
        delete[] event;
        return ret;
    }
    size_t eventPos = BLOCK_SIZE + MAX_DOMAIN_LEN + MAX_EVENT_NAME_LEN;
    if (memcpy_s(event + eventPos, eventSize - eventPos, reinterpret_cast<uint8_t*>(&contentHeader) + SEQ_SIZE,
        sizeof(EventStore::ContentHeader) - SEQ_SIZE) != EOK) {
        HIVIEW_LOGE("failed to copy event content header to raw event");
        delete[] event;
        return DOC_STORE_ERROR_MEMORY;
    }
    eventPos += (sizeof(EventStore::ContentHeader) - SEQ_SIZE);
    size_t contentPos = BLOCK_SIZE + GetContentHeaderSize();
    if (memcpy_s(event + eventPos, eventSize - eventPos, content + contentPos, contentSize - contentPos) != EOK) {
        HIVIEW_LOGE("failed to copy customized parameters to raw event");
        delete[] event;
        return DOC_STORE_ERROR_MEMORY;
    }
    *rawEvent = event;
    return DOC_STORE_SUCCESS;
}
} // HiviewDFX
} // OHOS