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
#include "sys_event_query.h"
#include "sys_event_service_adapter.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr uint64_t PRIME = 0x100000001B3ull;
constexpr uint64_t BASIS = 0xCBF29CE484222325ull;
constexpr int INVALID_EVENT_TYPE = 0;
constexpr char BASE[] = "__BASE";
constexpr char DOMAIN_[] = "domain_";
constexpr char NAME_[] = "name_";
constexpr char ID_[] = "id_";
constexpr char LEVEL[] = "level";
constexpr char LEVEL_[] = "level_";
constexpr char TAG[] = "tag";
constexpr char TAG_[] = "tag_";
constexpr char TYPE[] = "type";
constexpr char TYPE_[] = "type_";
constexpr char TEST_TYPE_PARAM_KEY[] = "persist.sys.hiview.testtype";
constexpr char TEST_TYPE_KEY[] = "test_type_";
constexpr char SEQ_[] = "seq_";
constexpr char SEQ_PERSISTS_FILE_NAME[] = "event_sequence";
const std::map<std::string, int> EVENT_TYPE_MAP = {
    {"FAULT", 1}, {"STATISTIC", 2}, {"SECURITY", 3}, {"BEHAVIOR", 4}
};

std::string GenerateHash(const std::string& info)
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
    size_t hashRetLenLimit = 20; // max number of digits for uint64_t type
    std::string retStr = std::to_string(ret);
    size_t retLen = retStr.size();
    if (retLen < hashRetLenLimit) {
        retStr.append(hashRetLenLimit - retLen, '0'); // fill suffix digits with '0'
    }
    return retStr;
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
}

DEFINE_LOG_TAG("Event-JsonParser");

bool DuplicateIdFilter::IsDuplicateEvent(const std::string& sysEventId)
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

