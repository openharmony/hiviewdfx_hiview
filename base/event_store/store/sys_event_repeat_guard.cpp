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

#include "sys_event_repeat_guard.h"

#include <algorithm>

#include "file_util.h"
#include "logger.h"
#include "parameter_ex.h"
#include "string_util.h"
#include "sys_event_database.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-SysEvent-Repeat-Guard");
namespace {
constexpr uint64_t TIME_RANGE_COMMERCIAL = 24 * 60 * 60; // 24h
constexpr uint64_t TIME_RANGE_BETA = 1 * 60 * 60; // 1h
}

bool SysEventRepeatGuard::IsEventTypeMatched(std::shared_ptr<SysEvent> event, const std::string& file)
{
    std::string fileName = file.substr(file.rfind("/") + 1);
    std::vector<std::string> splitStrs;
    StringUtil::SplitStr(fileName, "-", splitStrs);
    if (splitStrs.size() < FILE_NAME_SPLIT_SIZE) {
        HIVIEW_LOGE("invalid file name, file = %{public}s", fileName.c_str());
        return false;
    }
    std::string eventName = splitStrs[EVENT_NAME_INDEX];
    std::string eventType = splitStrs[EVENT_TYPE_INDEX];
    if (eventName == event->eventName_ && eventType == std::to_string(event->GetEventType())) {
        return true;
    }
    return false;
}

uint64_t SysEventRepeatGuard::GetMinValidTime()
{
    uint64_t timeRange = TIME_RANGE_COMMERCIAL;
    if (Parameter::IsBetaVersion()) {
        timeRange = TIME_RANGE_BETA;
    }
    uint64_t timeNow = time(nullptr);
    uint64_t timeMin = timeNow > timeRange ? (timeNow - timeRange) : 0;
    return timeMin;
}

bool SysEventRepeatGuard::IsTimeRangeMatched(const std::string& file)
{
    struct stat fileInfo;
    stat(file.c_str(), &fileInfo);
    uint64_t modiTime = fileInfo.st_mtime;
    return modiTime >= GetMinValidTime();
}

bool SysEventRepeatGuard::IsFileMatched(std::shared_ptr<SysEvent> event, const std::string& file)
{
    return IsEventTypeMatched(event, file) && IsTimeRangeMatched(file);
}

bool SysEventRepeatGuard::GetMatchedFileList(std::shared_ptr<SysEvent> event, std::vector<std::string>& fileList)
{
    std::string domainDir = EventStore::SysEventDatabase::GetInstance().GetDatabaseDir() + "/" + event->domain_;
    std::vector<std::string> files;
    FileUtil::GetDirFiles(domainDir, files);
    for (const auto& file : files) {
        if (IsFileMatched(event, file)) {
            fileList.emplace_back(file);
        }
    }
    return true;
}
 
void SysEventRepeatGuard::Check(std::shared_ptr<SysEvent> event)
{
    if (event->GetEventType() != SysEventCreator::EventType::FAULT) {
        return;
    }
    std::vector<std::string> fileList;
    GetMatchedFileList(event, fileList);
    for (const auto& fileName : fileList) {
        EventStore::ContentList contentList;
        EventStore::SysEventDocReader reader(fileName);
        reader.Read(contentList);
        if (IsEventRepeat(event, contentList)) {
            event->log_ = LOG_NOT_ALLOW_PACK|LOG_REPEAT;
            return;
        }
    }
    event->log_ = LOG_ALLOW_PACK|LOG_PACKED;
    return;
}

bool SysEventRepeatGuard::IsEventRepeat(std::shared_ptr<SysEvent> event, EventStore::ContentList& contentList)
{
    uint8_t* eventData = event->AsRawData();
    if (eventData == nullptr) {
        HIVIEW_LOGE("invalid eventData.");
        return false;
    }
    EventRawDataInfo<EventRaw::HiSysEventHeader> eventInfo(eventData);
    for (const auto& content : contentList) {
        EventRawDataInfo<EventStore::ContentHeader> contentInfo(content.get());
        if (!contentInfo.IsInTimeRange(GetMinValidTime()) || !contentInfo.IsLogPacked() ||
            (eventInfo.dataSize - eventInfo.dataPos) > (contentInfo.dataSize - contentInfo.dataPos)) {
            continue;
        }
        if (memcmp(eventData + eventInfo.dataPos, content.get() + contentInfo.dataPos,
            eventInfo.dataSize - eventInfo.dataPos) == 0) {
            return true;
        }
    }
    return false;
}
} // HiviewDFX
} // OHOS