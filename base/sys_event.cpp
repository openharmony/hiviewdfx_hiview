/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "sys_event.h"

#include <chrono>
#include <regex>
#include <sstream>
#include <string>
#include <sys/time.h>

#include "string_util.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
static const std::vector<ParseItem> PARSE_ORDER = {
    {"domain_",     ":\"",  "\"",   nullptr, STATE_PARSING_DOMAIN,          true},
    {"name_",       ":\"",  "\"",   nullptr, STATE_PARSING_NAME,            true},
    {"type_",       ":",    ",",    nullptr, STATE_PARSING_TYPE,            true},
    {"time_",       ":",    ",",    nullptr, STATE_PARSING_TIME,            true},
    {"seq_",        ":",    ",",    "}",     STATE_PARSING_EVENT_SEQ,       true},
    {"tz_",         ":\"",  "\"",   nullptr, STATE_PARSING_TZONE,           true},
    {"pid_",        ":",    ",",    nullptr, STATE_PARSING_PID,             true},
    {"tid_",        ":",    ",",    nullptr, STATE_PARSING_TID,             true},
    {"uid_",        ":",    ",",    "}",     STATE_PARSING_UID,             true},
    {"traceid_",    ":\"",  "\"",   nullptr, STATE_PARSING_TRACE_ID,        false},
    {"spanid_",     ":\"",  "\"",   nullptr, STATE_PARSING_SPAN_ID,         false},
    {"pspanid_",    ":\"",  "\"",   nullptr, STATE_PARSING_PARENT_SPAN_ID,  false},
    {"trace_flag_", ":\"",  "\"",   nullptr, STATE_PARSING_TRACE_FLAG,      false},
};

std::atomic<uint32_t> SysEvent::totalCount_(0);
std::atomic<int64_t> SysEvent::totalSize_(0);

SysEvent::SysEvent(const std::string& sender, PipelineEventProducer* handler, const std::string& jsonStr)
    : PipelineEvent(sender, handler), seq_(0), pid_(0), tid_(0), uid_(0), tz_(0)
{
    messageType_ = Event::MessageType::SYS_EVENT;
    jsonExtraInfo_ = jsonStr;
    totalCount_.fetch_add(1);
    totalSize_.fetch_add(jsonStr.length());
}

SysEvent::SysEvent(const std::string& sender, PipelineEventProducer* handler, SysEventCreator& sysEventCreator)
    : SysEvent(sender, handler, sysEventCreator.BuildSysEventJson())
{
    ParseJson();
}

SysEvent::~SysEvent()
{
    if (totalCount_ > 0) {
        totalCount_.fetch_sub(1);
    }

    totalSize_.fetch_sub(jsonExtraInfo_.length());
    if (totalSize_ < 0) {
        totalSize_.store(0);
    }
}

int SysEvent::ParseJson()
{
    if (jsonExtraInfo_.empty()) {
        return -1;
    }
    size_t curPos = 0;
    for (auto ele = PARSE_ORDER.cbegin(); ele != PARSE_ORDER.cend(); ele++) {
        size_t keyPos = jsonExtraInfo_.find(ele->keyString, curPos);
        if (keyPos != std::string::npos) {
            size_t startPos = jsonExtraInfo_.find(ele->valueStart, keyPos);
            if (startPos == std::string::npos) {
                continue;
            }
            startPos += strlen(ele->valueStart);
            size_t endPos = jsonExtraInfo_.find(ele->valueEnd1, startPos);
            if (endPos == std::string::npos && ele->valueEnd2 != nullptr) {
                endPos = jsonExtraInfo_.find(ele->valueEnd2, startPos);
            }
            if (endPos != std::string::npos) {
                std::string content = jsonExtraInfo_.substr(startPos, endPos - startPos);
                InitialMember(ele->status, content);
                curPos = endPos;
            }
        } else {
            if (!ele->isParseContinue) {
                break;
            }
        }
    }
    if (domain_.empty() || eventName_.empty() || what_ == 0 || happenTime_ == 0) {
        return -1;
    }
    return 0;
}

void SysEvent::InitialMember(ParseStatus status, const std::string &content)
{
    switch (status) {
        case STATE_PARSING_DOMAIN:
            domain_ = content;
            break;
        case STATE_PARSING_NAME:
            eventName_ = content;
            break;
        case STATE_PARSING_TYPE:
            what_ = static_cast<uint16_t>(std::atoi(content.c_str()));
            break;
        case STATE_PARSING_TIME:
            happenTime_ = static_cast<uint64_t>(std::atoll(content.c_str()));
            break;
        case STATE_PARSING_EVENT_SEQ:
            eventSeq_ = static_cast<int64_t>(std::atoll(content.c_str()));
            break;
        case STATE_PARSING_TZONE:
            tz_ = std::atoi(content.c_str());
            break;
        case STATE_PARSING_PID:
            pid_ = std::atoi(content.c_str());
            break;
        case STATE_PARSING_TID:
            tid_ = std::atoi(content.c_str());
            break;
        case STATE_PARSING_UID:
            uid_ = std::atoi(content.c_str());
            break;
        case STATE_PARSING_TRACE_ID:
            traceId_ = content;
            break;
        case STATE_PARSING_SPAN_ID:
            spanId_ = content;
            break;
        case STATE_PARSING_PARENT_SPAN_ID:
            parentSpanId_ = content;
            break;
        case STATE_PARSING_TRACE_FLAG:
            traceFlag_ = content;
            break;
        default:
            break;
    }
}

void SysEvent::SetSeq(int64_t seq)
{
    seq_ = seq;
}

