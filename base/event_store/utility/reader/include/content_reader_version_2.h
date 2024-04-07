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
 
#ifndef CONTENT_READER_VERSION_2_H
#define CONTENT_READER_VERSION_2_H

#include <string>

#include "content_reader.h"

namespace OHOS {
namespace HiviewDFX {
#define MAGIC_NUM_VERSION2 0x894556454E541a0a
class ContentReaderVersion2 : public ContentReader {
public:
    int ReadDocDetails(std::ifstream& docStream, EventStore::DocHeader& header, uint64_t& docHeaderSize,
        std::string& sysVersion) override;
    bool IsValidMagicNum(const uint64_t magicNum) override;

protected:
    virtual int GetContentHeader(uint8_t* content, EventStore::ContentHeader& header) override;
    virtual size_t GetContentHeaderSize() override;
};
} // HiviewDFX
} // OHOS
#endif // CONTENT_READER_VERSION_2_H