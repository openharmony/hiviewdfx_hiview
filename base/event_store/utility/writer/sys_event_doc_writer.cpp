/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "sys_event_doc_writer.h"

#include "crc_generator.h"
#include "event_store_config.h"
#include "logger.h"
#include "securec.h"
#include "sys_event_doc_reader.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
DEFINE_LOG_TAG("HiView-SysEventDocWriter");
namespace {
constexpr uint32_t HEADER_BLOCK_SIZE = 27;
constexpr uint32_t CRC_INIT_VALUE = 0x0;
constexpr uint32_t RAW_DATA_OFFSET = BLOCK_SIZE + MAX_DOMAIN_LEN + MAX_EVENT_NAME_LEN;

template<typename T>
int CopyValue(uint8_t* dst, size_t dstSize, T value)
{
    return memcpy_s(dst, dstSize, &value, sizeof(T));
}

int CopyValue(uint8_t* dst, size_t dstSize, const uint8_t* src, size_t count)
{
    return memcpy_s(dst, dstSize, src, count);
}
}
SysEventDocWriter::SysEventDocWriter(const std::string& path): EventDocWriter(path)
{
    out_.open(path, std::ios::binary | std::ios::app);
}

SysEventDocWriter::~SysEventDocWriter()
{
    if (out_.is_open()) {
        out_.close();
    }
}

int SysEventDocWriter::Write(const std::shared_ptr<SysEvent>& sysEvent)
{
    if (sysEvent == nullptr) {
        HIVIEW_LOGE("event is null");
        return DOC_STORE_ERROR_NULL;
    }
    if (!out_.is_open()) {
        HIVIEW_LOGE("file=%{public}s is not open", docPath_.c_str());
        return DOC_STORE_ERROR_IO;
    }
    uint32_t contentSize = 0;
    if (int ret = GetContentSize(sysEvent, contentSize); ret != DOC_STORE_SUCCESS) {
        return ret;
    }
    SysEventDocReader reader(docPath_);
    int fileSize = reader.ReadFileSize();
    if (fileSize < 0) {
        HIVIEW_LOGE("failed to get the size of file=%{public}s", docPath_.c_str());
        return DOC_STORE_ERROR_IO;
    }

    // if file is empty, write header to the file first
    if (fileSize == 0) {
        if (auto ret = WriteHeader(sysEvent, contentSize); ret != DOC_STORE_SUCCESS) {
            return ret;
        }
        return WriteContent(sysEvent, contentSize);
    }

    DocHeader header;
    reader.ReadHeader(header);
    if (header.version != EventRaw::EVENT_DATA_FORMATE_VERSION::DEFAULT_DATA_VERSION) {
        return DOC_STORE_NEW_FILE;
    }

    // if file is not empty, read the file header for writing
    uint32_t pageSize = 0;
    if (int ret = reader.ReadPageSize(pageSize); ret != DOC_STORE_SUCCESS) {
        HIVIEW_LOGE("failed to get pageSize from the file=%{public}s", docPath_.c_str());
        return ret;
    }

    // if the current file is full, need to create a new file
    if (pageSize == 0 || pageSize < contentSize) {
        HIVIEW_LOGD("the current page is full, page=%{public}u, content=%{public}u", pageSize, contentSize);
        return DOC_STORE_NEW_FILE;
    }

    // if the current page is full, need to add zero
    if (auto remainSize = GetCurrPageRemainSize(fileSize, pageSize); remainSize < contentSize) {
        if (int ret = FillCurrPageWithZero(remainSize); ret != DOC_STORE_SUCCESS) {
            return ret;
        }
    }
    return WriteContent(sysEvent, contentSize);
}

uint32_t SysEventDocWriter::GetCurrPageRemainSize(int fileSize, uint32_t pageSize)
{
    return (pageSize - ((static_cast<uint32_t>(fileSize) - HEADER_SIZE) % pageSize));
}

int SysEventDocWriter::FillCurrPageWithZero(uint32_t remainSize)
{
    if (remainSize == 0) {
        return DOC_STORE_SUCCESS;
    }
    if (remainSize > MAX_NEW_SIZE) {
        HIVIEW_LOGE("invalid new size=%{public}u", remainSize);
        return DOC_STORE_ERROR_MEMORY;
    }
    uint8_t* fillData = new(std::nothrow) uint8_t[remainSize]{ 0x0 };
    if (fillData == nullptr) {
        HIVIEW_LOGE("failed to new memory for fillData, size=%{public}u", remainSize);
        return DOC_STORE_ERROR_MEMORY;
    }
    out_.write(reinterpret_cast<char*>(fillData), remainSize);
    delete[] fillData;
    return DOC_STORE_SUCCESS;
}

