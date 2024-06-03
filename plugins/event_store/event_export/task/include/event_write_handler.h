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

#ifndef HIVIEW_BASE_EVENT_EXPORT_EXPORTED_EVENT_WRITE_HANDLER_H
#define HIVIEW_BASE_EVENT_EXPORT_EXPORTED_EVENT_WRITE_HANDLER_H

#include <functional>
#include <list>
#include <tuple>
#include <unordered_map>

#include "export_base_handler.h"
#include "export_db_storage.h"
#include "export_json_file_writer.h"

namespace OHOS {
namespace HiviewDFX {
struct CachedEventItem {
    // event version
    std::string version;

    // event domain
    std::string domain;

    // event sequence
    int64_t seq = INVALID_SEQ_VAL;

    // event name
    std::string name;

    // event json string
    std::string eventStr;
};

struct EventWriteRequest : public BaseRequest {
    // name of export module
    std::string moduleName;

    // max size of a single event file
    int64_t maxSingleFileSize = 0;

    // item: <system version, domain, sequecen, sysevent content>
    std::list<CachedEventItem> sysEvents;

    // directory configured for export event file to store
    std::string exportDir;
    
    // tag whether the query is completed
    bool isQueryCompleted;
};

class EventWriteHandler : public ExportBaseHandler {
public:
    using ExportDoneListener = std::function<void(const std::string&, int64_t)>;
    void SetExportDoneListener(ExportDoneListener listener);

public:
    bool HandleRequest(RequestPtr req) override;
    std::shared_ptr<ExportJsonFileWriter> GetEventWriter(const std::string& sysVersion,
        std::shared_ptr<EventWriteRequest> writeReq);

private:
    ExportDoneListener exportDoneListener_;
    // <key: <module name, system version>, value: writer>
    std::map<std::pair<std::string, std::string>, std::shared_ptr<ExportJsonFileWriter>> allJsonFileWriters_;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIVIEW_BASE_EVENT_EXPORT_EXPORTED_EVENT_WRITE_HANDLER_H