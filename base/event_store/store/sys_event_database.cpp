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
#include "sys_event_database.h"

#include "file_util.h"
#include "hiview_global.h"
#include "logger.h"
#include "string_util.h"
#include "sys_event_dao.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
DEFINE_LOG_TAG("HiView-SysEventDatabase");
namespace {
constexpr size_t DEFAULT_CAPACITY = 20;
const char FILE_DELIMIT_STR[] = "/";
}

SysEventDatabase::SysEventDatabase()
{
    lruCache_ = std::make_unique<SysEventDocLruCache>(DEFAULT_CAPACITY);
}

std::string SysEventDatabase::GetDatabaseDir()
{
    static std::string dir;
    if (!dir.empty()) {
        return dir;
    }
    auto& context = HiviewGlobal::GetInstance();
    if (context == nullptr) {
        HIVIEW_LOGE("hiview context is null");
        return dir;
    }
    std::string workPath = context->GetHiViewDirectory(HiviewContext::DirectoryType::WORK_DIRECTORY);
    dir = FileUtil::IncludeTrailingPathDelimiter(workPath) + "sys_event_db" + FILE_DELIMIT_STR;
    if (!FileUtil::FileExists(dir)) {
        if (FileUtil::ForceCreateDirectory(dir, FileUtil::FILE_PERM_770)) {
            HIVIEW_LOGI("succeeded in creating sys_event_db path=%{public}s", dir.c_str());
        } else {
            dir = workPath;
            HIVIEW_LOGW("failed to create sys_event_db path, use default=%{public}s", dir.c_str());
        }
    }
    return dir;
}

int SysEventDatabase::Insert(const std::shared_ptr<SysEvent>& event)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    std::shared_ptr<SysEventDoc> sysEventDoc = nullptr;
    auto keyOfCache = std::pair<std::string, std::string>(event->domain_, event->eventName_);
    if (lruCache_->Contain(keyOfCache)) {
        sysEventDoc = lruCache_->Get(keyOfCache);
        HIVIEW_LOGI("liangyujian get sysEventDoc from cache");
    } else {
        sysEventDoc = std::make_shared<SysEventDoc>(event->domain_, event->eventName_);
        lruCache_->Add(keyOfCache, sysEventDoc);
        HIVIEW_LOGI("liangyujian get sysEventDoc from create");
    }
    return sysEventDoc->Insert(event);
}

int SysEventDatabase::Delete(const std::string& domain, const std::string& name)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto keyOfCache = std::pair<std::string, std::string>(domain, name);
    if (!lruCache_->Contain(keyOfCache)) {
        return DOC_STORE_SUCCESS;
    }
    auto sysEventDoc = lruCache_->Get(keyOfCache);
    if (!lruCache_->Remove(keyOfCache)) {
        HIVIEW_LOGE("failed to remove doc from cache, domain=%{public}s, name=%{public}s",
             domain.c_str(), name.c_str());
        return DOC_STORE_ERROR_INVALID;
    }
    return sysEventDoc->Delete();
}

int SysEventDatabase::Query(const SysEventQuery& sysEventQuery, std::vector<Entry>& entries)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::vector<std::string> queryFiles;
    const auto& queryArg = sysEventQuery.queryArg_;
    GetQueryFiles(queryArg, queryFiles);
    HIVIEW_LOGD("get the file list, size=%{public}zu", queryFiles.size());

    if (queryFiles.empty()) {
        return DOC_STORE_SUCCESS;
    }

    return QueryByFiles(sysEventQuery, entries, queryFiles);
}

void SysEventDatabase::GetQueryFiles(const SysEventQueryArg& queryArg, std::vector<std::string>& queryFiles)
{
    std::vector<std::string> queryDirs;
    GetQueryDirsByDomain(queryArg.domain, queryDirs);
    HIVIEW_LOGD("liangyujian get the dir size=%{public}zu", queryDirs.size());
    if (queryDirs.empty()) {
        return;
    }

    for (auto& queryDir : queryDirs) {
        std::vector<std::string> files;
        FileUtil::GetDirFiles(queryDir, files);
        for (auto& file : files) {
            HIVIEW_LOGD("liangyujian get the file=%{public}s", file.c_str());
            if (IsContainQueryArg(file, queryArg)) {
                queryFiles.push_back(file);
            }
        }
    }
}

void SysEventDatabase::GetQueryDirsByDomain(const std::string& domain, std::vector<std::string>& queryDirs)
{
    if (domain.empty()) {
        FileUtil::GetDirDirs(GetDatabaseDir(), queryDirs);
    } else {
        std::string domainDir = GetDatabaseDir() + domain;
        if (FileUtil::IsDirectory(domainDir)) {
            queryDirs.push_back(domainDir + FILE_DELIMIT_STR);
        }
    }
}

bool SysEventDatabase::IsContainQueryArg(const std::string file, const SysEventQueryArg& queryArg)
{
    if (queryArg.names.empty() && queryArg.type == 0 && queryArg.toSeq == INVALID_VALUE_INT) {
        return true;
    }
    std::string fileName = file.substr(file.rfind(FILE_DELIMIT_STR) + 1); // 1 for next char
    std::vector<std::string> splitStrs;
    StringUtil::SplitStr(fileName, "-", splitStrs);
    if (splitStrs.size() < FILE_NAME_SPLIT_SIZE) {
        HIVIEW_LOGE("invalid file name, file=%{public}s", fileName.c_str());
        return false;
    }
    if (!queryArg.names.empty() && !std::any_of(queryArg.names.begin(), queryArg.names.end(),
        [&splitStrs] (auto& item) {
            return item == splitStrs[EVENT_NAME_INDEX];
        })) {
        HIVIEW_LOGD("liangyujian failed 1");
        return false;
    }
    if (queryArg.type != 0 && StringUtil::ToString(queryArg.type) != splitStrs[EVENT_TYPE_INDEX]) {
        HIVIEW_LOGD("liangyujian failed 2");
        return false;
    }
    if (queryArg.toSeq != INVALID_VALUE_INT && queryArg.toSeq > std::stoll(splitStrs[EVENT_SEQ_INDEX])) {
        HIVIEW_LOGD("liangyujian failed 3");
        return false;
    }
    HIVIEW_LOGD("liangyujian success");
    return true;
}

int SysEventDatabase::QueryByFiles(const SysEventQuery& sysEventQuery, std::vector<Entry>& entries,
    const std::vector<std::string>& queryFiles)
{
    HIVIEW_LOGI("liangyujian query files start");
    DocQuery docQuery;
    sysEventQuery.BuildDocQuery(docQuery);
    int limit = sysEventQuery.limit_;
    for (auto& file : queryFiles) {
        if (limit <= 0) {
            break;
        }
        auto sysEventDoc = std::make_shared<SysEventDoc>(file);
        if (auto res = sysEventDoc->Query(docQuery, entries, limit); res != DOC_STORE_SUCCESS) {
            HIVIEW_LOGE("failed to query event from doc, file=%{public}s", file.c_str());
            return res;
        }
        HIVIEW_LOGI("liangyujian query file end, file=%{public}s, limit=%{public}d", file.c_str(), limit);
    }
    HIVIEW_LOGI("liangyujian query files end");
    return DOC_STORE_SUCCESS;
}
} // EventStore
} // HiviewDFX
} // OHOS
