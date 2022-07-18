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
#include <fstream>
#include <map>

#include "cJSON.h"
#include "logger.h"
#include "sys_event_query.h"

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
const std::map<std::string, int> EVENT_TYPE_MAP = {
    {"FAULT", 1}, {"STATISTIC", 2}, {"SECURITY", 3}, {"BEHAVIOR", 4}
};

uint64_t GenerateHash(const Json::Value& info)
{
    uint64_t ret {BASIS};
    const char *p = reinterpret_cast<char*>(const_cast<Json::Value*>(&info));
    unsigned long i = 0;
    while (i < sizeof(Json::Value)) {
        ret ^= *(p + i);
        ret *= PRIME;
        i++;
    }
    return ret;
}
}

DEFINE_LOG_TAG("Event-JsonParser");

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
}

std::string EventJsonParser::GetTagByDomainAndName(const std::string& domain, const std::string& name) const
{
    return GetDefinedBaseInfoByDomainName(domain, name).tag;
}

int EventJsonParser::GetTypeByDomainAndName(const std::string& domain, const std::string& name) const
{
    return GetDefinedBaseInfoByDomainName(domain, name).type;
}

bool EventJsonParser::HandleEventJson(const std::shared_ptr<SysEvent>& event) const
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
        HIVIEW_LOGE("domain_ or name_ not found in the event json string.");
        return false;
    }
    std::string domain = eventJson[DOMAIN_].asString();
    std::string name = eventJson[NAME_].asString();
    auto baseInfo = GetDefinedBaseInfoByDomainName(domain, name);
    if (baseInfo.type == INVALID_EVENT_TYPE) {
        HIVIEW_LOGE("type defined for domain: %{public}s, name: %{public}s is invalid.",
            domain.c_str(), name.c_str());
        return false;
    }
    if (!CheckBaseInfoValidity(baseInfo, eventJson)) {
        HIVIEW_LOGE("failed to verify the base info of the event.");
        return false;
    }

    AppendExtensiveInfo(eventJson, jsonStr);
    event->jsonExtraInfo_ = jsonStr;
    return true;
}

void EventJsonParser::AppendExtensiveInfo(const Json::Value& eventJson, std::string& jsonStr) const
{
    // convert JsonValue to the correct order by event->jsonExtraInfo_
    cJSON *cJsonArr = cJSON_Parse(jsonStr.c_str());
    if (cJsonArr == NULL) {
        return;
    }

    // cJsonArr need to add "level_" and "tag_" by hisysevent.def, "level" is must-option
    cJSON_AddStringToObject(cJsonArr, LEVEL_, eventJson[LEVEL_].asString().c_str());
    if (eventJson.isMember(TAG_)) {
        cJSON_AddStringToObject(cJsonArr, TAG_, eventJson[TAG_].asString().c_str());
    }

    // hash code need to add
    cJSON_AddStringToObject(cJsonArr, ID_, std::to_string(GenerateHash(eventJson)).c_str());

    // FreezeDetector needs to add
    cJSON_AddStringToObject(cJsonArr, EventStore::EventCol::INFO.c_str(), "");
    char* jsonPtr =  cJSON_PrintUnformatted(cJsonArr);
    if (jsonPtr != nullptr) {
        jsonStr = std::string(jsonPtr);
        cJSON_free(jsonPtr);
    }
    cJSON_Delete(cJsonArr);
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
        HIVIEW_LOGE("value of type_ found in the event json string need INT type.");
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
        HIVIEW_LOGE("domain named %{public}s is not defined.", domain.c_str());
        return baseInfo;
    }
    auto domaintNames = hiSysEventDef.at(domain);
    auto nameIter = domaintNames.find(name);
    if (nameIter == domaintNames.end()) {
        HIVIEW_LOGE("%{public}s is not defined in domain named %{public}s.",
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
        HIVIEW_LOGE("__BASE definition is invalid.");
        return baseInfo;
    }
    Json::Value baseJsonInfo = eventNameJson[BASE];
    if (!baseJsonInfo.isObject() || !HasStringMember(baseJsonInfo, TYPE)) {
        HIVIEW_LOGE("type is not defined in __BASE.");
        return baseInfo;
    }
    std::string typeDes = baseJsonInfo[TYPE].asString();
    if (EVENT_TYPE_MAP.find(typeDes) == EVENT_TYPE_MAP.end()) {
        HIVIEW_LOGE("type is defined as %{public}s, but a valid type must be FAULT, STATISTIC, SECURITY, or BEHAVIOR",
            typeDes.c_str());
        return baseInfo;
    }
    baseInfo.type = EVENT_TYPE_MAP.at(typeDes);
    if (!baseJsonInfo.isObject() || !HasStringMember(baseJsonInfo, LEVEL)) {
        HIVIEW_LOGE("level is not defined in __BASE.");
        return baseInfo;
    }
    baseInfo.level = baseJsonInfo[LEVEL].asString();
    if (!baseJsonInfo.isObject() || !HasStringMember(baseJsonInfo, TAG)) {
        HIVIEW_LOGE("tag is not defined in __BASE.");
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
} // namespace HiviewDFX
} // namespace OHOS