int64_t SysEvent::GetSeq() const
{
    return seq_;
}

int64_t SysEvent::GetEventSeq() const
{
    return eventSeq_;
}

int32_t SysEvent::GetPid() const
{
    return pid_;
}

int32_t SysEvent::GetTid() const
{
    return tid_;
}

int32_t SysEvent::GetUid() const
{
    return uid_;
}

int16_t SysEvent::GetTz() const
{
    return tz_;
}

std::string SysEvent::GetEventValue(const std::string& key)
{
    if (jsonExtraInfo_.empty() || key.empty()) {
        return "";
    }
    std::string targetStr = "\"" + key + "\":\"";
    size_t startPos = jsonExtraInfo_.find(targetStr);
    if (startPos == std::string::npos) {
        return "";
    }
    startPos += targetStr.size();

    size_t endPos = startPos;
    while (endPos < jsonExtraInfo_.size()) {
        if (jsonExtraInfo_[endPos] == '\"') {
            std::string value = jsonExtraInfo_.substr(startPos, endPos - startPos);
            if (!value.empty()) {
                SetValue(key, value);
            }
            return value;
        }
        if (jsonExtraInfo_[endPos] == '\\' && endPos < (jsonExtraInfo_.size() - 1)) { // 1: for '"'
            endPos += 2; // 2: for '\' and '"'
            continue;
        }
        endPos++;
    }
    return "";
}

uint64_t SysEvent::GetEventIntValue(const std::string& key)
{
    if (jsonExtraInfo_.empty() || key.empty()) {
        return 0;
    }
    std::string targetStr = "\"" + key + "\":";
    size_t startPos = jsonExtraInfo_.find(targetStr);
    if (startPos == std::string::npos) {
        return 0;
    }
    startPos += targetStr.size();

    size_t endPos = startPos;
    while (endPos < jsonExtraInfo_.size()) {
        if (!std::isdigit(jsonExtraInfo_[endPos])) {
            break;
        }
        endPos++;
    }
    return std::atoll(jsonExtraInfo_.substr(startPos, endPos - startPos).c_str());
}

void SysEvent::SetEventValue(const std::string& key, int64_t value)
{
    std::smatch keyMatch;
    std::string keyReplace = "\"" + key + "\":" + std::to_string(value);
    std::regex keyReg("\"" + key + "\":([\\d]*)");
    if (std::regex_search(jsonExtraInfo_, keyMatch, keyReg)) {
        jsonExtraInfo_ = std::regex_replace(jsonExtraInfo_, keyReg, keyReplace);
        return;
    }

    // new key here
    std::regex newReg("\\{([.\\s\\S\\r\\n]*)\\}");
    std::string newReplace = "{$1,\"" + key + "\":" + std::to_string(value) + "}";
    if (std::regex_search(jsonExtraInfo_, keyMatch, newReg)) {
        jsonExtraInfo_ = std::regex_replace(jsonExtraInfo_, newReg, newReplace);
    }
    else {
        jsonExtraInfo_ = "{\"" + key + "\":" + std::to_string(value) + "}";
    }
    return;
}

void SysEvent::SetEventValue(const std::string& key, const std::string& value, bool append)
{
    // fixme, $1 in value may cause error
    std::smatch keyMatch;
    std::string keyReplace;
    if (append) {
        keyReplace = "\"" + key + "\":\"" + value + ",$1\"";
    }
    else {
        keyReplace = "\"" + key + "\":\"" + value + "\"";
    }
    std::regex keyReg("\"" + key + "\":\"([.\\s\\S\\r\\n]*?)\"");
    if (std::regex_search(jsonExtraInfo_, keyMatch, keyReg)) {
        jsonExtraInfo_ = std::regex_replace(jsonExtraInfo_, keyReg, keyReplace);
        return;
    }

    // new key here
    std::regex newReg("\\{([.\\s\\S\\r\\n]*)\\}");
    auto kvStr = "\"" + key + "\":\"" + value + "\"";
    if (std::regex_search(jsonExtraInfo_, keyMatch, newReg)) {
        auto pos = jsonExtraInfo_.find_last_of("}");
        if (pos == std::string::npos) {
            return;
        }
        jsonExtraInfo_.insert(pos, "," + kvStr);
    } else {
        jsonExtraInfo_ = "{" + kvStr + "}";
    }
    return;
}

SysEventCreator::SysEventCreator(const std::string &domain, const std::string &eventName,
    SysEventCreator::EventType type)
{
    jsonStr_ << "{";
    SetKeyValue("domain_", domain);
    SetKeyValue("name_", eventName);
    SetKeyValue("type_", static_cast<int>(type));
    SetKeyValue("time_", TimeUtil::GetMilliseconds());
    SetKeyValue("tz_", TimeUtil::GetTimeZone());
    SetKeyValue("pid_", getpid());
    SetKeyValue("tid_", gettid());
    SetKeyValue("uid_", getuid());
}

std::string SysEventCreator::BuildSysEventJson()
{
    jsonStr_.seekp(-1, std::ios_base::end);
    jsonStr_ << "}";
    return jsonStr_.str();
}

std::string SysEventCreator::EscapeStringValue(const std::string& value)
{
    return StringUtil::EscapeJsonStringValue(value);
}

std::string SysEventCreator::EscapeStringValue(const char* value)
{
    return StringUtil::EscapeJsonStringValue(value);
}

std::string SysEventCreator::EscapeStringValue(char* value)
{
    return StringUtil::EscapeJsonStringValue(value);
}
} // namespace HiviewDFX
} // namespace OHOS
