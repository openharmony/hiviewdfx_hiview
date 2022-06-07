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
const char* BASE_EVENT_PAR[] = {"domain_", "name_", "type_", "time_", "level_",
    "uid_", "tag_", "tz_", "pid_", "tid_", "traceid_", "spanid_", "pspanid_", "trace_flag_"};
const std::map<std::string, int> EVENT_TYPE_MAP = {
    {"FAULT", 1}, {"STATISTIC", 2}, {"SECURITY", 3}, {"BEHAVIOR", 4}
};
constexpr uint64_t PRIME = 0x100000001B3ull;
constexpr uint64_t BASIS = 0xCBF29CE484222325ull;

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

EventJsonParser::EventJsonParser(const std::string &path) : isRootValid_(false)
{
    std::ifstream fin(path, std::ifstream::binary);
#ifdef JSONCPP_VERSION_STRING
    Json::CharReaderBuilder jsonRBuilder;
    Json::CharReaderBuilder::strictMode(&jsonRBuilder.settings_);
    JSONCPP_STRING errs;
    if (!parseFromStream(jsonRBuilder, fin, &root_, &errs)) {
#else
    Json::Reader reader(Json::Features::strictMode());
    if (!reader.parse(fin, root_)) {
#endif
        HIVIEW_LOGE("parse json file failed, please check the style of json file: %{public}s", path.c_str());
    } else {
        isRootValid_ = true;
    }
}

EventJsonParser::~EventJsonParser() {}

bool EventJsonParser::HandleEventJson(std::shared_ptr<SysEvent> &event) const
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
        HIVIEW_LOGE("parse json file failed, please check the style of json file: %{public}s", jsonStr.c_str());
        return false;
    }

    if (!HasDomainAndName(eventJson)) {
        HIVIEW_LOGE("the domain and name of the event do not exist");
        return false;
    }
    std::string domain = eventJson["domain_"].asString();
    std::string name = eventJson["name_"].asString();
    if (!CheckDomainAndName(domain, name)) {
        HIVIEW_LOGE("failed to verify the domain and name of the event");
        return false;
    }
    auto sysEventJson = root_[domain][name];
    if (!CheckBaseInfo(sysEventJson["__BASE"], eventJson)) {
        HIVIEW_LOGE("failed to verify the base info of the event");
        return false;
    }

    auto eventNameList = eventJson.getMemberNames();
    for (auto it = eventNameList.cbegin(); it != eventNameList.cend(); it++) {
        std::string ps = *it;
        if (std::find_if(std::cbegin(BASE_EVENT_PAR), std::cend(BASE_EVENT_PAR), [ps](const char* ele) {
            return (ps.compare(ele) == 0); }) == std::cend(BASE_EVENT_PAR)) {
            if (!CheckExtendInfo(ps, sysEventJson, eventJson)) {
                HIVIEW_LOGI("failed to verify the extend info, need to remove %{public}s ", ps.c_str());
                eventJson.removeMember(ps);
            }
        }
    }
    GetOrderlyJsonInfo(eventJson, jsonStr);
    event->jsonExtraInfo_ = jsonStr;
    return true;
}

bool EventJsonParser::IsIntValue(const Json::Value &jsonObj, const std::string &name) const
{
    return jsonObj.isMember(name.c_str()) && jsonObj[name.c_str()].isInt();
}

bool EventJsonParser::IsStringValue(const Json::Value &jsonObj, const std::string &name) const
{
    return jsonObj.isMember(name.c_str()) && jsonObj[name.c_str()].isString();
}

bool EventJsonParser::IsObjectValue(const Json::Value &jsonObj, const std::string &name) const
{
    return jsonObj.isMember(name.c_str()) && jsonObj[name.c_str()].isObject();
}

bool EventJsonParser::IsNullValue(const Json::Value &jsonObj, const std::string &name) const
{
    return jsonObj.isMember(name.c_str()) && jsonObj[name.c_str()].isNull();
}

bool EventJsonParser::HasDomainAndName(const Json::Value &eventJson) const
{
    return IsStringValue(eventJson, "domain_") &&  IsStringValue(eventJson, "name_");
}

bool EventJsonParser::CheckDomainAndName(const std::string &domain, const std::string &name) const
{
    return isRootValid_ && root_.isObject() && IsObjectValue(root_, domain)
        && IsObjectValue(root_[domain], name) && !IsNullValue(root_[domain][name], "__BASE");
}

bool EventJsonParser::CheckEventType(const Json::Value &sysBaseJson, const Json::Value &eventJson) const
{
    if (!IsIntValue(eventJson, "type_")) {
        return false;
    }
    std::string typeStr = sysBaseJson["type"].asString();
    if (EVENT_TYPE_MAP.find(typeStr) == EVENT_TYPE_MAP.end()) {
        return false;
    }
    return EVENT_TYPE_MAP.at(typeStr) == eventJson["type_"].asInt();
}

