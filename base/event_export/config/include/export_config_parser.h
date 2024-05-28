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

#ifndef HIVIEW_BASE_EVENT_EXPORT_CONFIG_PARSER_H
#define HIVIEW_BASE_EVENT_EXPORT_CONFIG_PARSER_H

#include <memory>
#include <string>
#include <vector>

#include "cJSON.h"
#include "export_event_list_parser.h"

namespace OHOS {
namespace HiviewDFX {
struct SettingDbParam {
    // name of the congifured setting db parameter
    std::string name;

    // the value set when the parameter is enabled
    std::string enabledVal;

    // the value set when the parameter is disabled
    std::string disabledVal;

    // Check vadility of this parameter
    bool IsInValid();
};

struct ExportConfig {
    // name of the module which this config belong to
    std::string moduleName;

    // setting db parameters associated with event export ability
    std::vector<SettingDbParam> settingDbParams;

    // the directory to store exported event file
    std::string exportDir;

    // the maximum capacity of the export directory, unit: Mb
    int64_t maxCapcity = 0;

    // the maximum size of the export event file, unit: Mb
    int64_t maxSize = 0;

    // the executing cycle for exporting&expiring task, unit: hour
    int64_t taskCycle = 0;

    // the list consists of events configured to export
    ExportEventList eventList;

    // the maximum count of day for event files to store. unit: day
    int64_t dayCnt = 0;

    // Get paramter configured which is accociated with event export
    SettingDbParam GetExportAbilityParam();

    // Get paramter configured which is accociated with system upgrade
    SettingDbParam GetUpgradeAbilityParam();
};

class ExportConfigParser {
public:
    ExportConfigParser(const std::string& configFile);
    virtual ~ExportConfigParser();

public:
    std::shared_ptr<ExportConfig> Parse();

private:
    bool ParseExportEventList(ExportEventList& list);
    bool ParseSettingDbParamList(std::vector<SettingDbParam>& settingDbParams);
    bool ParseResidualContent(std::shared_ptr<ExportConfig> config);

private:
    cJSON* jsonRoot_ = nullptr;
};
} // HiviewDFX
} // OHOS

#endif // HIVIEW_BASE_EVENT_EXPORT_CONFIG_PARSER_H
