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

#ifndef HIVIEW_BASE_EVENT_STORE_UTILITY_SYS_EVENT_DOC_READER_H
#define HIVIEW_BASE_EVENT_STORE_UTILITY_SYS_EVENT_DOC_READER_H

#include <fstream>
#include <string>

#include "event_doc_reader.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
class SysEventDocReader : public EventDocReader {
public:
    SysEventDocReader(const std::string& path);
    ~SysEventDocReader();
    int Read(const DocQuery& query, EntryQueue& entries, int& num) override;
    int ReadFileSize();
    int ReadPageSize(uint32_t& pageSize);

private:
    void Init(const std::string& path);
    int ReadHeader(DocHeader& header);
    int ReadContent(uint8_t** content, uint32_t& contentSize);
    int ReadPages(const DocQuery& query, EntryQueue& entries, int& num);
    bool HasReadFileEnd();
    bool HasReadPageEnd();
    bool IsValidHeader(const DocHeader& header);
    bool IsValidContent(uint8_t* content, uint32_t contentSize);
    int SeekgPage(uint32_t pageIndex);
    int BuildRawEvent(uint8_t** rawEvent, uint32_t& eventSize, uint8_t* content, uint32_t contentSize);
    int BuildEventJson(std::string& eventJson, uint32_t eventSize, int64_t seq);
    void TryToAddEntry(uint8_t* content, uint32_t contentSize, const DocQuery& query,
        EntryQueue& entries, int& num);

private:
    std::ifstream in_;
    int fileSize_;
    uint32_t pageSize_;

    std::string domain_;
    std::string name_;
    std::string level_;
    std::string tag_;
}; // EventDocWriter
} // EventStore
} // HiviewDFX
} // OHOS
#endif // HIVIEW_BASE_EVENT_STORE_UTILITY_SYS_EVENT_DOC_READER_H
