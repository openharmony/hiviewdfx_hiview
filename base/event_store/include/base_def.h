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
#ifndef HIVIEW_BASE_EVENT_STORE_INCLUDE_BASE_DEF_H
#define HIVIEW_BASE_EVENT_STORE_INCLUDE_BASE_DEF_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
#define MAGIC_NUM 0x894556454E541a0a
#define NUM_OF_BYTES_IN_KB 1024
#define NUM_OF_BYTES_IN_MB (1024 * 1024)
#define BLOCK_SIZE sizeof(uint32_t)
#define HEADER_SIZE sizeof(DocHeader)
#define DOC_EVENT_HEADER_SIZE sizeof(DocEventHeader)

#define MAX_TAG_LEN 17

#define MAX_NEW_SIZE (386 * 1024)

#define EVENT_NAME_INDEX 0
#define EVENT_TYPE_INDEX 1
#define EVENT_LEVEL_INDEX 2
#define EVENT_SEQ_INDEX 3
#define FILE_NAME_SPLIT_SIZE 4

#define INVALID_VALUE_INT (-1)

#define DOC_STORE_SUCCESS 0
#define DOC_STORE_NEW_FILE 1
#define DOC_STORE_READ_EMPTY 2
#define DOC_STORE_ERROR_NULL (-1)
#define DOC_STORE_ERROR_IO (-2)
#define DOC_STORE_ERROR_MEMORY (-3)
#define DOC_STORE_ERROR_INVALID (-4)

const char FILE_NAME_SEPARATOR[] = "-";
const char FILE_SEPARATOR[] = "/";
const char FILE_EXT[] = ".db";

/* Object returned by the event query */
struct Entry {
    /* Event sequence */
    int64_t id = 0;

    /* Event timestamp */
    int64_t ts = 0;

    /* Event json string */
    std::string value;
};
using Entry = struct Entry;

/* File header of the binary storage file */
struct DocHeader {
    /* Magic number */
    uint64_t magicNum = 0;

    /* Page size */
    uint8_t pageSize = 0;

    /* Version number */
    uint8_t version = 0;

    /* Event tag */
    char tag[MAX_TAG_LEN] = {0};
};
using DocHeader = struct DocHeader;

/* Event content header for each event in the binary storage file */
struct DocEventHeader {
    int64_t seq = 0;
    uint64_t timestamp = 0;
    uint8_t timeZone = 0;
    uint32_t uid = 0;
    uint32_t pid = 0;
    uint32_t tid = 0;
    uint64_t id = 0;
    uint8_t isTraceOpened : 1;
};
using DocEventHeader = struct DocEventHeader;

/* Common event information read from the binary storage file */
struct CommonEventInfo {
    std::string domain;
    std::string name;
    std::string level;
    std::string tag;
};
using CommonEventInfo = struct CommonEventInfo;
} // EventStore
} // HiviewDFX
} // OHOS
#endif // HIVIEW_BASE_EVENT_STORE_INCLUDE_BASE_DEF_H
