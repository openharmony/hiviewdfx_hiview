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
#ifndef XPERF_PLUGIN_H
#define XPERF_PLUGIN_H

#include <map>
#include "plugin.h"
#include "IMonitor.h"
#include "sys_event.h"
#include "IXperfContext.h"
#include "app_event_publisher.h"
#include "IEventObserver.h"

namespace OHOS {
    namespace HiviewDFX {
        class XperfPlugin : public AppEventPublisher, public IEventObserver {
        public:
            void OnLoad() override;
            void OnUnload() override;
            void OnEventListeningCallback(const Event& event) override;
            void AddAppEventHandler(std::shared_ptr<AppEventHandler> handler) override;
            void PostAppStartEvent(const AppStartInfo& ase) override;
            void PostScrollJankEvent(const ScrollJankInfo& scrollJankInfo) override;

        private:
            std::shared_ptr<IXperfContext> perfContext{nullptr};
            static constexpr const char* const pluginName = "Xperf";
            static constexpr const char* const pluginVersion = "Xperf 1.0";
            std::shared_ptr<AppEventHandler> appEventHandler{nullptr};

            void XperfDispatch(const SysEvent &sysEvent);
            void DispatchToMonitor(unsigned int logId, std::shared_ptr<XperfEvt> xperfEvt);
            void NormalInit();
        };
    } // HiviewDFX
} // OHOS
#endif