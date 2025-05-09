/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "event_logger_config.h"

#include <chrono>
#include <regex>

#include "hiview_logger.h"
#include "file_util.h"
namespace OHOS {
namespace HiviewDFX {
namespace {
    constexpr char EVENT_LOGGER_CONFIG_PATH[] = "/system/etc/hiview/event_logger_config";
    constexpr int VERSION_FIELD = 1;
    constexpr int ID_FIELD = 1;
    constexpr int NAME_FIELD = 2;
    constexpr int ACTION_FIELD = 3;
    constexpr int INTERVAL_FIELD = 4;
}

DEFINE_LOG_TAG("EventLogger-EventLoggerConfig");

EventLoggerConfig::EventLoggerConfig()
{
    configPath_ = EVENT_LOGGER_CONFIG_PATH;
}

EventLoggerConfig::EventLoggerConfig(std::string configPath)
{
    configPath_ = configPath;
}

bool EventLoggerConfig::OpenConfig()
{
    std::string realPath;
    if (!FileUtil::PathToRealPath(configPath_, realPath)) {
        HIVIEW_LOGI("fail to realPath.");
        return false;
    }
    in_.open(realPath);
    if (!in_.is_open()) {
        HIVIEW_LOGW("fail to open config file.\n");
        return false;
    }
    return true;
}

void EventLoggerConfig::CloseConfig()
{
    if (in_.is_open()) {
        in_.close();
    }
}

bool EventLoggerConfig::FindConfigVersion()
{
    if (!OpenConfig()) {
        return false;
    }

    std::string buf = "";
    if (!getline(in_, buf)) {
        HIVIEW_LOGW("Configfile is none.\n");
        CloseConfig();
        return false;
    }

    std::smatch result;
    auto versionRegex = std::regex("version=\"([0-9\\.]+)\".*");
    if (!regex_search(buf, result, versionRegex)) {
        HIVIEW_LOGW("match version failed.\n");
        CloseConfig();
        return false;
    }
    version_ = result[VERSION_FIELD];
    HIVIEW_LOGI("version: %{public}s\n", version_.c_str());
    return true;
}

bool EventLoggerConfig::ParseConfigData(std::function<bool(EventLoggerConfigData&)> func)
{
    HIVIEW_LOGI("called\n");
    if (!FindConfigVersion()) {
        return false;
    }

    std::string buf = "";
    std::smatch result;
    auto eventRegex = std::regex(
        "event id=\"([0-9xX]*)\"\\s*name=\"([A-Z0-9_]+)\"\\s*action=\"(.*)\"\\s*interval=\"([0-9]*)\".*");

    while (getline(in_, buf)) {
        if (!regex_search(buf, result, eventRegex)) {
            HIVIEW_LOGW("match event failed, getline duf is %{public}s.\n", buf.c_str());
            continue;
        }

        EventLoggerConfigData tmpConfigDate;
        std::string idString = result[ID_FIELD];
        if (idString.empty()) {
            tmpConfigDate.id = -1;
        } else {
            tmpConfigDate.id = std::stoi(idString, nullptr, 0);
        }
        tmpConfigDate.name = result[NAME_FIELD];
        tmpConfigDate.action = result[ACTION_FIELD];
        std::string intervalString = result[INTERVAL_FIELD];
        if (intervalString.empty()) {
            tmpConfigDate.interval = 0;
        } else {
            tmpConfigDate.interval = std::stoi(intervalString);
        }
        if (!func(tmpConfigDate)) {
            break;
        }
    }
    CloseConfig();
    return true;
}

bool EventLoggerConfig::FindConfigLine(int eventId, std::string eventName, EventLoggerConfigData &configOut)
{
    HIVIEW_LOGI("called\n");
    bool ret = false;
    ParseConfigData([&](EventLoggerConfigData& configDate)->bool {
        if (eventName == configDate.name) {
            ret = true;
        }
        if (eventId == configDate.id) {
            ret = true;
        }
        if (ret) {
            configOut.id = configDate.id;
            configOut.name = configDate.name;
            configOut.interval = configDate.interval;
            configOut.action = configDate.action;
            HIVIEW_LOGI("configDate-> id: 0x%{public}x, name: %{public}s, action: %{public}s, interval: %{public}d\n",
                configOut.id, configOut.name.c_str(), configOut.action.c_str(), configOut.interval);
        return false;
        }
        return true;
    });
    return ret;
}

std::unordered_map<std::string, EventLoggerConfig::EventLoggerConfigData> EventLoggerConfig::GetConfig()
{
    std::unordered_map<std::string, EventLoggerConfigData> ret;
    ParseConfigData([&](EventLoggerConfigData& data)->bool {
        ret.insert({ data.name, data });
        return true;
    });
    return ret;
}
} // namespace HiviewDFX
} // namespace OHOS
