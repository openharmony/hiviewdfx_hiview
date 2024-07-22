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

#include "event_json_parser.h"

#include <cctype>
#include <cinttypes>
#include <fstream>
#include <map>

#include "hiview_logger.h"
#include "parameter.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
DEFINE_LOG_TAG("Event-JsonParser");
constexpr uint64_t PRIME = 0x100000001B3ULL;
constexpr uint64_t BASIS = 0xCBF29CE484222325ULL;
constexpr char BASE[] = "__BASE";
constexpr char LEVEL[] = "level";
constexpr char TAG[] = "tag";
constexpr char TYPE[] = "type";
constexpr char PRIVACY[] = "privacy";
constexpr char PRESERVE[] = "preserve";
constexpr char TEST_TYPE_PARAM_KEY[] = "hiviewdfx.hiview.testtype";
constexpr char TEST_TYPE_KEY[] = "test_type_";
const std::map<std::string, uint8_t> EVENT_TYPE_MAP = {
    {"FAULT", 1}, {"STATISTIC", 2}, {"SECURITY", 3}, {"BEHAVIOR", 4}
};

uint64_t GenerateHash(std::shared_ptr<SysEvent> event)
{
    constexpr size_t infoLenLimit = 256;
    size_t infoLen = event->rawData_->GetDataLength();
    size_t hashLen = (infoLen < infoLenLimit) ? infoLen : infoLenLimit;
    const uint8_t* p = event->rawData_->GetData();
    uint64_t ret {BASIS};
    size_t i = 0;
    while (i < hashLen) {
        ret ^= *(p + i);
        ret *= PRIME;
        i++;
    }
    return ret;
}

