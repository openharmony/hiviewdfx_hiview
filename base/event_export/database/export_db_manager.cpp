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

#include "export_db_manager.h"

#include <algorithm>
#include <limits>

#include "hiview_logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("EventExport");
ExportDbManager::ExportDbManager(const std::string& dbStoreDir)
{
    HIVIEW_LOGD("db store directory is %{public}s.", dbStoreDir.c_str());
    storage_ = std::make_shared<ExportDbStorage>(dbStoreDir);
}

int64_t ExportDbManager::GetExportBeginningSeq(const std::string& moduleName)
{
    HIVIEW_LOGD("get beginning sequence of event for module %{public}s to export", moduleName.c_str());
    ExportDetailRecord record;
    storage_->QueryExportDetailRecord(moduleName, record);
    if (record.moduleName.empty()) {
        HIVIEW_LOGW("no export details record found of %{public}s module in db", moduleName.c_str());
        return INVALID_SEQ_VAL;
    }
    if (record.exportEnabledSeq == INVALID_SEQ_VAL) {
        HIVIEW_LOGI("export switch of %{public}s is off, no need to export event", moduleName.c_str());
        return INVALID_SEQ_VAL;
    }
    return std::max(record.exportEnabledSeq, record.exportedMaxSeq + 1); // next sequence
}

void ExportDbManager::HandleExportModuleInit(const std::string& moduleName, int64_t curSeq)
{
    HIVIEW_LOGD("try to initilize export %{public}s module, current event sequence is %{public}" PRId64 "",
        moduleName.c_str(), curSeq);
    if (IsUnrecordedModule(moduleName)) {
        HIVIEW_LOGW("no export details record found of %{public}s module in db", moduleName.c_str());
        ExportDetailRecord record {
            .moduleName = moduleName,
            .exportEnabledSeq = curSeq,
            .exportedMaxSeq = INVALID_SEQ_VAL,
        };
        storage_->InsertExportDetailRecord(record);
    }
}

void ExportDbManager::HandleExportSwitchChanged(const std::string& moduleName, int64_t curSeq)
{
    HIVIEW_LOGD("export switch for %{public}s module is changed, current event sequence is %{public}" PRId64 "",
        moduleName.c_str(), curSeq);
    if (IsUnrecordedModule(moduleName)) {
        HIVIEW_LOGW("no export details record found of %{public}s module in db", moduleName.c_str());
        ExportDetailRecord record = {
            .moduleName = moduleName,
            .exportEnabledSeq = curSeq,
            .exportedMaxSeq = INVALID_SEQ_VAL,
        };
        storage_->InsertExportDetailRecord(record);
        return;
    }
    ExportDetailRecord record {
        .moduleName = moduleName,
        .exportEnabledSeq = curSeq,
    };
    storage_->UpdateExportEnabledSeq(record);
}

void ExportDbManager::HandleExportTaskFinished(const std::string& moduleName, int64_t eventSeq)
{
    HIVIEW_LOGD("export task of %{public}s module is finished, maximum event sequence is %{public}" PRId64 "",
        moduleName.c_str(), eventSeq);
    if (IsUnrecordedModule(moduleName)) {
        HIVIEW_LOGW("no export details record found of %{public}s module in db", moduleName.c_str());
        ExportDetailRecord record = {
            .moduleName = moduleName,
            .exportEnabledSeq = INVALID_SEQ_VAL,
            .exportedMaxSeq = eventSeq,
        };
        storage_->InsertExportDetailRecord(record);
        return;
    }
    ExportDetailRecord record {
        .moduleName = moduleName,
        .exportedMaxSeq = eventSeq,
    };
    storage_->UpdateExportedMaxSeq(record);
}

bool ExportDbManager::IsUnrecordedModule(const std::string& moduleName)
{
    ExportDetailRecord record;
    storage_->QueryExportDetailRecord(moduleName, record);
    return record.moduleName.empty();
}
} // HiviewDFX
} // OHOS