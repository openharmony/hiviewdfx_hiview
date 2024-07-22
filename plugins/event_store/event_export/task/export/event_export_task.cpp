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

#include "event_export_task.h"

#include "file_util.h"
#include "hiview_logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-EventExportTask");
using  ExportEventListParsers = std::map<std::string, std::shared_ptr<ExportEventListParser>>;
namespace {
constexpr int64_t BYTE_TO_MB = 1024 * 1024;

std::shared_ptr<ExportEventListParser> GetParser(ExportEventListParsers& parsers,
    const std::string& path)
{
    auto iter = parsers.find(path);
    if (iter == parsers.end()) {
        parsers.emplace(path, std::make_shared<ExportEventListParser>(path));
        return parsers[path];
    }
    return iter->second;
}
}

void EventExportTask::OnTaskRun()
{
    if (config_ == nullptr || dbMgr_ == nullptr) {
        HIVIEW_LOGE("config manager or db manager is invalid");
        return;
    }
    if (FileUtil::GetFolderSize(config_->exportDir) >= static_cast<uint64_t>(config_->maxCapcity * BYTE_TO_MB)) {
        HIVIEW_LOGE("event export directory is full");
        return;
    }

    // init handler request
    auto readReq = std::make_shared<EventReadRequest>();
    readReq->moduleName = config_->moduleName;
    readReq->beginSeq = dbMgr_->GetExportBeginningSeq(config_->moduleName);
    readReq->maxSize = config_->maxSize;
    readReq->exportDir = config_->exportDir;
    if (!ParseExportEventList(readReq->eventList)) {
        HIVIEW_LOGE("failed to parse event list to export");
        return;
    }

    // init write handler
    auto writeHandler = std::make_shared<EventWriteHandler>();
    writeHandler->SetExportDoneListener([this] (const std::string& moduleName, int64_t seq) {
        HIVIEW_LOGI("update maximum seqeuence of exported event: %{public}" PRId64 " for module %{public}s",
            seq, moduleName.c_str());
            this->dbMgr_->HandleExportTaskFinished(moduleName, seq);
    });
    // init read handler
    auto readHandler = std::make_shared<EventReadHandler>();
    // init handler chain
    readHandler->SetNextHandler(writeHandler);
    // start handler chain, read event from origin db file
    if (!readHandler->HandleRequest(readReq)) {
        HIVIEW_LOGE("some error occured during event exporting");
        return;
    }
    HIVIEW_LOGI("task of exporting events finished");
}

bool EventExportTask::ParseExportEventList(ExportEventList& list)
{
    ExportEventListParsers parsers;
    auto iter = std::max_element(config_->eventsConfigFiles.begin(), config_->eventsConfigFiles.end(),
        [&parsers] (const std::string& path1, const std::string& path2) {
            auto p1 = GetParser(parsers, path1);
            auto p2 = GetParser(parsers, path2);
            return p1->GetConfigurationVersion() < p2->GetConfigurationVersion();
        });
    if (iter == config_->eventsConfigFiles.end()) {
        HIVIEW_LOGE("no event list file path is configured.");
        return false;
    }
    HIVIEW_LOGD("event list file path is %{public}s", (*iter).c_str());
    auto parser = GetParser(parsers, *iter);
    parser->GetExportEventList(list);
    return true;
}
} // HiviewDFX
} // OHOS