void ParameterWatchCallback(const char* key, const char* value, void* context)
{
    if (context == nullptr) {
        HIVIEW_LOGE("context is null");
        return;
    }
    auto parser = reinterpret_cast<EventJsonParser*>(context);
    if (parser == nullptr) {
        HIVIEW_LOGE("parser is null");
        return;
    }
    size_t testTypeStrMaxLen = 256;
    std::string testTypeStr(value);
    if (testTypeStr.size() > testTypeStrMaxLen) {
        HIVIEW_LOGE("length of the test type string set exceeds the limit");
        return;
    }
    HIVIEW_LOGI("test_type is set to be \"%{public}s\"", testTypeStr.c_str());
    parser->UpdateTestType(testTypeStr);
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

bool DuplicateIdFilter::IsDuplicateEvent(const uint64_t sysEventId)
{
    for (auto iter = sysEventIds_.begin(); iter != sysEventIds_.end(); iter++) {
        if (*iter == sysEventId) {
            return true;
        }
    }
    FILTER_SIZE_TYPE maxSize { 5 }; // size of queue limit to 5
    if (sysEventIds_.size() >= maxSize) {
        sysEventIds_.pop_front();
    }
    sysEventIds_.emplace_back(sysEventId);
    return false;
}

EventJsonParser::EventJsonParser(const std::string& defFilePath)
{
    WatchTestTypeParameter();
    // read json file
    ReadDefFile(defFilePath);
}

void EventJsonParser::WatchTestTypeParameter()
{
    if (WatchParameter(TEST_TYPE_PARAM_KEY, ParameterWatchCallback, this) != 0) {
        HIVIEW_LOGW("failed to watch the change of parameter %{public}s", TEST_TYPE_PARAM_KEY);
    }
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
    if (!CheckEvent(event)) {
        return false;
    }
    AppendExtensiveInfo(event);
    return true;
}

void EventJsonParser::UpdateTestType(const std::string& testType)
{
    testType_ = testType;
}

bool EventJsonParser::CheckEvent(std::shared_ptr<SysEvent> event)
{
    if (event == nullptr) {
        HIVIEW_LOGW("event is null.");
        return false;
    }
    if (!CheckBaseInfo(event)) {
        return false;
    }
    if (!CheckDuplicate(event)) {
        return false;
    }
    return true;
}

bool EventJsonParser::CheckBaseInfo(std::shared_ptr<SysEvent> event) const
{
    if (event->domain_.empty() || event->eventName_.empty()) {
        HIVIEW_LOGW("domain=%{public}s or name=%{public}s is empty.",
            event->domain_.c_str(), event->eventName_.c_str());
        return false;
    }
    auto baseInfo = GetDefinedBaseInfoByDomainName(event->domain_, event->eventName_);
    if (baseInfo.type == INVALID_EVENT_TYPE) {
        HIVIEW_LOGW("type defined for event[%{public}s|%{public}s|%{public}" PRIu64 "] is invalid.",
            event->domain_.c_str(), event->eventName_.c_str(), event->happenTime_);
        return false;
    }
    if (event->GetEventType() != baseInfo.type) {
        HIVIEW_LOGW("type=%{public}d of event[%{public}s|%{public}s|%{public}" PRIu64 "] is invalid.",
            event->GetEventType(), event->domain_.c_str(), event->eventName_.c_str(), event->happenTime_);
        return false;
    }
    if (!baseInfo.level.empty()) {
        event->SetLevel(baseInfo.level);
    }
    if (!baseInfo.tag.empty()) {
        event->SetTag(baseInfo.tag);
    }
    event->SetPrivacy(baseInfo.privacy);
    return true;
}

BaseInfo EventJsonParser::GetDefinedBaseInfoByDomainName(const std::string& domain,
    const std::string& name) const
{
    auto domainIter = hiSysEventDefMap_->find(domain);
    if (domainIter == hiSysEventDefMap_->end()) {
        HIVIEW_LOGD("domain %{public}s is not defined.", domain.c_str());
        return BaseInfo();
    }
    auto domainNames = hiSysEventDefMap_->at(domain);
    auto nameIter = domainNames.find(name);
    if (nameIter == domainNames.end()) {
        HIVIEW_LOGD("%{public}s is not defined in domain %{public}s.", name.c_str(), domain.c_str());
        return BaseInfo();
    }
    return nameIter->second;
}

bool EventJsonParser::CheckDuplicate(std::shared_ptr<SysEvent> event)
{
    auto sysEventId = GenerateHash(event);
    if (filter_.IsDuplicateEvent(sysEventId)) {
        HIVIEW_LOGW("ignore duplicate event[%{public}s|%{public}s|%{public}" PRIu64 "].",
            event->domain_.c_str(), event->eventName_.c_str(), event->happenTime_);
        return false;
    }
    // hash code need to add
    event->SetId(sysEventId);
    return true;
}

void EventJsonParser::AppendExtensiveInfo(std::shared_ptr<SysEvent> event) const
{
    // add testtype configured as system property named persist.sys.hiview.testtype
    if (!testType_.empty()) {
        event->SetEventValue(TEST_TYPE_KEY, testType_);
    }

    event->SetTag(GetTagByDomainAndName(event->domain_, event->eventName_));
    event->preserve_ = GetPreserveByDomainAndName(event->domain_, event->eventName_);
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
    BaseInfo baseInfo;
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

    if (!HasStringMember(baseJsonInfo, LEVEL)) {
        HIVIEW_LOGD("level is not defined in __BASE.");
        return baseInfo;
    }
    baseInfo.level = baseJsonInfo[LEVEL].asString();

    if (HasStringMember(baseJsonInfo, TAG)) {
        baseInfo.tag = baseJsonInfo[TAG].asString();
    }

    if (HasIntMember(baseJsonInfo, PRIVACY)) {
        int privacy = baseJsonInfo[PRIVACY].asInt();
        baseInfo.privacy = privacy > 0 ? static_cast<uint8_t>(privacy) : baseInfo.privacy;
    }

    if (HasBoolMember(baseJsonInfo, PRESERVE)) {
        baseInfo.preserve = baseJsonInfo[PRESERVE].asBool();
    }

    return baseInfo;
}

void EventJsonParser::ParseHiSysEventDef(const Json::Value& hiSysEventDef, std::shared_ptr<DOMAIN_INFO_MAP> sysDefMap)
{
    InitEventInfoMapRef(hiSysEventDef, [this, sysDefMap] (const std::string& key, const Json::Value& value) {
       sysDefMap->insert(std::make_pair(key, this->ParseNameConfig(value)));
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

void EventJsonParser::ReadDefFile(const std::string& defFilePath)
{
    Json::Value hiSysEventDef;
    if (!ReadSysEventDefFromFile(defFilePath, hiSysEventDef)) {
        HIVIEW_LOGE("parse json file failed, please check the style of json file: %{public}s", defFilePath.c_str());
        return;
    }
    auto tmpMap = std::make_shared<DOMAIN_INFO_MAP>();
    ParseHiSysEventDef(hiSysEventDef, tmpMap);
    hiSysEventDefMap_ = tmpMap;
}
} // namespace HiviewDFX
} // namespace OHOS
