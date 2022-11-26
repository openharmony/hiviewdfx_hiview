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
#ifndef BBOX_DETECTOR_PLUGIN_H
#define BBOX_DETECTOR_PLUGIN_H
#include <string>

#include "event_source.h"
#include "plugin.h"
#include "sys_event.h"
namespace OHOS {
namespace HiviewDFX {
class BBoxDetectorPlugin : public Plugin {
public:
    BBoxDetectorPlugin() {};
    ~BBoxDetectorPlugin() {};
    void OnLoad() override;
    void OnUnload() override;
    bool OnEvent(std::shared_ptr<Event> &event) override;

private:
    bool IsInterestedPipelineEvent(std::shared_ptr<Event> event) override;
    void HandleBBoxEvent(std::shared_ptr<SysEvent> &sysEvent);
    void WaitForLogs(const std::string& logDir);

    std::string logParseConfig_ = "/system/etc/hiview";
};
}
}
#endif /* BBOX_DETECTOR_PLUGIN_H */