int SysEventDocWriter::GetContentSize(const std::shared_ptr<SysEvent>& sysEvent, uint32_t& contentSize)
{
    if (sysEvent->AsRawData() == nullptr) {
        HIVIEW_LOGE("The raw data of event is null");
        return DOC_STORE_ERROR_NULL;
    }
    uint32_t dataSize = *(reinterpret_cast<uint32_t*>(sysEvent->AsRawData()));
    if (dataSize < RAW_DATA_OFFSET) {
        HIVIEW_LOGE("The length=%{public}u of raw data is invalid", dataSize);
        return DOC_STORE_ERROR_INVALID;
    }
    contentSize = dataSize - RAW_DATA_OFFSET + BLOCK_SIZE + SEQ_SIZE + CRC_SIZE;
    return DOC_STORE_SUCCESS;
}

int SysEventDocWriter::WriteHeader(const std::shared_ptr<SysEvent>& sysEvent, uint32_t contentSize)
{
    uint32_t pageSize = EventStoreConfig::GetInstance().GetPageSize(sysEvent->eventType_);
    if (pageSize == 0) {
        HIVIEW_LOGE("failed to get page size");
        return DOC_STORE_ERROR_IO;
    }
    pageSize = contentSize > (pageSize * NUM_OF_BYTES_IN_KB) ? 0 : pageSize;

    DocHeader header = {
        .magicNum = MAGIC_NUM,
        .blockSize = HEADER_BLOCK_SIZE,
        .pageSize = pageSize,
        .version = EventRaw::EVENT_DATA_FORMATE_VERSION::DEFAULT_DATA_VERSION,
        .crc = CRC_INIT_VALUE,
    };
    if (!sysEvent->GetTag().empty() && strcpy_s(header.tag, MAX_TAG_LEN, sysEvent->GetTag().c_str()) != EOK) {
        HIVIEW_LOGW("failed to copy tag to event, tag=%{public}s", sysEvent->GetTag().c_str());
    }
    header.crc = CrcGenerator::GetCrc32(reinterpret_cast<uint8_t*>(&header), HEADER_SIZE - CRC_SIZE);
    out_.write(reinterpret_cast<char*>(&header), HEADER_SIZE);
    return DOC_STORE_SUCCESS;
}

int SysEventDocWriter::WriteContent(const std::shared_ptr<SysEvent>& sysEvent, uint32_t contentSize)
{
    uint8_t* content = nullptr;
    if (int ret = BuildContent(sysEvent, &content, contentSize); ret != DOC_STORE_SUCCESS) {
        return ret;
    }
    out_.seekp(0, std::ios::end); // move to the end
    out_.write(reinterpret_cast<char*>(content), contentSize);
    delete[] content;
    out_.flush();
    HIVIEW_LOGD("write content size=%{public}u, seq=%{public}" PRId64 ", file=%{public}s", contentSize,
        sysEvent->GetEventSeq(), docPath_.c_str());
    return DOC_STORE_SUCCESS;
}

int SysEventDocWriter::BuildContent(const std::shared_ptr<SysEvent>& sysEvent,
    uint8_t** contentPtr, uint32_t contentSize)
{
    if (contentSize > MAX_NEW_SIZE) {
        HIVIEW_LOGE("invalid new content size=%{public}u", contentSize);
        return DOC_STORE_ERROR_MEMORY;
    }
    uint8_t* content = new(std::nothrow) uint8_t[contentSize];
    if (content == nullptr) {
        HIVIEW_LOGE("failed to new memory for content, size=%{public}u", contentSize);
        return DOC_STORE_ERROR_MEMORY;
    }
    if (CopyValue(content, contentSize, contentSize) != EOK) {
        HIVIEW_LOGE("failed to write contentSize=%{public}u", contentSize);
        delete[] content;
        return DOC_STORE_ERROR_MEMORY;
    }
    uint32_t pos = sizeof(contentSize);
    if (CopyValue(content + pos, contentSize, sysEvent->GetEventSeq()) != EOK) {
        HIVIEW_LOGE("failed to write seq=%{public}" PRId64, sysEvent->GetEventSeq());
        delete[] content;
        return DOC_STORE_ERROR_MEMORY;
    }
    pos += sizeof(sysEvent->GetEventSeq());
    uint8_t* rawData = sysEvent->AsRawData();
    uint32_t dataSize = *(reinterpret_cast<uint32_t*>(rawData)) - RAW_DATA_OFFSET;
    if (CopyValue(content + pos, contentSize, rawData + RAW_DATA_OFFSET, dataSize) != EOK) {
        HIVIEW_LOGE("failed to write event data");
        delete[] content;
        return DOC_STORE_ERROR_MEMORY;
    }
    pos += dataSize;
    uint32_t crc = CrcGenerator::GetCrc32(reinterpret_cast<uint8_t*>(content), contentSize - CRC_SIZE);
    if (CopyValue(content + pos, contentSize, crc) != EOK) {
        HIVIEW_LOGE("failed to write crc=%{public}u", crc);
        delete[] content;
        return DOC_STORE_ERROR_MEMORY;
    }
    *contentPtr = content;
    return DOC_STORE_SUCCESS;
}
} // EventStore
} // HiviewDFX
} // OHOS
