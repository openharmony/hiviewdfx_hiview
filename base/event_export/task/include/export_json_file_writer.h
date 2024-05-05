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

#ifndef HIVIEW_BASE_EVENT_EXPORT_JSON_WRITER_H
#define HIVIEW_BASE_EVENT_EXPORT_JSON_WRITER_H

#include <functional>
#include <map>
#include <string>
#include <unordered_map>

#include "cJSON.h"
#include "export_db_storage.h"

namespace OHOS {
namespace HiviewDFX {
class ExportJsonFileWriter {
// <domain, <seq, <name, event string>>
using EventsDividedInDomainGroupType =
    std::unordered_map<std::string, std::unordered_map<int64_t, std::pair<std::string, std::string>>>;
public:
    ExportJsonFileWriter(const std::string& moduleName, const std::string& eventVersion, const std::string& exportDir,
        int64_t maxFileSize);

public:
    using MaxSequenceWriteListener = std::function<void(int64_t)>;
    void SetMaxSequenceWriteListener(MaxSequenceWriteListener listener);

public:
    bool AppendEvent(const std::string& domain, int64_t seq, const std::string& name, const std::string& eventStr);
    bool Write(bool isLastPartialQuery = false);

private:
    bool PackJsonStrToFile(EventsDividedInDomainGroupType& cachedToPackEvents);

private:
    std::string moduleName_;
    std::string eventVersion_;
    std::string exportDir_;
    int64_t maxFileSize_ = 0;
    int64_t totalJsonStrSize_ = 0;
    int64_t maxEventSeq_ = INVALID_SEQ_VAL;
    MaxSequenceWriteListener maxSequenceWriteListener_;
    EventsDividedInDomainGroupType eventInDomains_;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIVIEW_BASE_EVENT_EXPORT_JSON_WRITER_H