EventJsonParser::EventJsonParser(const std::string& path)
{
    Json::Value hiSysEventDef;
    std::ifstream fin(path, std::ifstream::binary);
#ifdef JSONCPP_VERSION_STRING
    Json::CharReaderBuilder jsonRBuilder;
    Json::CharReaderBuilder::strictMode(&jsonRBuilder.settings_);
    JSONCPP_STRING errs;
    if (!parseFromStream(jsonRBuilder, fin, &hiSysEventDef, &errs)) {
#else
    Json::Reader reader(Json::Features::strictMode());
    if (!reader.parse(fin, hiSysEventDef)) {
#endif
        HIVIEW_LOGE("parse json file failed, please check the style of json file: %{public}s", path.c_str());
    }
    ParseHiSysEventDef(hiSysEventDef);
    if (WatchParameter(TEST_TYPE_PARAM_KEY, ParameterWatchCallback, nullptr) != 0) {
        HIVIEW_LOGW("failed to watch the change of parameter %{public}s", TEST_TYPE_PARAM_KEY);
    }

    ReadSeqFromFile(curSeq);
}

std::string EventJsonParser::GetTagByDomainAndName(const std::string& domain, const std::string& name) const
{
    return GetDefinedBaseInfoByDomainName(domain, name).tag;
}

int EventJsonParser::GetTypeByDomainAndName(const std::string& domain, const std::string& name) const
{
    return GetDefinedBaseInfoByDomainName(domain, name).type;
}

bool EventJsonParser::HandleEventJson(const std::shared_ptr<SysEvent>& event)
{
    Json::Value eventJson;
    std::string jsonStr = event->jsonExtraInfo_;
#ifdef JSONCPP_VERSION_STRING
    Json::CharReaderBuilder jsonRBuilder;
    Json::CharReaderBuilder::strictMode(&jsonRBuilder.settings_);
    std::unique_ptr<Json::CharReader> const reader(jsonRBuilder.newCharReader());
    JSONCPP_STRING errs;
    if (!reader->parse(jsonStr.data(), jsonStr.data() + jsonStr.size(), &eventJson, &errs)) {
#else
    Json::Reader reader(Json::Features::strictMode());
    if (!reader.parse(jsonStr, eventJson)) {
#endif
        HIVIEW_LOGE("parse json file failed, please check the style of json file: %{public}s.", jsonStr.c_str());
        return false;
    }

    if (!CheckEventValidity(eventJson)) {
        HIVIEW_LOGD("domain_ or name_ not found in the event json string.");
        return false;
    }
    std::string domain = eventJson[DOMAIN_].asString();
    std::string name = eventJson[NAME_].asString();
    auto baseInfo = GetDefinedBaseInfoByDomainName(domain, name);
    if (baseInfo.type == INVALID_EVENT_TYPE) {
        HIVIEW_LOGD("type defined for domain: %{public}s, name: %{public}s is invalid.",
            domain.c_str(), name.c_str());
        return false;
    }
    if (!CheckBaseInfoValidity(baseInfo, eventJson)) {
        HIVIEW_LOGD("failed to verify the base info of the event.");
        return false;
    }
    auto curSysEventId = GenerateHash(jsonStr);
    if (filter.IsDuplicateEvent(curSysEventId)) {
        HIVIEW_LOGW("duplicate sys event, ignore it directly.");
        return false; // ignore duplicate sys event
    }
    AppendExtensiveInfo(eventJson, jsonStr, curSysEventId);
    WriteSeqToFile(++curSeq);
    event->jsonExtraInfo_ = jsonStr;
    return true;
}

void EventJsonParser::AppendExtensiveInfo(const Json::Value& eventJson, std::string& jsonStr,
    const std::string& sysEventId) const
{
    // this customized parser would maintain the original order of JSON key-value pairs
    FlatJsonParser parser(jsonStr);

    // cJsonArr need to add "level_" and "tag_" by hisysevent.def, "level" is must-option
    parser.AppendStringValue(LEVEL_, eventJson[LEVEL_].asString());
    if (eventJson.isMember(TAG_)) {
        parser.AppendStringValue(TAG_, eventJson[TAG_].asString());
    }

    // hash code need to add
    parser.AppendStringValue(ID_, sysEventId);

    // FreezeDetector needs to add
    parser.AppendStringValue(EventStore::EventCol::INFO.c_str(), "");

    // add testtype configured as system property named persist.sys.hiview.testtype
    if (!testTypeConfigured.empty()) {
        parser.AppendStringValue(TEST_TYPE_KEY, testTypeConfigured);
    }

    // add seq to sys event and then persist it into local file
    parser.AppendUInt64Value(SEQ_, static_cast<uint64_t>(curSeq));

    jsonStr = parser.Print();
}

bool EventJsonParser::CheckBaseInfoValidity(const BaseInfo& baseInfo, Json::Value& eventJson) const
{
    if (!CheckTypeValidity(baseInfo, eventJson)) {
        return false;
    }
    if (!baseInfo.level.empty()) {
        eventJson[LEVEL_] = baseInfo.level;
    }
    if (!baseInfo.tag.empty()) {
        eventJson[TAG_] = baseInfo.tag;
    }
    return true;
}

bool EventJsonParser::CheckEventValidity(const Json::Value& eventJson) const
{
    return HasStringMember(eventJson, DOMAIN_) && HasStringMember(eventJson, NAME_);
}

bool EventJsonParser::CheckTypeValidity(const BaseInfo& baseInfo, const Json::Value& eventJson) const
{
    if (!HasIntMember(eventJson, TYPE_)) {
        HIVIEW_LOGD("value of type_ found in the event json string need INT type.");
        return false;
    }
    return eventJson[TYPE_].asInt() == baseInfo.type;
}

BaseInfo EventJsonParser::GetDefinedBaseInfoByDomainName(const std::string& domain,
    const std::string& name) const
{
    BaseInfo baseInfo = {
        .type = INVALID_EVENT_TYPE,
        .level = "",
        .tag = ""
    };
    auto domainIter = hiSysEventDef.find(domain);
    if (domainIter == hiSysEventDef.end()) {
        HIVIEW_LOGD("domain named %{public}s is not defined.", domain.c_str());
        return baseInfo;
    }
    auto domaintNames = hiSysEventDef.at(domain);
    auto nameIter = domaintNames.find(name);
    if (nameIter == domaintNames.end()) {
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
        .tag = ""
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
    if (!baseJsonInfo.isObject() || !HasStringMember(baseJsonInfo, TAG)) {
        HIVIEW_LOGD("tag is not defined in __BASE.");
        return baseInfo;
    }
    baseInfo.tag = baseJsonInfo[TAG].asString();
    return baseInfo;
}

void EventJsonParser::ParseHiSysEventDef(const Json::Value& hiSysEventDef)
{
    InitEventInfoMapRef(hiSysEventDef, [this] (const std::string& key, const Json::Value& value) {
       this->hiSysEventDef[key] = ParseNameConfig(value);
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
