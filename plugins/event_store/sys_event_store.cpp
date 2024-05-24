/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "sys_event_store.h"

#include <cstdio>
#include <fstream>
#include <memory>

#include "event.h"
#include "event_export_engine.h"
#include "file_util.h"
#include "hiview_global.h"
#include "hiview_logger.h"
#include "hiview_platform.h"
#include "parameter_ex.h"
#include "plugin_factory.h"
#include "sys_event.h"
#include "sys_event_dao.h"
#include "sys_event_db_mgr.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
REGISTER(SysEventStore);
DEFINE_LOG_TAG("HiView-SysEventStore");
constexpr char SEQ_PERSISTS_FILE_NAME[] = "event_sequence";
const std::string PROP_LAST_BACKUP = "persist.hiviewdfx.priv.sysevent.backup_time";

bool SaveStringToFile(const std::string& filePath, const std::string& content)
{
    std::ofstream file;
    file.open(filePath.c_str(), std::ios::in | std::ios::out);
    if (!file.is_open()) {
        file.open(filePath.c_str(), std::ios::out);
        if (!file.is_open()) {
            return false;
        }
    }
    file.seekp(0);
    file.write(content.c_str(), content.length() + 1);
    bool ret = !file.fail();
    file.close();
    return ret;
}
}

SysEventStore::SysEventStore() : hasLoaded_(false)
{
    sysEventDbMgr_ = std::make_unique<SysEventDbMgr>();
}

SysEventStore::~SysEventStore() {}

void SysEventStore::OnLoad()
{
    HIVIEW_LOGI("sys event service load");
    sysEventDbMgr_->StartCheckStoreTask(this->workLoop_);
    auto context = GetHiviewContext();
    HiviewPlatform* hiviewPlatform = static_cast<HiviewPlatform*>(context);
    if (hiviewPlatform == nullptr) {
        HIVIEW_LOGW("hiviewPlatform is null");
        return;
    }
    sysEventParser_ = hiviewPlatform->GetEventJsonParser();
    if (sysEventParser_ == nullptr) {
        hasLoaded_ = false;
        return;
    }
    auto getTagFunc = std::bind(&EventJsonParser::GetTagByDomainAndName, *(sysEventParser_.get()),
        std::placeholders::_1, std::placeholders::_2);
    SysEventServiceAdapter::BindGetTagFunc(getTagFunc);
    auto getTypeFunc = std::bind(&EventJsonParser::GetTypeByDomainAndName, *(sysEventParser_.get()),
        std::placeholders::_1, std::placeholders::_2);
    SysEventServiceAdapter::BindGetTypeFunc(getTypeFunc);

    if (!FileUtil::FileExists(GetSequenceFile())) {
        EventStore::SysEventDao::Restore();
    }
    ReadSeqFromFile(curSeq_);
    lastBackupTime_ = Parameter::GetString(PROP_LAST_BACKUP, "");
    EventExportEngine::GetInstance().Start();
    hasLoaded_ = true;
}

void SysEventStore::OnUnload()
{
    HIVIEW_LOGI("sys event service unload");
    EventExportEngine::GetInstance().Stop();
}

std::string SysEventStore::GetSequenceFile() const
{
    return EventStore::SysEventDao::GetDatabaseDir() + SEQ_PERSISTS_FILE_NAME;
}

void SysEventStore::WriteSeqToFile(int64_t seq) const
{
    std::string seqFile(GetSequenceFile());
    std::string content(std::to_string(seq));
    if (!SaveStringToFile(seqFile, content)) {
        HIVIEW_LOGE("failed to write sequence %{public}s.", content.c_str());
    }
    SysEventServiceAdapter::UpdateEventSeq(seq);
}

void SysEventStore::ReadSeqFromFile(int64_t& seq)
{
    std::string content;
    std::string seqFile = GetSequenceFile();
    if (!FileUtil::LoadStringFromFile(seqFile, content)) {
        HIVIEW_LOGE("failed to read sequence value from %{public}s.", seqFile.c_str());
        return;
    }
    seq = static_cast<int64_t>(strtoll(content.c_str(), nullptr, 0));
    HIVIEW_LOGI("read sequence successful, value is %{public}" PRId64 ".", seq);
    SysEventServiceAdapter::UpdateEventSeq(seq);
}

bool SysEventStore::IsNeedBackup(const std::string& dateStr)
{
    if (lastBackupTime_ == dateStr) {
        return false;
    }
    if (lastBackupTime_.empty()) {
        // first time boot, no need to backup
        lastBackupTime_ = dateStr;
        Parameter::SetProperty(PROP_LAST_BACKUP, dateStr);
        HIVIEW_LOGI("first time boot, record backup time: %{public}s.", dateStr.c_str());
        return false;
    }
    return true;
}

std::shared_ptr<SysEvent> SysEventStore::Convert2SysEvent(std::shared_ptr<Event>& event)
{
    if (event == nullptr) {
        HIVIEW_LOGE("event is null");
        return nullptr;
    }
    if (event->messageType_ != Event::MessageType::SYS_EVENT) {
        HIVIEW_LOGE("receive out of sys event type");
        return nullptr;
    }
    std::shared_ptr<SysEvent> sysEvent = Event::DownCastTo<SysEvent>(event);
    if (sysEvent == nullptr) {
        HIVIEW_LOGE("sysevent is null");
    }
    return sysEvent;
}

bool SysEventStore::OnEvent(std::shared_ptr<Event>& event)
{
    if (!hasLoaded_) {
        HIVIEW_LOGE("SysEventService not ready");
        return false;
    }

    std::shared_ptr<SysEvent> sysEvent = Convert2SysEvent(event);
    if (sysEvent != nullptr && sysEvent->preserve_) {
        // add seq to sys event and save it to local file
        sysEvent->SetEventSeq(curSeq_);
        WriteSeqToFile(++curSeq_);
        sysEventDbMgr_->SaveToStore(sysEvent);

        std::string dateStr(TimeUtil::TimestampFormatToDate(TimeUtil::GetSeconds(), "%Y%m%d"));
        if (IsNeedBackup(dateStr)) {
            EventStore::SysEventDao::Backup();
            lastBackupTime_ = dateStr;
            Parameter::SetProperty(PROP_LAST_BACKUP, dateStr);
        }
    }
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
