/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "event_threshold_manager.h"

#include <fstream>

#include "file_util.h"
#include "logger.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventThreshold {
DEFINE_LOG_TAG("HiView-EventThresholdManager");
namespace {
const std::string DEFAULT_THRESHOLD_FILE_NAME = "/system/etc/hiview/hisysevent_threshold.json";
constexpr char USERS_KEY[] = "USERS";
constexpr char NAME_KEY[] = "NAME";
constexpr char TYPE_KEY[] = "TYPE";
constexpr char CONFIGS_KEY[] = "CONFIGS";
constexpr char QUERY_RULE_LIMIT_KEY[] = "QUERY_RULE_LIMIT";
constexpr size_t DEFAULT_QUERY_RULE_LIMIT = 100;
constexpr size_t QUERY_RULE_MAX_LIMIT = 1000;

bool ReadEventThresholdConfigsFromFile(const std::string& path, Json::Value& thresholds)
{
    std::ifstream fin(path, std::ifstream::binary);
    Json::CharReaderBuilder jsonRBuilder;
    Json::CharReaderBuilder::strictMode(&jsonRBuilder.settings_);
    JSONCPP_STRING errs;
    return parseFromStream(jsonRBuilder, fin, &thresholds, &errs);
}
}

EventThresholdManager::EventThresholdManager()
{
    if (!ParseThresholdFile(DEFAULT_THRESHOLD_FILE_NAME)) {
        HIVIEW_LOGE("failed to parse threshold file: %{public}s.", DEFAULT_THRESHOLD_FILE_NAME.c_str());
        return;
    }
}

EventThresholdManager& EventThresholdManager::GetInstance()
{
    static EventThresholdManager instance;
    return instance;
}

size_t EventThresholdManager::GetQueryRuleLimit(const std::string& name, ProcessType type)
{
    for (auto& user : sysEventUsers_) {
        std::string userName;
        user.GetName(userName);
        if (name == userName && user.GetProcessType() == type) {
            Configs config;
            user.GetConfigs(config);
            return config.queryRuleLimit;
        }
    }
    return DEFAULT_QUERY_RULE_LIMIT;
}

size_t EventThresholdManager::GetDefaultQueryRuleLimit()
{
    return DEFAULT_QUERY_RULE_LIMIT;
}

bool EventThresholdManager::ParseThresholdFile(const std::string& path)
{
    if (!FileUtil::FileExists(path)) {
        HIVIEW_LOGE("threshold file not exist.");
        return false;
    }
    Json::Value thresholds;
    if (!ReadEventThresholdConfigsFromFile(path, thresholds)) {
        HIVIEW_LOGE("failed to parse threshold file.");
        return false;
    }
    return ParseSysEventUsers(thresholds, sysEventUsers_);
}

bool EventThresholdManager::ParseSysEventUsers(Json::Value& thresholdsJson, std::vector<SysEventUser>& users)
{
    users.clear();
    if (!thresholdsJson.isObject()) {
        HIVIEW_LOGE("thresholdjson is not object.");
        return false;
    }
    if (!thresholdsJson.isMember(USERS_KEY) || !thresholdsJson[USERS_KEY].isArray()) {
        HIVIEW_LOGE("USERS isn't configured or type matched in threshold file.");
        return false;
    }
    auto usersJson = thresholdsJson[USERS_KEY];
    auto size = usersJson.size();
    for (unsigned int index = 0; index < size; ++index) {
        SysEventUser user;
        if (!ParseUser(usersJson[index], user)) {
            continue;
        }
        users.emplace_back(user);
    }
    return true;
}

bool EventThresholdManager::ParseUser(Json::Value& userJson, SysEventUser& user)
{
    if (!userJson.isObject()) {
        HIVIEW_LOGE("user configured in array is not object.");
        return false;
    }
    if (!userJson.isMember(NAME_KEY) || !userJson[NAME_KEY].isString()) {
        HIVIEW_LOGE("NAME isn't configured or type matched in user.");
        return false;
    }
    auto configuredName = userJson[NAME_KEY].asString();
    user.SetName(configuredName);
    if (!userJson.isMember(TYPE_KEY) || !userJson[TYPE_KEY].isInt()) {
        HIVIEW_LOGE("TYPE isn't configured or type matched in user.");
        return false;
    }
    user.SetProcessType(ProcessType(userJson[TYPE_KEY].asInt()));
    if (!userJson.isMember(CONFIGS_KEY) || !userJson[CONFIGS_KEY].isObject()) {
        HIVIEW_LOGE("CONFIGS isn't configured or type matched in user.");
        return false;
    }
    Configs configs;
    if (!ParseConfigs(userJson[CONFIGS_KEY], configs)) {
        return false;
    }
    user.SetConfigs(configs);
    return true;
}

bool EventThresholdManager::ParseConfigs(Json::Value& configJson, Configs& config)
{
    if (!configJson.isMember(QUERY_RULE_LIMIT_KEY) || !configJson[QUERY_RULE_LIMIT_KEY].isUInt()) {
        HIVIEW_LOGE("QUERY_RULE_LIMIT_KEY isn't configured or type matched in configs.");
        return false;
    }
    config.queryRuleLimit = static_cast<size_t>(configJson[QUERY_RULE_LIMIT_KEY].asUInt());
    if (config.queryRuleLimit > QUERY_RULE_MAX_LIMIT) {
        config.queryRuleLimit = QUERY_RULE_MAX_LIMIT;
    }
    return true;
}
} // namespace EventThreshold
} // namespace HiviewDFX
} // namespace OHOS