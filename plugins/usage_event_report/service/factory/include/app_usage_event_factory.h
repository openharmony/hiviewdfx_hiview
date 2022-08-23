/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef HIVIEW_PLUGINS_USAGE_EVENT_REPORT_SERVICE_APP_USAGE_EVENT_FACTORY_H
#define HIVIEW_PLUGINS_USAGE_EVENT_REPORT_SERVICE_APP_USAGE_EVENT_FACTORY_H

#include <vector>

#include "logger_event_factory.h"

namespace OHOS {
namespace HiviewDFX {
struct AppUsageInfo {
    AppUsageInfo(const std::string& package, const std::string& version, int64_t usage, const std::string& date)
        : package_(package), version_(version), usage_(usage), date_(date)
    {}

    std::string package_;
    std::string version_;
    uint64_t usage_;
    std::string date_;
};

class AppUsageEventFactory : public LoggerEventFactory {
public:
    std::unique_ptr<LoggerEvent> Create() override;
    void Create(std::vector<std::unique_ptr<LoggerEvent>>& events) override;

private:
    void GetAllCreatedOsAccountIds(std::vector<int32_t>& ids);
    void GetAppUsageInfosByUserId(std::vector<AppUsageInfo>& infos, int32_t userId);
    std::string GetAppVersion(const std::string& bundleName);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_USAGE_EVENT_REPORT_SERVICE_APP_USAGE_EVENT_FACTORY_H
