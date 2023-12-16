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

#include "event_entry_creator.h"
#include "logger.h"
#include "string_util.h"

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

uint8_t ParseTypeFromString(const std::string& typeStr)
{
    uint32_t typeNum = std::stoul(typeStr);
    return (typeNum <= 4 && typeNum >= 1) ? (typeNum - 1) : 0; // 4: max value, 1: min value
}
}

SysEventDocReader::SysEventDocReader(const std::string& path): EventDocReader(path),
    fileSize_(INVALID_VALUE_INT), pageSize_(0)
{
    Init();
}

SysEventDocReader::~SysEventDocReader()
{
    if (in_.is_open()) {
        in_.close();
    }
}

void SysEventDocReader::Init()
{
    in_.open(docPath_, std::ios::binary);
    InitCommonEventInfo();
    InitFileSize();
}

void SysEventDocReader::InitCommonEventInfo()
{
    // path example: /data/log/hiview/sys_event_db/{domain}/{name}-{type}-{level}-{seq}.db
    std::vector<std::string> dirNames;
    StringUtil::SplitStr(docPath_, FILE_SEPARATOR, dirNames);
    constexpr size_t domainIndexFromBack = 2;
    if (dirNames.size() >= domainIndexFromBack) {
        comEventInfo_.domain = dirNames[dirNames.size() - domainIndexFromBack];
        std::string fileName = dirNames.back();
        std::vector<std::string> eventFields;
        StringUtil::SplitStr(fileName, FILE_NAME_SEPARATOR, eventFields);
        if (eventFields.size() == FILE_NAME_SPLIT_SIZE) {
            comEventInfo_.name = eventFields[EVENT_NAME_INDEX];
            comEventInfo_.type = ParseTypeFromString(eventFields[EVENT_TYPE_INDEX]);
            comEventInfo_.level = eventFields[EVENT_LEVEL_INDEX];
        }
    }
}

void SysEventDocReader::InitFileSize()
{
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
    if (comEventInfo_.tag.empty() && strlen(header.tag) != 0) {
        comEventInfo_.tag = header.tag;
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
        delete[] content;
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
                HIVIEW_LOGI("end to seekg the next page index=%{public}zu, file=%{public}s",
                    pageIndex, docPath_.c_str());
                break;
            }
            HIVIEW_LOGD("read the next page index=%{public}zu, file=%{public}s", pageIndex, docPath_.c_str());
            continue;
        }
        TryToAddEntry(content, contentSize, query, entries, num);
        delete[] content;
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
    if (contentSize < (BLOCK_SIZE + DOC_EVENT_HEADER_SIZE)) {
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

void SysEventDocReader::TryToAddEntry(uint8_t* content, uint32_t contentSize, const DocQuery& query,
    EntryQueue& entries, int& num)
{
    EventEntryCreator entryCreator(comEventInfo_, query);
    auto entry = entryCreator.CreateEntry(content, contentSize);
    if (entry.has_value()) {
        entries.emplace(entry.value());
        num++;
    }
}
} // EventStore
} // HiviewDFX
} // OHOS
