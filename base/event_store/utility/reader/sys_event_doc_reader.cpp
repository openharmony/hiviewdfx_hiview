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
#include "sys_event_doc_reader.h"

#include <cinttypes>

#include "crc_generator.h"
#include "decoded/decoded_event.h"
#include "logger.h"
#include "securec.h"
#include "string_util.h"
#include "sys_event_query.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
DEFINE_LOG_TAG("HiView-SysEventDocReader");
namespace {
template<typename T>
int32_t GetNegativeNum(T num)
{
    return static_cast<int32_t>(~(num - 1)); // 1 for binary conversion
}

template<typename T>
void ReadValueAndReset(std::ifstream& in, T& value)
{
    in.read(reinterpret_cast<char*>(&value), sizeof(T));
    in.seekg(GetNegativeNum(sizeof(T)), std::ios::cur);
}

template<typename T>
void ReadValue(std::ifstream& in, T& value)
{
    in.read(reinterpret_cast<char*>(&value), sizeof(T));
}

template<typename T>
void AppendJsonValue(std::string& eventJson, const std::string& key, T val)
{
    std::string appendStr;
    appendStr.append(",\"").append(key).append("\":");
    if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
        appendStr.append("\"").append(val).append("\"");
    } else {
        appendStr.append(std::to_string(val));
    }
    eventJson.insert(eventJson.size() - 1, appendStr); // 1 for '}'
}
}

SysEventDocReader::SysEventDocReader(const std::string& path): EventDocReader(path),
    fileSize_(INVALID_VALUE_INT), pageSize_(0)
{
    Init(path);
}

SysEventDocReader::~SysEventDocReader()
{
    if (in_.is_open()) {
        in_.close();
    }
}

void SysEventDocReader::Init(const std::string& path)
{
    in_.open(path, std::ios::binary);

    // get domain, name and level from file path
    std::vector<std::string> dirNames;
    StringUtil::SplitStr(path, "/", dirNames);
    constexpr size_t domainOffset = 2;
    if (dirNames.size() >= domainOffset) {
        domain_ = dirNames[dirNames.size() - domainOffset];
        std::string file = dirNames.back();
        std::vector<std::string> fileNames;
        StringUtil::SplitStr(file, "-", fileNames);
        if (fileNames.size() == FILE_NAME_SPLIT_SIZE) {
            name_ = fileNames[EVENT_NAME_INDEX];
            level_ = fileNames[EVENT_LEVEL_INDEX];
        }
    }

    // get file size
    if (in_.is_open()) {
        auto curPos = in_.tellg();
        in_.seekg(0, std::ios::end);
        fileSize_ = in_.tellg();
        in_.seekg(curPos, std::ios::beg);
    }
}

int SysEventDocReader::Read(const DocQuery& query, EntryQueue& entries, int& num)
{
    // read the header
    DocHeader header;
    if (auto ret = ReadHeader(header); ret != DOC_STORE_SUCCESS) {
        return ret;
    }
    if (!IsValidHeader(header)) {
        return DOC_STORE_ERROR_INVALID;
    }

    // set event tag if have
    if (tag_.empty() && strlen(header.tag) != 0) {
        tag_ = header.tag;
    }

    // set page size
    pageSize_ = header.pageSize * NUM_OF_BYTES_IN_KB;

    // read the events
    if (pageSize_ == 0) {
        uint8_t* content = nullptr;
        uint32_t contentSize = 0;
        if (auto ret = ReadContent(&content, contentSize); ret != DOC_STORE_SUCCESS) {
            return ret;
        }
        TryToAddEntry(content, contentSize, query, entries, num);
        return DOC_STORE_SUCCESS;
    }
    return ReadPages(query, entries, num);
}

int SysEventDocReader::ReadHeader(DocHeader& header)
{
    if (!in_.is_open()) {
        HIVIEW_LOGE("failed to open the file, file=%{public}s", docPath_.c_str());
        return DOC_STORE_ERROR_IO;
    }
    in_.seekg(0, std::ios::beg);
    in_.read(reinterpret_cast<char*>(&header), HEADER_SIZE);
    return DOC_STORE_SUCCESS;
}

int SysEventDocReader::ReadPages(const DocQuery& query, EntryQueue& entries, int& num)
{
    uint32_t pageIndex = 0;
    while (!HasReadFileEnd()) {
        uint8_t* content = nullptr;
        uint32_t contentSize = 0;
        if (ReadContent(&content, contentSize) != DOC_STORE_SUCCESS) {
            pageIndex++;
            if (SeekgPage(pageIndex) != DOC_STORE_SUCCESS) {
                HIVIEW_LOGD("end to seekg the next page index=%{public}" PRIu32 ", file=%{public}s",
                    pageIndex, docPath_.c_str());
                break;
            }
            HIVIEW_LOGD("read the next page index=%{public}" PRIu32 ", file=%{public}s", pageIndex, docPath_.c_str());
            continue;
        }
        TryToAddEntry(content, contentSize, query, entries, num);
    }
    return DOC_STORE_SUCCESS;
}

