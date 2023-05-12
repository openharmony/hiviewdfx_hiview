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
#include "sys_event_doc.h"

#include <errno.h>

#include "base_def.h"
#include "event_store_config.h"
#include "file_util.h"
#include "logger.h"
#include "sys_event_database.h"
#include "sys_event_doc_reader.h"
#include "sys_event_doc_writer.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
DEFINE_LOG_TAG("HiView-SysEventDoc");
namespace {
const char FILE_NAME_SEPARATOR[] = "-";
const char FILE_SEPARATOR[] = "/";
const char FILE_EXT[] = ".db";
}

SysEventDoc::SysEventDoc(const std::string& domain, const std::string& name)
    : writer_(nullptr), reader_(nullptr), domain_(domain), name_(name)
{}

SysEventDoc::SysEventDoc(const std::string& file) : writer_(nullptr), reader_(nullptr), curFile_(file)
{}

SysEventDoc::~SysEventDoc()
{}

int SysEventDoc::Insert(const std::shared_ptr<SysEvent>& sysEvent)
{
    if (sysEvent == nullptr) {
        HIVIEW_LOGI("event is null");
        return DOC_STORE_ERROR_NULL;
    }

    // init a writer for writing
    auto ret = DOC_STORE_SUCCESS;
    if (ret = InitWriter(sysEvent); ret != 0) {
        HIVIEW_LOGE("failed to init writer from file=%{public}s", curFile_.c_str());
        return ret;
    }

    // write event to the file
    if (ret = writer_->Write(sysEvent); ret == DOC_STORE_NEW_FILE) {
        // need to store to a new file
        if (ret = CreateCurFile(GetDir(), sysEvent); ret != DOC_STORE_SUCCESS) {
            HIVIEW_LOGE("failed to update curFile=%{public}s", curFile_.c_str());
            return ret;
        }
        writer_ = std::make_shared<SysEventDocWriter>(curFile_);
        HIVIEW_LOGD("update writer in new File=%{public}s", curFile_.c_str());
        return writer_->Write(sysEvent);
    }
    return ret;
}

int SysEventDoc::Delete()
{
    return 0;
}

int SysEventDoc::Query(const DocQuery& query, std::vector<Entry>& entries, int& limit)
{
    auto ret = DOC_STORE_SUCCESS;
    if (reader_ == nullptr) {
        if (ret = InitReader(); ret != DOC_STORE_SUCCESS) {
            return ret;
        }
    }
    return reader_->Read(query, entries, limit);
}

int SysEventDoc::InitWriter(const std::shared_ptr<SysEvent>& sysEvent)
{
    if (writer_ == nullptr || IsNeedUpdateCurFile()) {
        type_ = sysEvent->eventType_;
        level_ = sysEvent->GetLevel();
        if (auto ret = UpdateCurFile(sysEvent); ret != 0) {
            HIVIEW_LOGE("failed to update current file");
            return ret;
        }
        writer_ = std::make_shared<SysEventDocWriter>(curFile_);
        HIVIEW_LOGD("init writer in curFile=%{public}s", curFile_.c_str());
    }
    return DOC_STORE_SUCCESS;
}

int SysEventDoc::InitReader()
{
    if (curFile_.empty() || !FileUtil::FileExists(curFile_)) {
        HIVIEW_LOGE("failed to init reader from file=%{public}s", curFile_.c_str());
        return DOC_STORE_ERROR_IO;;
    }
    reader_ = std::make_shared<SysEventDocReader>(curFile_);
    return DOC_STORE_SUCCESS;
}

bool SysEventDoc::IsFileFull(const std::string& file)
{
    return FileUtil::GetFileSize(file) >= GetMaxFileSize();
}

bool SysEventDoc::IsNeedUpdateCurFile()
{
    return curFile_.empty() || IsFileFull(curFile_);
}

int SysEventDoc::UpdateCurFile(const std::shared_ptr<SysEvent>& sysEvent)
{
    std::string dir = GetDir();
    if (dir.empty()) {
        return DOC_STORE_ERROR_IO;
    }
    std::string filePath = GetCurFile(dir);
    if (filePath.empty() || IsFileFull(filePath)) {
        return CreateCurFile(dir, sysEvent);
    }
    curFile_ = filePath;
    return DOC_STORE_SUCCESS;
}

std::string SysEventDoc::GetDir()
{
    std::string dir = SysEventDatabase::GetInstance().GetDatabaseDir() + domain_;
    if (!FileUtil::IsDirectory(dir) && !FileUtil::ForceCreateDirectory(dir)) {
        HIVIEW_LOGE("failed to create domain dir=%{public}s", dir.c_str());
        return "";
    }
    return dir;
}

std::string SysEventDoc::GetCurFile(const std::string& dir)
{
    std::vector<std::string> files;
    FileUtil::GetDirFiles(dir, files);
    HIVIEW_LOGI("liangyujian get curFiles size=%{public}zu", files.size());
    std::string curFile;
    for (auto& file : files) {
        HIVIEW_LOGI("liangyujian file=%{public}s, filter=%{public}s", file.c_str(), (name_ + FILE_NAME_SEPARATOR).c_str());
        if (file.find(name_ + FILE_NAME_SEPARATOR) == std::string::npos) {
            continue;
        }
        if ((file.size() == curFile.size() && file > curFile) || file.size() > curFile.size()) {
            curFile = file;
        }
    }
    HIVIEW_LOGI("liangyujian get File=%{public}s", curFile.c_str());
    return curFile;
}

uint32_t SysEventDoc::GetMaxFileSize()
{
    return EventStoreConfig::GetInstance().GetMaxFileSize(type_) * NUM_OF_BYTES_IN_KB;
}

int SysEventDoc::CreateCurFile(const std::string& dir, const std::shared_ptr<SysEvent>& sysEvent)
{
    auto seq = sysEvent->GetEventSeq();
    std::string filePath = dir + FILE_SEPARATOR;
    filePath.append(name_).append(FILE_NAME_SEPARATOR).append(std::to_string(type_)).append(FILE_NAME_SEPARATOR)
        .append(level_).append(FILE_NAME_SEPARATOR).append(std::to_string(seq)).append(FILE_EXT);
    if (auto ret = FileUtil::CreateFile(filePath, FileUtil::FILE_PERM_660); ret != 0) {
        HIVIEW_LOGE("failed to create file=%{public}s", filePath.c_str());
        return DOC_STORE_ERROR_IO;
    }
    curFile_ = filePath;
    return DOC_STORE_SUCCESS;
}
} // EventStore
} // HiviewDFX
} // OHOS
