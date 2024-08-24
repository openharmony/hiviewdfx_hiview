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

namespace OHOS {
namespace HiviewDFX {
namespace {
DEFINE_LOG_TAG("Event-JsonParser");

constexpr char BASE[] = "__BASE";
constexpr char LEVEL[] = "level";
constexpr char TAG[] = "tag";
constexpr char TYPE[] = "type";
constexpr char PRIVACY[] = "privacy";
constexpr char PRESERVE[] = "preserve";
const std::map<std::string, uint8_t> EVENT_TYPE_MAP = {
    {"FAULT", 1}, {"STATISTIC", 2}, {"SECURITY", 3}, {"BEHAVIOR", 4}
};

bool ReadSysEventDefFromFile(const std::string& path, Json::Value& hiSysEventDef)
{
    std::ifstream fin(path, std::ifstream::binary);
    Json::CharReaderBuilder jsonRBuilder;
    Json::CharReaderBuilder::strictMode(&jsonRBuilder.settings_);
    JSONCPP_STRING errs;
    return parseFromStream(jsonRBuilder, fin, &hiSysEventDef, &errs);
}
}

EventJsonParser::EventJsonParser(const std::string& defFilePath)
{
    // read json file
    ReadDefFile(defFilePath);
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

BaseInfo EventJsonParser::GetDefinedBaseInfoByDomainName(const std::string& domain,
    const std::string& name) const
{
    if (hiSysEventDefMap_ == nullptr) {
        HIVIEW_LOGD("sys def map is null");
        return BaseInfo();
    }
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
