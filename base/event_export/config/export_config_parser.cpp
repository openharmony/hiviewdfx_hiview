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

#include "export_config_parser.h"

#include <fstream>

#include "cjson_util.h"
#include "export_event_list_parser.h"
#include "hiview_logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-EventConfigParser");
using  ExportEventListParsers = std::map<std::string, std::shared_ptr<ExportEventListParser>>;
namespace {
constexpr char SETTING_DB_PARAMS[] = "settingDbParams";
constexpr char SETTING_DB_PARAM_NAME[] = "name";
constexpr char SETTING_DB_ENABLED[] = "enabledValue";
constexpr char SETTING_DB_DISABLED[] = "disabledValue";
constexpr char EXPORT_DIR[] = "exportDir";
constexpr char EXPORT_DIR_MAX_CAPACITY[] = "exportDirMaxCapacity";
constexpr char EXPORT_SINGLE_FILE_MAX_SIZE[] = "exportSingleFileMaxSize";
constexpr char TASK_EXECUTING_CYCLE[] = "taskExecutingCycle";
constexpr char EXPORT_EVENT_LIST_CONFIG_PATHS[] = "exportEventListConfigPaths";
constexpr char FILE_STORED_MAX_DAY_CNT[] = "fileStoredMaxDayCnt";
constexpr int32_t INVALID_INT_VAL = -1;
constexpr double INVALID_DOUBLE_VAL = -1.0;
constexpr size_t EXPORT_ABILITY_PARAM_INDEX = 0;
constexpr size_t UPGRADE_ABILITY_PARAM_INDEX = 1;

std::shared_ptr<ExportEventListParser> GetParser(ExportEventListParsers& parsers,
    const std::string& path)
{
    auto iter = parsers.find(path);
    if (iter == parsers.end()) {
        parsers.emplace(path, std::make_shared<ExportEventListParser>(path));
        return parsers[path];
    }
    return iter->second;
}

bool AddConfiguredParamToList(cJSON* paramItem, std::vector<SettingDbParam>& paramList)
{
    if (paramItem == nullptr || !cJSON_IsObject(paramItem)) {
        HIVIEW_LOGW("setting parameter configured is invalid");
        return false;
    }
    SettingDbParam param {
        .name = "",
        .enabledVal = "",
        .disabledVal = "",
    };
    param.name = CJsonUtil::GetStringValue(paramItem, SETTING_DB_PARAM_NAME);
    if (param.name.empty()) {
        HIVIEW_LOGE("name of setting db parameter configured is empty.");
        return false;
    }
    param.enabledVal = CJsonUtil::GetStringValue(paramItem, SETTING_DB_ENABLED);
    if (param.enabledVal.empty()) {
        HIVIEW_LOGW("enabled value of setting db parameter configured is invalid.");
    }
    param.disabledVal = CJsonUtil::GetStringValue(paramItem, SETTING_DB_DISABLED);
    if (param.disabledVal.empty()) {
        HIVIEW_LOGW("disabled value of setting db parameter configured is invalid.");
    }
    return true;
}

SettingDbParam GetParamByIndex(const std::vector<SettingDbParam> params, const size_t index)
{
    SettingDbParam invalidParam {
        .name = "",
        .enabledVal = "",
        .disabledVal = "",
    };
    if (params.size() <= index) {
        return invalidParam;
    }
    return params[index];
}
}

bool SettingDbParam::IsInValid()
{
    return name.empty();
}

SettingDbParam ExportConfig::GetExportAbilityParam()
{
    return GetParamByIndex(settingDbParams, EXPORT_ABILITY_PARAM_INDEX);
}

SettingDbParam ExportConfig::GetUpgradeAbilityParam()
{
    return GetParamByIndex(settingDbParams, UPGRADE_ABILITY_PARAM_INDEX);
}

ExportConfigParser::ExportConfigParser(const std::string& configFile)
{
    jsonRoot_ = CJsonUtil::ParseJsonRoot(configFile);
}

ExportConfigParser::~ExportConfigParser()
{
    if (jsonRoot_ == nullptr) {
        return;
    }
    cJSON_Delete(jsonRoot_);
}

