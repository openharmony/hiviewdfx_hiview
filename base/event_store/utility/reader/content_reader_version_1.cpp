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
 
#include "content_reader_version_1.h"

#include "base_def.h"
#include "content_reader_factory.h"
#include "base/raw_data_base_def.h"
#include "hiview_logger.h"
#include "securec.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-ContentReaderVersion1");
REGISTER_CONTENT_READER(EventStore::EVENT_DATA_FORMATE_VERSION::VERSION1, ContentReaderVersion1);
namespace {
#pragma pack(1)
struct DocHeaderVersion1 {
    /* Magic number */
    uint64_t magicNum = 0;

    /* Block size */
    uint32_t blockSize = 0;

    /* Page size */
    uint8_t pageSize = 0;

    /* Version number */
    uint8_t version = 0;

    /* Event tag */
    char tag[MAX_TAG_LEN] = {0};

    /* version block size */
    uint32_t crc = 0;
};

struct ContentHeaderVersion1 {
    /* Event Seq */
    int64_t seq;

    /* Event timestamp */
    uint64_t timestamp;

    /* Time zone */
    uint8_t timeZone;

    /* User id */
    uint32_t uid;

    /* Process id */
    uint32_t pid;

    /* Thread id */
    uint32_t tid;

    /* Event hash code*/
    uint64_t id;

    /* Event type */
    uint8_t type : 2;

    /* Trace info flag*/
    uint8_t isTraceOpened : 1;
};
#pragma pack()
}

int ContentReaderVersion1::ReadDocDetails(std::ifstream& docStream, EventStore::DocHeader& header,
    uint64_t& docHeaderSize, HeadExtraInfo& headExtra)
{
    if (!docStream.is_open()) {
        return DOC_STORE_ERROR_IO;
    }
    docStream.seekg(0, std::ios::beg);
    docHeaderSize = sizeof(DocHeaderVersion1);
    DocHeaderVersion1 docHeaderV1;
    docStream.read(reinterpret_cast<char*>(&docHeaderV1), docHeaderSize);
    header.magicNum = docHeaderV1.magicNum;
    header.blockSize = docHeaderV1.blockSize;
    header.pageSize = docHeaderV1.pageSize;
    header.version = docHeaderV1.version;
    if (memcpy_s(header.tag, MAX_TAG_LEN, docHeaderV1.tag, MAX_TAG_LEN) != EOK) {
        HIVIEW_LOGE("failed to copy tag to doc header");
        return DOC_STORE_ERROR_MEMORY;
    }
    
    headExtra.sysVersion = "";  // system version was not stored in doc header in version 1
    return DOC_STORE_SUCCESS;
}

bool ContentReaderVersion1::IsValidMagicNum(const uint64_t magicNum)
{
    return magicNum == MAGIC_NUM_VERSION1;
}

int ContentReaderVersion1::GetContentHeader(uint8_t* content, EventStore::ContentHeader& header)
{
    if (content == nullptr) {
        return DOC_STORE_ERROR_NULL;
    }

    ContentHeaderVersion1 contentHeaderV1 = *(reinterpret_cast<ContentHeaderVersion1*>(content + HIVIEW_BLOCK_SIZE));
    header.timestamp = contentHeaderV1.timestamp;
    header.timeZone = contentHeaderV1.timeZone;
    header.uid = contentHeaderV1.uid;
    header.pid = contentHeaderV1.pid;
    header.tid = contentHeaderV1.tid;
    header.id = contentHeaderV1.id;
    header.type = contentHeaderV1.type;
    header.isTraceOpened = contentHeaderV1.isTraceOpened;
    return DOC_STORE_SUCCESS;
}

size_t ContentReaderVersion1::GetContentHeaderSize()
{
    return sizeof(ContentHeaderVersion1);
}
} // HiviewDFX
} // OHOS