bool SysEventDocReader::HasReadFileEnd()
{
    if (!in_.is_open()) {
        return true;
    }
    return (static_cast<int>(in_.tellg()) < 0) || in_.eof();
}

bool SysEventDocReader::HasReadPageEnd()
{
    if (HasReadFileEnd()) {
        return true;
    }
    uint32_t curPos = static_cast<uint32_t>(in_.tellg());
    if (curPos <= HEADER_SIZE) {
        return false;
    }
    return ((curPos - HEADER_SIZE) % pageSize_ + BLOCK_SIZE) >= pageSize_;
}

int SysEventDocReader::ReadContent(uint8_t** content, uint32_t& contentSize)
{
    if (HasReadPageEnd()) {
        HIVIEW_LOGD("end to read the page, file=%{public}s", docPath_.c_str());
        return DOC_STORE_READ_EMPTY;
    }
    ReadValueAndReset(in_, contentSize);
    if (contentSize <= BLOCK_SIZE) {
        HIVIEW_LOGD("invalid content size=%{public}u, file=%{public}s", contentSize, docPath_.c_str());
        return DOC_STORE_READ_EMPTY;
    }
    if (contentSize > MAX_NEW_SIZE) {
        HIVIEW_LOGE("invalid content size=%{public}u", contentSize);
        return DOC_STORE_ERROR_MEMORY;
    }
    *content = new(std::nothrow) uint8_t[contentSize];
    if (*content == nullptr) {
        HIVIEW_LOGE("failed to new memory for content, size=%{public}u", contentSize);
        return DOC_STORE_ERROR_MEMORY;
    }
    in_.read(reinterpret_cast<char*>(*content), contentSize);
    if (!IsValidContent(*content, contentSize)) {
        HIVIEW_LOGE("failed to read the content, file=%{public}s", docPath_.c_str());
        delete[] *content;
        return DOC_STORE_ERROR_INVALID;
    }
    return DOC_STORE_SUCCESS;
}

int SysEventDocReader::ReadFileSize()
{
    return fileSize_;
}

int SysEventDocReader::ReadPageSize(uint32_t& pageSize)
{
    DocHeader header;
    if (int ret = ReadHeader(header); ret != DOC_STORE_SUCCESS) {
        return ret;
    }
    pageSize = header.pageSize * NUM_OF_BYTES_IN_KB;
    return DOC_STORE_SUCCESS;
}

bool SysEventDocReader::IsValidHeader(const DocHeader& header)
{
    if (header.magicNum != MAGIC_NUM) {
        HIVIEW_LOGE("invalid magic number of file=%{public}s", docPath_.c_str());
        return false;
    }
    uint32_t crc = CrcGenerator::GetCrc32(reinterpret_cast<uint8_t*>(const_cast<DocHeader*>(&header)),
        HEADER_SIZE - CRC_SIZE);
    if (header.crc != crc) {
        HIVIEW_LOGE("invalid crc of header, file=%{public}s", docPath_.c_str());
        return false;
    }
    return true;
}

bool SysEventDocReader::IsValidContent(uint8_t* content, uint32_t contentSize)
{
    uint32_t crc = CrcGenerator::GetCrc32(content, contentSize - CRC_SIZE);
    in_.seekg(GetNegativeNum(CRC_SIZE), std::ios::cur);
    uint32_t contentCrc = 0;
    ReadValue(in_, contentCrc);
    if (contentCrc != crc) {
        HIVIEW_LOGE("invalid crc of content, contentCrc=%{public}u, getCrc=%{public}u, file=%{public}s",
            contentCrc, crc, docPath_.c_str());
        return false;
    }
    return true;
}

int SysEventDocReader::SeekgPage(uint32_t pageIndex)
{
    if (HasReadFileEnd()) {
        return DOC_STORE_ERROR_IO;
    }
    auto seekSize = HEADER_SIZE + pageSize_ * pageIndex;
    if (static_cast<int>(seekSize) < ReadFileSize()) {
        in_.seekg(seekSize, std::ios::beg);
        return DOC_STORE_SUCCESS;
    }
    in_.setstate(std::ios::eofbit);
    return DOC_STORE_ERROR_IO;
}

