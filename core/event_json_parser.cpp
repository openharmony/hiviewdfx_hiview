/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "event_json_parser.h"

#include <algorithm>
#include <cctype>
#include <cinttypes>
#include <fstream>
#include <map>
#include <cstdlib>

#include "file_util.h"
#include "flat_json_parser.h"
#include "hiview_global.h"
#include "logger.h"
#include "parameter.h"
#include "sys_event_service_adapter.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr uint64_t PRIME = 0x100000001B3ull;
constexpr uint64_t BASIS = 0xCBF29CE484222325ull;
constexpr int INVALID_EVENT_TYPE = 0;
constexpr char BASE[] = "__BASE";
constexpr char LEVEL[] = "level";
constexpr char TAG[] = "tag";
constexpr char TYPE[] = "type";
constexpr char PRESERVE[] = "preserve";
constexpr char TEST_TYPE_PARAM_KEY[] = "persist.sys.hiview.testtype";
constexpr char TEST_TYPE_KEY[] = "test_type_";
constexpr char SEQ_PERSISTS_FILE_NAME[] = "event_sequence";
const std::map<std::string, uint8_t> EVENT_TYPE_MAP = {
    {"FAULT", 1}, {"STATISTIC", 2}, {"SECURITY", 3}, {"BEHAVIOR", 4}
};

uint64_t GenerateHash(const std::string& info)
{
    uint64_t ret {BASIS};
    const char* p = info.c_str();
    size_t infoLen = info.size();
    size_t infoLenLimit = 256;
    size_t hashLen = (infoLen < infoLenLimit) ? infoLen : infoLenLimit;
    size_t i = 0;
    while (i < hashLen) {
        ret ^= *(p + i);
        ret *= PRIME;
        i++;
    }
    return ret;
}

std::string GetConfiguredTestType(const std::string& configuredType)
{
    std::string defaultType {""};
    size_t maxLen = 12;
    if (configuredType.empty() || configuredType.length() > maxLen ||
        any_of(configuredType.cbegin(), configuredType.cend(), [] (char c) {
            return !isalnum(c);
        })) {
        return defaultType;
    }
    return configuredType;
}

static std::string testTypeConfigured;
void ParameterWatchCallback(const char* key, const char* value, void* context)
{
    testTypeConfigured = GetConfiguredTestType(value);
}

bool ReadSysEventDefFromFile(const std::string& path, Json::Value& hiSysEventDef)
{
    std::ifstream fin(path, std::ifstream::binary);
    Json::CharReaderBuilder jsonRBuilder;
    Json::CharReaderBuilder::strictMode(&jsonRBuilder.settings_);
    JSONCPP_STRING errs;
    return parseFromStream(jsonRBuilder, fin, &hiSysEventDef, &errs);
}
}

DEFINE_LOG_TAG("Event-JsonParser");

bool DuplicateIdFilter::IsDuplicateEvent(const uint64_t sysEventId)
{
    for (auto iter = sysEventIds.begin(); iter != sysEventIds.end(); iter++) {
        if (*iter == sysEventId) {
            return true;
        }
    }
    FILTER_SIZE_TYPE maxSize { 5 }; // size of queue limit to 5
    if (sysEventIds.size() >= maxSize) {
        sysEventIds.pop_front();
    }
    sysEventIds.emplace_back(sysEventId);
    return false;
}

EventJsonParser::EventJsonParser(std::vector<std::string>& paths)
{
    Json::Value hiSysEventDef;
    for (auto path : paths) {
        if (!ReadSysEventDefFromFile(path, hiSysEventDef)) {
            HIVIEW_LOGE("parse json file failed, please check the style of json file: %{public}s", path.c_str());
        }
        ParseHiSysEventDef(hiSysEventDef);
    }
    WatchParameterAndReadLatestSeq();
}

void EventJsonParser::WatchParameterAndReadLatestSeq()
{
    if (WatchParameter(TEST_TYPE_PARAM_KEY, ParameterWatchCallback, nullptr) != 0) {
        HIVIEW_LOGW("failed to watch the change of parameter %{public}s", TEST_TYPE_PARAM_KEY);
    }
    ReadSeqFromFile(curSeq_);
}

std::string EventJsonParser::GetTagByDomainAndName(const std::string& domain, const std::string& name) const
{
    return GetDefinedBaseInfoByDomainName(domain, name).tag;
}

int EventJsonParser::GetTypeByDomainAndName(const std::string& domain, const std::string& name) const
{
    return GetDefinedBaseInfoByDomainName(domain, name).type;
}

