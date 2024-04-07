/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef HIVIEW_BASE_EVENT_STORE_UTILITY_SYS_EVENT_DOC_WRITER_H
#define HIVIEW_BASE_EVENT_STORE_UTILITY_SYS_EVENT_DOC_WRITER_H

#include <fstream>
#include <memory>
#include <string>

#include "event_doc_writer.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
class SysEventDocWriter : public EventDocWriter {
public:
    SysEventDocWriter(const std::string& path);
    ~SysEventDocWriter();
    int Write(const std::shared_ptr<SysEvent>& sysEvent);

private:
    int WriteHeader(const std::shared_ptr<SysEvent>& sysEvent, uint32_t contentSize);
    int WriteContent(const std::shared_ptr<SysEvent>& sysEvent, uint32_t contentSize);
    int BuildContent(const std::shared_ptr<SysEvent>& sysEvent, uint8_t** contentPtr, uint32_t contentSize);
    uint32_t GetCurrPageRemainSize(int fileSize, uint32_t pageSize);
    int FillCurrPageWithZero(uint32_t remainSize);
    int GetContentSize(const std::shared_ptr<SysEvent>& sysEvent, uint32_t& contentSize);

private:
    std::ofstream out_;
    uint32_t headerSize_ = 0;
}; // EventDocWriter
} // EventStore
} // HiviewDFX
} // OHOS
#endif // HIVIEW_BASE_EVENT_STORE_UTILITY_SYS_EVENT_DOC_WRITER_H
