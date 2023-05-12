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

#ifndef HIVIEW_BASE_EVENT_STORE_SYS_EVENT_DOC_H
#define HIVIEW_BASE_EVENT_STORE_SYS_EVENT_DOC_H

#include <memory>
#include <string>

#include "base_def.h"
#include "event_doc_reader.h"
#include "event_doc_writer.h"
#include "sys_event.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
class SysEventDoc {
public:
    SysEventDoc(const std::string& domain, const std::string& name);
    SysEventDoc(const std::string& file);
    ~SysEventDoc();

    int Insert(const std::shared_ptr<SysEvent>& sysEvent);
    int Query(const DocQuery& query, std::vector<Entry>& entries, int& limit);
    int Delete();

private:
    int InitWriter(const std::shared_ptr<SysEvent>& sysEvent);
    int InitReader();
    std::string CreateFile();
    std::string GetDir();
    std::string GetCurFile(const std::string& dir);
    uint32_t GetMaxFileSize();
    bool IsFileFull(const std::string& file);
    bool IsNeedUpdateCurFile();
    int UpdateCurFile(const std::shared_ptr<SysEvent>& sysEvent);
    int CreateCurFile(const std::string& dir, const std::shared_ptr<SysEvent>& sysEvent);

private:
    std::shared_ptr<EventDocWriter> writer_;
    std::shared_ptr<EventDocReader> reader_;
    std::string domain_;
    std::string name_;
    int type_;
    std::string level_;
    std::string curFile_;
}; // SysEventDoc
} // EventStore
} // HiviewDFX
} // OHOS
#endif // HIVIEW_BASE_EVENT_STORE_SYS_EVENT_DOC_H