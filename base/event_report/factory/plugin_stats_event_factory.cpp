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
#include "plugin_stats_event_factory.h"

#include "hiview_event_common.h"
#include "plugin_stats_event.h"

namespace OHOS {
namespace HiviewDFX {
std::unique_ptr<LoggerEvent> PluginStatsEventFactory::Create()
{
    return std::make_unique<PluginStatsEvent>(PluginStatsEventSpace::EVENT_NAME, HiSysEvent::STATISTIC);
}
} // namespace HiviewDFX
} // namespace OHOS
