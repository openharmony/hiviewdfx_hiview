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

#include "sys_event_sequence_mgr.h"

#include <fstream>

#include "file_util.h"
#include "hiview_logger.h"
#include "sys_event_dao.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
namespace {
DEFINE_LOG_TAG("HiView-SysEventSeqMgr");

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

std::string GetSequenceBackupFile()
{
    return EventStore::SysEventDao::GetDatabaseDir() + SEQ_PERSISTS_BACKUP_FILE_NAME;
}

void WriteEventSeqToFile(int64_t seq, const std::string& file)
{
    std::string content(std::to_string(seq));
    if (!SaveStringToFile(file, content)) {
        HIVIEW_LOGE("failed to write sequence %{public}s to %{public}s.", content.c_str(), file.c_str());
    }
}

void ReadEventSeqFromFile(int64_t& seq, const std::string& file)
{
    std::string content;
    if (!FileUtil::LoadStringFromFile(file, content)) {
        HIVIEW_LOGE("failed to read sequence value from %{public}s.", file.c_str());
    }
    seq = static_cast<int64_t>(strtoll(content.c_str(), nullptr, 0));
}
}

SysEventSequenceManager& SysEventSequenceManager::GetInstance()
{
    static SysEventSequenceManager instance;
    return instance;
}

SysEventSequenceManager::SysEventSequenceManager()
{
    if (!FileUtil::FileExists(GetSequenceFile())) {
        EventStore::SysEventDao::Restore();
    }
    int64_t seq = 0;
    ReadSeqFromFile(seq);
    curSeq_.store(seq, std::memory_order_release);
}

void SysEventSequenceManager::SetSequence(int64_t seq)
{
    curSeq_.store(seq, std::memory_order_release);
    WriteSeqToFile(seq);
}

int64_t SysEventSequenceManager::GetSequence()
{
    return curSeq_.load(std::memory_order_acquire);
}

void SysEventSequenceManager::WriteSeqToFile(int64_t seq)
{
    WriteEventSeqToFile(seq, GetSequenceFile());
    WriteEventSeqToFile(seq, GetSequenceBackupFile());
}

void SysEventSequenceManager::ReadSeqFromFile(int64_t& seq)
{
    ReadEventSeqFromFile(seq, GetSequenceFile());
    int64_t seqBackup = 0;
    ReadEventSeqFromFile(seqBackup, GetSequenceBackupFile());
    if (seq == seqBackup) {
        HIVIEW_LOGI("succeed to read event sequence, value is %{public}" PRId64 ".", seq);
        return;
    }
    HIVIEW_LOGW("seq[%{public}" PRId64 "] is different with backup seq[%{public}" PRId64 "].", seq, seqBackup);
    if (seq > seqBackup) {
        WriteEventSeqToFile(seq, GetSequenceBackupFile());
    } else {
        seq = seqBackup;
        WriteEventSeqToFile(seq, GetSequenceFile());
    }
}

std::string SysEventSequenceManager::GetSequenceFile() const
{
    return EventStore::SysEventDao::GetDatabaseDir() + SEQ_PERSISTS_FILE_NAME;
}
} // EventStore
} // HiviewDFX
} // OHOS