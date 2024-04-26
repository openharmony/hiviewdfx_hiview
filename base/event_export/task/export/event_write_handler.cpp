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

#include "event_write_handler.h"

#include "file_util.h"
#include "hiview_logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-EventWriteHandler");
bool EventWriteHandler::HandleRequest(RequestPtr req)
{
    auto writeReq = BaseRequest::DownCastTo<EventWriteRequest>(req);
    for (const auto& sysEvent : writeReq->sysEvents) {
        auto writer = GetEventWriter(sysEvent.version, writeReq);
        writer->AppendEvent(sysEvent.domain, sysEvent.seq, sysEvent.name, sysEvent.eventStr);
    }
    if (!writeReq->isQueryCompleted) {
        return true;
    }
    for (const auto& writer : allJsonFileWriters_) {
        if (writer.second == nullptr) {
            continue;
        }
        writer.second->Write(true);
    }
    return true;
}

void EventWriteHandler::SetExportDoneListener(ExportDoneListener listener)
{
    exportDoneListener_ = listener;
}

std::shared_ptr<ExportJsonFileWriter> EventWriteHandler::GetEventWriter(const std::string& sysVersion,
    std::shared_ptr<EventWriteRequest> writeReq)
{
    auto writerKey = std::make_pair(writeReq->moduleName, sysVersion);
    auto iter = allJsonFileWriters_.find(writerKey);
    if (iter == allJsonFileWriters_.end()) {
        HIVIEW_LOGI("create json file writer with version %{public}s", sysVersion.c_str());
        auto jsonFileWriter = std::make_shared<ExportJsonFileWriter>(writeReq->moduleName, sysVersion,
            writeReq->exportDir, writeReq->maxSingleFileSize);
        auto moduleName = writeReq->moduleName;
        jsonFileWriter->SetMaxSequenceWriteListener([this, moduleName] (int64_t maxEventSeq) {
            if (this->exportDoneListener_ != nullptr) {
                this->exportDoneListener_(moduleName, maxEventSeq);
            }
        });
        allJsonFileWriters_.emplace(writerKey, jsonFileWriter);
        return jsonFileWriter;
    }
    return iter->second;
}
} // HiviewDFX
} // OHOS