int SysEventDocReader::BuildRawEvent(uint8_t** rawEvent, uint32_t& eventSize, uint8_t* content, uint32_t contentSize)
{
    if (domain_.empty() || name_.empty()) {
        HIVIEW_LOGE("domain=%{public}s or name=%{public}s is empty", domain_.c_str(), name_.c_str());
        return DOC_STORE_ERROR_INVALID;
    }

    eventSize = contentSize - SEQ_SIZE + MAX_DOMAIN_LEN + MAX_EVENT_NAME_LEN;
    if (eventSize > MAX_NEW_SIZE) {
        HIVIEW_LOGE("invalid new event size=%{public}u", eventSize);
        return DOC_STORE_ERROR_MEMORY;
    }
    uint8_t* event = new(std::nothrow) uint8_t[eventSize];
    if (event == nullptr) {
        HIVIEW_LOGE("failed to new memory for raw event, size=%{public}u", eventSize);
        return DOC_STORE_ERROR_MEMORY;
    }
    if (memcpy_s(event, eventSize, reinterpret_cast<char*>(&eventSize), BLOCK_SIZE) != EOK) {
        HIVIEW_LOGE("failed to copy block size to raw event");
        delete[] event;
        return DOC_STORE_ERROR_MEMORY;
    }
    uint32_t eventPos = BLOCK_SIZE;
    if (memcpy_s(event + eventPos, eventSize - eventPos, domain_.c_str(), MAX_DOMAIN_LEN) != EOK) {
        HIVIEW_LOGE("failed to copy domain to raw event");
        delete[] event;
        return DOC_STORE_ERROR_MEMORY;
    }
    eventPos += MAX_DOMAIN_LEN;
    if (memcpy_s(event + eventPos, eventSize - eventPos, name_.c_str(), MAX_EVENT_NAME_LEN) != EOK) {
        HIVIEW_LOGE("failed to copy name to raw event");
        delete[] event;
        return DOC_STORE_ERROR_MEMORY;
    }
    eventPos += MAX_EVENT_NAME_LEN;
    if (memcpy_s(event + eventPos, eventSize - eventPos, content + BLOCK_SIZE + SEQ_SIZE,
        contentSize - BLOCK_SIZE - SEQ_SIZE) != EOK) {
        HIVIEW_LOGE("failed to copy name to raw event");
        delete[] event;
        return DOC_STORE_ERROR_MEMORY;
    }

    *rawEvent = event;
    return DOC_STORE_SUCCESS;
}

int SysEventDocReader::BuildEventJson(std::string& eventJson, uint32_t eventSize, int64_t seq)
{
    if (eventJson.empty()) {
        HIVIEW_LOGE("event json is empty");
        return DOC_STORE_ERROR_INVALID;
    }
    if (level_.empty()) {
        HIVIEW_LOGE("event level is empty");
        return DOC_STORE_ERROR_INVALID;
    }
    if (seq < 0) {
        HIVIEW_LOGE("event seq is invalid, seq=%{public}" PRId64, seq);
        return DOC_STORE_ERROR_INVALID;
    }
    if (!tag_.empty()) {
        AppendJsonValue(eventJson, EventCol::TAG, tag_);
    }
    AppendJsonValue(eventJson, EventCol::LEVEL, level_);
    AppendJsonValue(eventJson, EventCol::SEQ, seq);
    return DOC_STORE_SUCCESS;
}

void SysEventDocReader::TryToAddEntry(uint8_t* content, uint32_t contentSize, const DocQuery& query,
    EntryQueue& entries, int& num)
{
    // check inner condition
    if (!query.IsContainInnerConds(content)) {
        delete[] content;
        return;
    }
    int64_t seq = *(reinterpret_cast<int64_t*>(content + BLOCK_SIZE));
    int64_t ts = *(reinterpret_cast<int64_t*>(content + BLOCK_SIZE + SEQ_SIZE));

    uint8_t* rawEvent = nullptr;
    uint32_t eventSize = 0;
    auto ret = BuildRawEvent(&rawEvent, eventSize, content, contentSize);
    delete[] content;
    if (ret != DOC_STORE_SUCCESS) {
        return;
    }

    // check extra condition
    EventRaw::DecodedEvent decodedEvent(rawEvent);
    delete[] rawEvent;
    if (!query.IsContainExtraConds(decodedEvent)) {
        return;
    }

    // build the json string of the event
    std::string eventJson = decodedEvent.AsJsonStr();
    if (BuildEventJson(eventJson, eventSize, seq) != DOC_STORE_SUCCESS) {
        return;
    }

    num++;
    Entry entry;
    entry.id = seq;
    entry.ts = ts;
    entry.value = eventJson;
    entries.emplace(entry);
}
} // EventStore
} // HiviewDFX
} // OHOS
