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

namespace OHOS {
namespace HiviewDFX {
REGISTER_CONTENT_READER(EventRaw::EVENT_DATA_FORMATE_VERSION::VERSION1, ContentReaderVersion1);
namespace {
#pragma pack(1)
struct Version1ContentHeader {
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

int ContentReaderVersion1::GetContentHead(uint8_t* content, EventStore::ContentHeader& head)
{
    if (content == nullptr) {
        return DOC_STORE_ERROR_NULL;
    }

    Version1ContentHeader curHead = *(reinterpret_cast<Version1ContentHeader*>(content + BLOCK_SIZE));
    head.timestamp = curHead.timestamp;
    head.timeZone = curHead.timeZone;
    head.uid = curHead.uid;
    head.pid = curHead.pid;
    head.tid = curHead.tid;
    head.id = curHead.id;
    head.type = curHead.type;
    head.isTraceOpened = curHead.isTraceOpened;
    return DOC_STORE_SUCCESS;
}

size_t ContentReaderVersion1::GetHeaderSize()
{
    return sizeof(Version1ContentHeader);
}
} // HiviewDFX
} // OHOS