std::shared_ptr<ExportConfig> ExportConfigParser::Parse()
{
    auto exportConfig = std::make_shared<ExportConfig>();
    if (jsonRoot_ == nullptr || !cJSON_IsObject(jsonRoot_)) {
        HIVIEW_LOGE("the file format of export config file is not json.");
        return nullptr;
    }
    // read export event list
    if (!ParseExportEventList(exportConfig->eventList)) {
        HIVIEW_LOGE("failed to parse export event list.");
        return nullptr;
    }
    // parse setting db param list
    if (!ParseSettingDbParamList(exportConfig->settingDbParams)) {
        HIVIEW_LOGE("failed to parse setting db parameter list.");
        return nullptr;
    }
    // parse residual content of the config file
    if (!ParseResidualContent(exportConfig)) {
        HIVIEW_LOGE("failed to parse residual content.");
        return nullptr;
    }
    return exportConfig;
}

bool ExportConfigParser::ParseExportEventList(ExportEventList& list)
{
    std::vector<std::string> eventListFilePaths;
    CJsonUtil::GetStringArray(jsonRoot_, EXPORT_EVENT_LIST_CONFIG_PATHS, eventListFilePaths);
    ExportEventListParsers parsers;
    auto iter = std::max_element(eventListFilePaths.begin(), eventListFilePaths.end(),
        [&parsers] (const std::string& path1, const std::string& path2) {
            auto p1 = GetParser(parsers, path1);
            auto p2 = GetParser(parsers, path2);
            return p1->GetConfigurationVersion() < p2->GetConfigurationVersion();
        });
    if (iter == eventListFilePaths.end()) {
        HIVIEW_LOGE("no event list file path is configured.");
        return false;
    }
    HIVIEW_LOGD("event list file path is %{public}s", (*iter).c_str());
    auto parser = GetParser(parsers, *iter);
    parser->GetExportEventList(list);
    return true;
}

bool ExportConfigParser::ParseSettingDbParamList(std::vector<SettingDbParam>& settingDbParams)
{
    // read exportAbilitySwitchParam
    cJSON* settingDbParamsJson = cJSON_GetObjectItem(jsonRoot_, SETTING_DB_PARAMS);
    if (settingDbParamsJson == nullptr || !cJSON_IsArray(settingDbParamsJson)) {
        HIVIEW_LOGW("setting parameter list configured is invalid.");
        return false;
    }
    int paramSize = cJSON_GetArraySize(settingDbParamsJson);
    if (paramSize <= 0) {
        HIVIEW_LOGW("setting parameter list configured is empty");
        return false;
    }
    for (int index = 0; index < paramSize; ++paramSize) {
        cJSON* paramItem = cJSON_GetArrayItem(settingDbParamsJson, index);
        if (!AddConfiguredParamToList(paramItem, settingDbParams)) {
            return false;
        }
    }
    return true;
}

bool ExportConfigParser::ParseResidualContent(std::shared_ptr<ExportConfig> config)
{
    // read export diectory
    config->exportDir = CJsonUtil::GetStringValue(jsonRoot_, EXPORT_DIR);
    if (config->exportDir.empty()) {
        HIVIEW_LOGW("exportDirectory configured is invalid.");
        return false;
    }
    // read maximum capacity of the export diectory
    config->maxCapcity = CJsonUtil::GetIntValue(jsonRoot_, EXPORT_DIR_MAX_CAPACITY, INVALID_INT_VAL);
    if (config->maxCapcity == INVALID_INT_VAL) {
        HIVIEW_LOGW("exportDirMaxCapacity configured is invalid.");
        return false;
    }
    // read maximum size of the export single event file
    config->maxSize = CJsonUtil::GetIntValue(jsonRoot_, EXPORT_SINGLE_FILE_MAX_SIZE, INVALID_INT_VAL);
    if (config->maxSize == INVALID_INT_VAL) {
        HIVIEW_LOGW("exportSingleFileMaxSize configured is invalid.");
        return false;
    }
    // read task executing cycle
    config->taskCycle = CJsonUtil::GetIntValue(jsonRoot_, TASK_EXECUTING_CYCLE, INVALID_DOUBLE_VAL);
    if (config->taskCycle == INVALID_INT_VAL) {
        HIVIEW_LOGW("taskExecutingCycle configured is invalid.");
        return false;
    }
    // read day count for event file to store
    config->dayCnt = CJsonUtil::GetIntValue(jsonRoot_, FILE_STORED_MAX_DAY_CNT, INVALID_DOUBLE_VAL);
    if (config->dayCnt == INVALID_INT_VAL) {
        HIVIEW_LOGW("fileStoredMaxDayCnt configured is invalid.");
        return false;
    }
    return true;
}
} // HiviewDFX
} // OHOS