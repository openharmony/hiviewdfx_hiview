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
 
#include "content_reader_version_2.h"

#include "base_def.h"
#include "content_reader_factory.h"
#include "base/raw_data_base_def.h"
#include "hiview_logger.h"
#include "securec.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-ContentReaderVersion2");
REGISTER_CONTENT_READER(EventStore::EVENT_DATA_FORMATE_VERSION::VERSION2, ContentReaderVersion2);
namespace {
#pragma pack(1)
struct DocHeaderVersion2 {
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

struct ContentHeaderVersion2 {
    /* event seqno */
    uint64_t seq;

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

    /* Log packing flag */
    uint8_t log;
};
#pragma pack()
}

int ContentReaderVersion2::ReadDocDetails(std::ifstream& docStream, EventStore::DocHeader& header,
    uint64_t& docHeaderSize, std::string& sysVersion)
{
    if (!docStream.is_open()) {
        return DOC_STORE_ERROR_IO;
    }
    docStream.seekg(0, std::ios::beg);
    DocHeaderVersion2 docHeaderV2;
    docHeaderSize = sizeof(DocHeaderVersion2);
    docStream.read(reinterpret_cast<char*>(&docHeaderV2), docHeaderSize);
    header.magicNum = docHeaderV2.magicNum;
    header.blockSize = docHeaderV2.blockSize;
    header.pageSize = docHeaderV2.pageSize;
    header.version = docHeaderV2.version;
    if (memcpy_s(header.tag, MAX_TAG_LEN, docHeaderV2.tag, MAX_TAG_LEN) != EOK) {
        HIVIEW_LOGE("failed to copy tag to doc header");
        return DOC_STORE_ERROR_MEMORY;
    }
    sysVersion = "";  // system version was not stored in doc header with version 2
    return DOC_STORE_SUCCESS;
}

bool ContentReaderVersion2::IsValidMagicNum(const uint64_t magicNum)
{
    return magicNum == MAGIC_NUM_VERSION2;
}

int ContentReaderVersion2::GetContentHeader(uint8_t* content, EventStore::ContentHeader& header)
{
    if (content == nullptr) {
        return DOC_STORE_ERROR_NULL;
    }

    ContentHeaderVersion2 ContentHeaderV2 = *(reinterpret_cast<ContentHeaderVersion2*>(content + BLOCK_SIZE));
    header.timestamp = ContentHeaderV2.timestamp;
    header.timeZone = ContentHeaderV2.timeZone;
    header.uid = ContentHeaderV2.uid;
    header.pid = ContentHeaderV2.pid;
    header.tid = ContentHeaderV2.tid;
    header.id = ContentHeaderV2.id;
    header.type = ContentHeaderV2.type;
    header.isTraceOpened = ContentHeaderV2.isTraceOpened;
    header.log = ContentHeaderV2.log;
    return DOC_STORE_SUCCESS;
}

size_t ContentReaderVersion2::GetContentHeaderSize()
{
    return sizeof(ContentHeaderVersion2);
}
} // HiviewDFX
} // OHOS