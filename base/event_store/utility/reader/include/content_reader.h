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

#ifndef CONTENT_READER_H
#define CONTENT_READER_H

#include <fstream>
#include <string>

#include "base_def.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventRaw {
class RawData;
}
using EventRaw::RawData;

struct EventInfo {
    char domain[MAX_DOMAIN_LEN] = {0};

    char name[MAX_EVENT_NAME_LEN] = {0};

    std::string level;

    std::string tag;

    int64_t seq = 0;

    int64_t timestamp = 0;
};
using EventInfo = struct EventInfo;

class ContentReader {
public:
    virtual ~ContentReader() {};

public:
    static uint8_t ReadFmtVersion(std::ifstream& docStream);
    std::shared_ptr<RawData> ReadRawData(const EventInfo& eventInfo, uint8_t* content, uint32_t contentSize);
    virtual int ReadDocDetails(std::ifstream& docStream, EventStore::DocHeader& header,
        uint64_t& docHeaderSize, std::string& sysVersion) = 0;
    virtual bool IsValidMagicNum(const uint64_t magicNum) = 0;

protected:
    virtual int GetContentHeader(uint8_t* content, EventStore::ContentHeader& header) = 0;
    virtual size_t GetContentHeaderSize() = 0;

protected:
    int AppendDomainAndName(std::shared_ptr<RawData> rawData, const EventInfo& eventInfo);
    int AppendContentData(std::shared_ptr<RawData> rawData, uint8_t* content, uint32_t contentSize);
    int AppendExtraInfo(std::shared_ptr<RawData> rawData, const EventInfo& eventInfo);
    int AppendTag(std::shared_ptr<RawData> rawData, const std::string& tag);
    int AppendLevel(std::shared_ptr<RawData> rawData, const std::string& level);
    int AppendSeq(std::shared_ptr<RawData> rawData, int64_t seq);
};
} // HiviewDFX
} // OHOS
#endif // CONTENT_READER_H