bool EventJsonParser::GetPreserveByDomainAndName(const std::string& domain, const std::string& name) const
{
    return GetDefinedBaseInfoByDomainName(domain, name).preserve;
}

bool EventJsonParser::HandleEventJson(const std::shared_ptr<SysEvent>& event)
{
    if (event == nullptr) {
        HIVIEW_LOGD("sysevent is null.");
        return false;
    }

    if (!CheckEventValidity(event)) {
        HIVIEW_LOGD("domain_ or name_ not found in the event json string.");
        return false;
    }

    auto baseInfo = GetDefinedBaseInfoByDomainName(event->domain_, event->eventName_);
    if (baseInfo.type == INVALID_EVENT_TYPE) {
        HIVIEW_LOGD("type defined for domain: %{public}s, name: %{public}s is invalid.",
            event->domain_.c_str(), event->eventName_.c_str());
        return false;
    }
    if (!CheckBaseInfoValidity(baseInfo, event)) {
        HIVIEW_LOGD("failed to verify the base info of the event.");
        return false;
    }

    auto curSysEventId = GenerateHash(event->AsJsonStr());
    if (filter_.IsDuplicateEvent(curSysEventId)) {
        HIVIEW_LOGD("duplicate sys event, ignore it directly.");
        return false; // ignore duplicate sys event
    }

    AppendExtensiveInfo(event, curSysEventId);
    WriteSeqToFile(++curSeq_);

    return true;
}

void EventJsonParser::AppendExtensiveInfo(std::shared_ptr<SysEvent> event, const uint64_t sysEventId) const
{
    if (event == nullptr) {
        return;
    }

    // hash code need to add
    event->SetId(sysEventId);

    // FreezeDetector needs to add
    event->SetEventValue(EventStore::EventCol::INFO, "");

    // add testtype configured as system property named persist.sys.hiview.testtype
    if (!testTypeConfigured.empty()) {
        event->SetEventValue(TEST_TYPE_KEY, testTypeConfigured);
    }

    // add seq to sys event and then persist it into local file
    event->SetEventSeq(curSeq_);
}

bool EventJsonParser::CheckBaseInfoValidity(const BaseInfo& baseInfo, std::shared_ptr<SysEvent> event) const
{
    if (!CheckTypeValidity(baseInfo, event)) {
        return false;
    }
    if (!baseInfo.level.empty()) {
        event->SetLevel(baseInfo.level);
    }
    if (!baseInfo.tag.empty()) {
        event->SetTag(baseInfo.tag);
    }
    return true;
}

bool EventJsonParser::CheckEventValidity(std::shared_ptr<SysEvent> event) const
{
    if (event == nullptr) {
        return false;
    }
    return !(event->domain_.empty()) && !(event->eventName_.empty());
}

bool EventJsonParser::CheckTypeValidity(const BaseInfo& baseInfo, std::shared_ptr<SysEvent> event) const
{
    if (event == nullptr) {
        return false;
    }
    HIVIEW_LOGD("base info type is %{public}d, event type is %{public}d", baseInfo.type, event->GetEventType());
    return event->GetEventType() == baseInfo.type;
}

BaseInfo EventJsonParser::GetDefinedBaseInfoByDomainName(const std::string& domain,
    const std::string& name) const
{
    BaseInfo baseInfo = {
        .type = INVALID_EVENT_TYPE,
        .level = "",
        .tag = "",
        .preserve = true
    };
    auto domainIter = hiSysEventDef_.find(domain);
    if (domainIter == hiSysEventDef_.end()) {
        HIVIEW_LOGD("domain named %{public}s is not defined.", domain.c_str());
        return baseInfo;
    }
    auto domainNames = hiSysEventDef_.at(domain);
    auto nameIter = domainNames.find(name);
    if (nameIter == domainNames.end()) {
        HIVIEW_LOGD("%{public}s is not defined in domain named %{public}s.",
            name.c_str(), domain.c_str());
        return baseInfo;
    }
    return nameIter->second;
}

bool EventJsonParser::HasIntMember(const Json::Value& jsonObj, const std::string& name) const
{
    return jsonObj.isMember(name.c_str()) && jsonObj[name.c_str()].isInt();
}

bool EventJsonParser::HasStringMember(const Json::Value& jsonObj, const std::string& name) const
{
    return jsonObj.isMember(name.c_str()) && jsonObj[name.c_str()].isString();
}

bool EventJsonParser::HasBoolMember(const Json::Value& jsonObj, const std::string& name) const
{
    return jsonObj.isMember(name.c_str()) && jsonObj[name.c_str()].isBool();
}