bool EventJsonParser::CheckBaseInfo(const Json::Value &sysBaseJson, Json::Value &eventJson) const
{
    if (!CheckEventType(sysBaseJson, eventJson)) {
        return false;
    }
    if (!sysBaseJson.isMember("level")) {
        return false;
    }
    eventJson["level_"] = sysBaseJson["level"].asString();
    if (sysBaseJson.isMember("tag")) {
        eventJson["tag_"] = sysBaseJson["tag"].asString();
    }
    return true;
}

bool EventJsonParser::CheckExtendInfo(const std::string &name, const Json::Value &sysEvent,
    const Json::Value &eventJson) const
{
    if (!sysEvent.isMember(name)) {
        return false;
    }
    if (sysEvent[name].isMember("arrsize")) {
        if (!eventJson[name].isArray() || eventJson[name].size() > sysEvent[name]["arrsize"].asUInt()) {
            return false;
        }
        return JudgeDataType(sysEvent[name]["type"].asString(), eventJson[name][0]);
    }
    return JudgeDataType(sysEvent[name]["type"].asString(), eventJson[name]);
}

bool EventJsonParser::JudgeDataType(const std::string &dataType, const Json::Value &eventJson) const
{
    if (dataType.compare("BOOL") == 0) {
        return eventJson.isBool();
    } else if ((dataType.compare("INT8") == 0) || (dataType.compare("INT16") == 0) ||
        (dataType.compare("INT32") == 0)) {
        return eventJson.isInt();
    } else if (dataType.compare("INT64") == 0) {
        return eventJson.isInt64();
    } else if ((dataType.compare("UINT8") == 0) || (dataType.compare("UINT16") == 0) ||
        (dataType.compare("UINT32") == 0)) {
        return eventJson.isUInt();
    } else if (dataType.compare("UINT64") == 0) {
        return eventJson.isUInt64();
    } else if ((dataType.compare("FLOAT") == 0) || (dataType.compare("DOUBLE") == 0)) {
        return eventJson.isDouble();
    } else if (dataType.compare("STRING") == 0) {
        return eventJson.isString();
    } else {
        return false;
    }
}

void EventJsonParser::GetOrderlyJsonInfo(const Json::Value &eventJson, std::string &jsonStr) const
{
    // convert JsonValue to the correct order by event->jsonExtraInfo_
    cJSON *cJsonArr = cJSON_Parse(jsonStr.c_str());
    if (cJsonArr == NULL) {
        return;
    }
    int endJson = cJSON_GetArraySize(cJsonArr) - 1;
    cJSON *item = NULL;

    // cJsonArr need to delete the item that failed the check by hisysevent.def
    for (int i = endJson; i >= 0; i--) {
        item = cJSON_GetArrayItem(cJsonArr, i);
        if (!eventJson.isMember(item->string)) {
            cJSON_DeleteItemFromArray(cJsonArr, i);
        }
    }

    // cJsonArr need to add "level_" and "tag_" by hisysevent.def, "level" is must-option
    cJSON_AddStringToObject(cJsonArr, "level_", eventJson["level_"].asString().c_str());
    if (eventJson.isMember("tag_")) {
        cJSON_AddStringToObject(cJsonArr, "tag_", eventJson["tag_"].asString().c_str());
    }

    // hash code need to add
    cJSON_AddStringToObject(cJsonArr, "id_", std::to_string(GenerateHash(eventJson)).c_str());

    // FreezeDetector needs to add
    cJSON_AddStringToObject(cJsonArr, EventStore::EventCol::INFO.c_str(), "");
    jsonStr = cJSON_PrintUnformatted(cJsonArr);
    cJSON_Delete(cJsonArr);
    return;
}

std::string EventJsonParser::GetTagByDomainAndName(const std::string &domain, const std::string &eventName) const
{
    if (!CheckDomainAndName(domain, eventName)) {
        return "";
    }
    auto baseInfo = root_[domain][eventName]["__BASE"];
    return IsStringValue(baseInfo, "tag") ? baseInfo["tag"].asString() : "";
}

int EventJsonParser::GetTypeByDomainAndName(const std::string &domain, const std::string &eventName) const
{
    if (!CheckDomainAndName(domain, eventName)) {
        return 0;
    }
    auto baseInfo = root_[domain][eventName]["__BASE"];
    if (!IsStringValue(baseInfo, "type")) {
        return 0;
    }
    std::string typeStr = baseInfo["type"].asString();
    return EVENT_TYPE_MAP.find(typeStr) == EVENT_TYPE_MAP.end() ? 0 : EVENT_TYPE_MAP.at(typeStr);
}
} // namespace HiviewDFX
} // namespace OHOS