void EventJsonParser::InitEventInfoMapRef(const Json::Value& eventJson, JSON_VALUE_LOOP_HANDLER handler) const
{
    if (!eventJson.isObject()) {
        return;
    }
    auto attrList = eventJson.getMemberNames();
    for (auto it = attrList.cbegin(); it != attrList.cend(); it++) {
        std::string key = *it;
        if (key.empty()) {
            continue;
        }
        if (handler != nullptr) {
            handler(key, eventJson[key]);
        }
    }
}

BaseInfo EventJsonParser::ParseBaseConfig(const Json::Value& eventNameJson) const
{
    BaseInfo baseInfo = {
        .type = INVALID_EVENT_TYPE,
        .level = "",
        .tag = "",
        .preserve = true
    };
    if (!eventNameJson.isObject() || !eventNameJson[BASE].isObject()) {
        HIVIEW_LOGD("__BASE definition is invalid.");
        return baseInfo;
    }
    Json::Value baseJsonInfo = eventNameJson[BASE];
    if (!baseJsonInfo.isObject() || !HasStringMember(baseJsonInfo, TYPE)) {
        HIVIEW_LOGD("type is not defined in __BASE.");
        return baseInfo;
    }
    std::string typeDes = baseJsonInfo[TYPE].asString();
    if (EVENT_TYPE_MAP.find(typeDes) == EVENT_TYPE_MAP.end()) {
        HIVIEW_LOGD("type is defined as %{public}s, but a valid type must be FAULT, STATISTIC, SECURITY, or BEHAVIOR",
            typeDes.c_str());
        return baseInfo;
    }
    baseInfo.type = EVENT_TYPE_MAP.at(typeDes);
    if (!baseJsonInfo.isObject() || !HasStringMember(baseJsonInfo, LEVEL)) {
        HIVIEW_LOGD("level is not defined in __BASE.");
        return baseInfo;
    }
    baseInfo.level = baseJsonInfo[LEVEL].asString();
    if (baseJsonInfo.isObject() && HasStringMember(baseJsonInfo, TAG)) {
        baseInfo.tag = baseJsonInfo[TAG].asString();
    }
    if (baseJsonInfo.isObject() && HasBoolMember(baseJsonInfo, PRESERVE)) {
        baseInfo.preserve = baseJsonInfo[PRESERVE].asBool();
        return baseInfo;
    }
    return baseInfo;
}

void EventJsonParser::ParseHiSysEventDef(const Json::Value& hiSysEventDef)
{
    InitEventInfoMapRef(hiSysEventDef, [this] (const std::string& key, const Json::Value& value) {
       hiSysEventDef_[key] = ParseNameConfig(value);
    });
}

NAME_INFO_MAP EventJsonParser::ParseNameConfig(const Json::Value& domainJson) const
{
    NAME_INFO_MAP allNames;
    if (!domainJson.isObject()) {
        return allNames;
    }
    InitEventInfoMapRef(domainJson, [this, &allNames] (const std::string& key, const Json::Value& value) {
        allNames[key] = ParseBaseConfig(value);
    });
    return allNames;
}

std::string EventJsonParser::GetSequenceFile() const
{
    std::string workPath = HiviewGlobal::GetInstance()->GetHiViewDirectory(
        HiviewContext::DirectoryType::WORK_DIRECTORY);
    if (workPath.back() != '/') {
        workPath = workPath + "/";
    }
    return workPath + "sys_event_db/" + SEQ_PERSISTS_FILE_NAME;
}

void EventJsonParser::ReadSeqFromFile(int64_t& seq)
{
    std::string content;
    std::string seqFile = GetSequenceFile();
    if (!FileUtil::LoadStringFromFile(seqFile, content) && !content.empty()) {
        HIVIEW_LOGE("failed to read sequence value from %{public}s.", seqFile.c_str());
        return;
    }
    seq = static_cast<int64_t>(strtoll(content.c_str(), nullptr, 0));
    HIVIEW_LOGI("read max sequence from local file successful, value is %{public}" PRId64 ".", seq);
    SysEventServiceAdapter::UpdateEventSeq(seq);
}

void EventJsonParser::WriteSeqToFile(int64_t seq) const
{
    std::string seqFile = GetSequenceFile();
    std::string content = std::to_string(seq);
    if (!FileUtil::SaveStringToFile(seqFile, content, true)) {
        HIVIEW_LOGE("failed to write sequence %{public}s to %{public}s.", content.c_str(), seqFile.c_str());
    }
    SysEventServiceAdapter::UpdateEventSeq(seq);
}
} // namespace HiviewDFX
} // namespace OHOS
