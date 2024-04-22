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
#include "plugin_factory.h"
#include "app_event_publisher_factory.h"
#include "XperfPlugin.h"
#include "EvtParser.h"
#include "JlogId.h"
#include "EventObserverConverter.h"
#include "NormalContext.h"
#include "hiview_logger.h"

const std::string EvtParser::separator = ":";

const std::map<std::string, unsigned int> EvtParser::logIdMap = {
    {"GRAPHIC:JANK_FRAME_SKIP",                         JLID_JANK_FRAME_SKIP},
    {"AAFWK:START_ABILITY",                             JLID_START_ABILITY},
    {"AAFWK:ABILITY_ONFOREGROUND",                      JLID_ABILITY_ONFOREGROUND},
    {"AAFWK:APP_FOREGROUND",                            JLID_APP_FOREGROUND},
    {"AAFWK:ABILITY_ONACTIVE",                          JLID_ABILITY_ONACTIVE},
    {"KERNEL_WAKEUP:LID_WAKEUP_END",                    JLID_LID_WAKEUP_END},
    {"LCD:LCD_POWER_OFF",                               JLID_LCD_POWER_OFF},
    {"AAFWK:TERMINATE_ABILITY",                         JLID_AAFWK_TERMINATE_ABILITY},
    {"AAFWK:APP_BACKGROUND",                            JLID_APP_BACKGROUND},
    {"AAFWK:ABILITY_ONBACKGROUND",                      JLID_ABILITY_ONBACKGROUND},
    {"AAFWK:APP_TERMINATE",                             JLID_APP_TERMINATE},
    {"AAFWK:APP_ATTACH",                                JLID_APP_ATTACH},
    {"GRAPHIC:RS_COMPOSITION_TIMEOUT",                  JLID_RS_COMPOSITION_TIMEOUT},
    {"KERNEL_WAKEUP:LID_WAKEUP_START",                  JLID_LID_WAKEUP_START},
    {"SCREENLOCK_APP:SCREENON_EVENT",                   JLID_SCREENON_EVENT},
    {"LCD:LCD_POWER_ON",                                JLID_LCD_POWER_ON},
    {"LCD:LCD_BACKLIGHT_ON",                            JLID_LCD_BACKLIGHT_ON},
    {"MULTIMODALINPUT:INPUT_POWER_DOWN",                JLID_INPUT_POWER_DOWN},
    {"LCD:LCD_BACKLIGHT_OFF",                           JLID_LCD_BACKLIGHT_OFF},
    {"POWER:STATE",                                     JLID_POWER_STATE},
    {"GRAPHIC:INTERACTION_RESPONSE_LATENCY",            JLID_GRAPHIC_INTERACTION_RESPONSE_LATENCY},
    {"GRAPHIC:INTERACTION_COMPLETED_LATENCY",           JLID_GRAPHIC_INTERACTION_COMPLETED_LATENCY},
    {"AAFWK:DRAWN_COMPLETED",                           JLID_AAFWK_DRAWN_COMPLETED},
    {"INIT:STARTUP_TIME",                               JLID_INIT_STARTUP_TIME},
    {"AAFWK:APP_STARTUP_TYPE",                          JLID_AAFWK_APP_STARTUP_TYPE},
    {"AAFWK:PROCESS_START",                             JLID_AAFWK_PROCESS_START},
    {"WINDOWMANAGER:START_WINDOW",                      JLID_WINDOWMANAGER_START_WINDOW},
    {"GRAPHIC:FIRST_FRAME_DRAWN",                       JLID_GRAPHIC_FIRST_FRAME_DRAWN},
    {"ACE:INTERACTION_COMPLETED_LATENCY",               JLID_ACE_INTERACTION_COMPLETED_LATENCY},
    {"ACE:INTERACTION_APP_JANK",                        JLID_ACE_INTERACTION_APP_JANK},
    {"GRAPHIC:INTERACTION_RENDER_JANK",                 JLID_GRAPHIC_INTERACTION_RENDER_JANK},
    {"AAFWK:CLOSE_ABILITY",                             JLID_AAFWK_CLOSE_ABILITY},
    {"AAFWK:PROCESS_EXIT",                              JLID_AAFWK_PROCESS_EXIT},
    {"SCENE_BOARD_APP:SCENE_PANEL_ROTATION_END",        JLID_SCENE_BOARD_APP_CONTAINER_ANIMATION_END},
    {"SCENE_BOARD_APP:SCREENUNLOCK_EVENT",              JLID_BOARD_SCREENUNLOCK_EVENT},
    {"SCENE_BOARD_APP:CLICK_BUTTON_EVENT",              JLID_BOARD_CLICK_BUTTON_EVENT},
    {"SCENE_BOARD_APP:START_UNLOCK",                    JLID_BOARD_START_UNLOCK},
    {"SCENE_BOARD_APP:UNLOCK_TO_GRID_ANIMATION_END",    JLID_BOARD_UNLOCK_TO_GRID_ANIMATION_END},
    {"SCENE_BOARD_APP:UNLOCK_TO_DOCK_ANIMATION_END",    JLID_BOARD_UNLOCK_TO_DOCK_ANIMATION_END},
    {"SCREEN_RECORDER:RECORDER_STOP",                   JLID_SCREEN_RECORDER_STOP},
    {"ACE:JANK_FRAME_APP",                              JLID_JANK_FRAME_APP},
    {"WINDOWMANAGER:FOCUS_WINDOW",                      JLID_WINDOWMANAGER_FOCUS_WINDOW},
    {"RSS:APP_ASSOCIATED_START",                        JLID_APP_ASSOCIATED_START},
    {"SCENE_BOARD_APP:SCREEN_ON_ANIMATION",             JLID_SCENE_BOARD_APP_SCREEN_ON_ANIMATION},
    {"XGATE:XGATE_WIFI_CONNECT_START",                  JLID_XGATE_WIFI_CONNECT_START},
    {"XGATE:XGATE_WIFI_CONNECT_END",                    JLID_XGATE_WIFI_CONNECT_END},
    {"XGATE:XGATE_SPES_LOGIN_START",                    JLID_XGATE_SPES_LOGIN_START},
    {"XGATE:XGATE_SPES_LOGIN_END",                      JLID_XGATE_SPES_LOGIN_END},
    {"XGATE:XGATE_IACCESS_LOGIN_START",                 JLID_XGATE_IACCESS_LOGIN_START},
    {"XGATE:XGATE_IACCESS_LOGIN_END",                   JLID_XGATE_IACCESS_LOGIN_END},
    {"WINDOWMANAGER:FOLD_STATE_CHANGE_BEGIN",           JLID_FOLD_STATE_CHANGE_BEGIN},
    {"PERFORMANCE:PERF_FACTORY_TEST_START",             JLID_PERF_FACTORY_TEST_START},
    {"PERFORMANCE:PERF_FACTORY_TEST_STOP",              JLID_PERF_FACTORY_TEST_STOP},
    {"PERFORMANCE:PERF_FACTORY_TEST_CLEAR",             JLID_PERF_FACTORY_TEST_CLEAR},
    {"RSS:LIMIT_BOOST",                                 JLID_LIMIT_BOOST},
    {"RSS:LIMIT_REQUEST",                               JLID_LIMIT_FREQUENCY},
    {"GRAPHIC:HGM_VOTER_INFO",                          JLID_LTPO_DYNAMICS_FRAME},
    {"GRAPHIC:SHADER_MALFUNCTION",                      JLID_SHADER_MALFUNCTION},
    {"GRAPHIC:SHADER_STATS",                            JLID_SHADER_STATS},
    {"WEBVIEW:PAGE_LOAD_TIME",                          JLID_WEBVIEW_PAGE_LOAD},
    {"WEBVIEW:DYNAMIC_FRAME_DROP_STATISTICS",           JLID_WEBVIEW_DYNAMIC_FRAME_DROP},
    {"WEBVIEW:AUDIO_FRAME_DROP_STATISTICS",             JLID_WEBVIEW_AUDIO_FRAME_DROP},
    {"WEBVIEW:VIDEO_FRAME_DROP_STATISTICS",             JLID_WEBVIEW_VIDEO_FRAME_DROP},
    {"GRAPHIC:INTERACTION_HITCH_TIME_RATIO",            JLID_GRAPHIC_INTERACTION_HITCH_TIME_RATIO}
};

namespace OHOS {
    namespace HiviewDFX {

        REGISTER(XperfPlugin);
        REGISTER_PUBLISHER(XperfPlugin);
        DEFINE_LOG_TAG("XPerformance");

        void XperfPlugin::OnLoad()
        {
            HIVIEW_LOGI("XperfPlugin::OnLoad");
            SetName(pluginName);
            SetVersion(pluginVersion);
            NormalInit();
            HIVIEW_LOGI("Xperf Plugin Load Finish");
        }

        void XperfPlugin::OnUnload()
        {
            HIVIEW_LOGI("Xperf Plugin OnUnload.");
        }

        void XperfPlugin::OnEventListeningCallback(const Event &event)
        {
            HIVIEW_LOGD("Xperf Listening Event is %{public}s", (event.eventName_).c_str());
            if (event.messageType_ != Event::MessageType::SYS_EVENT) {
                HIVIEW_LOGW("Event is not Sys event type");
                return;
            }
            Event& eventRef = const_cast<Event&>(event);
            SysEvent& sysEvent = static_cast<SysEvent&>(eventRef);
            XperfDispatch(sysEvent);
        }

        void XperfPlugin::AddAppEventHandler(std::shared_ptr<AppEventHandler> handler)
        {
            this->appEventHandler = handler;
        }

        void XperfPlugin::NormalInit()
        {
            /* create context */
            perfContext = std::make_shared<NormalContext>();
            /* create monitors here */
            perfContext->CreateContext();
            /* register Observer */
            IEventObservable* eventObservable = perfContext->GetEventObservable();
            if (eventObservable != nullptr) {
                eventObservable->RegObserver(this);
            } else {
                HIVIEW_LOGW("[XperfPlugin::OnLoad] eventObservable is null");
            }
        }

        void XperfPlugin::XperfDispatch(const SysEvent& sysEvent)
        {
            try {
                std::shared_ptr<XperfEvt> evt = EvtParser::FromHivewEvt(sysEvent);
                DispatchToMonitor(evt->logId, evt);
            }
            catch (std::out_of_range& outEx) {
                std::string logIdStr = std::string(sysEvent.domain_ + EvtParser::separator + sysEvent.eventName_);
                HIVIEW_LOGW("event %{public}s not exist in logIdMap", logIdStr.c_str());
            }
            catch (std::invalid_argument& logIdEx) {
                HIVIEW_LOGW("dispatch exception: %{public}s", logIdEx.what());
            }
        }

        void XperfPlugin::DispatchToMonitor(unsigned int logId, std::shared_ptr<XperfEvt> xperfEvt)
        {
            std::vector<IMonitor*> monitorVec;
            try {
                monitorVec = perfContext->GetMonitorsByLogID(logId);
            } catch (const std::invalid_argument& ex) {
                HIVIEW_LOGW("[XperfPlugin::DispatchToMonitor] no monitor register for %{public}d", logId);
                return;
            }
            for (IMonitor* monitor : monitorVec) {
                monitor->HandleEvt(xperfEvt);
            }
        }

        void XperfPlugin::PostAppStartEvent(const AppStartInfo& ase)
        {
            if (appEventHandler != nullptr) {
                AppEventHandler::AppLaunchInfo appLaunchInfo = EventObserverConverter::ConvertToAppStartInfo(ase);
                HIVIEW_LOGD("[XperfPlugin::PostAppStartEvent] PostEvent begin");
                appEventHandler->PostEvent(appLaunchInfo);
                HIVIEW_LOGD("[XperfPlugin::PostAppStartEvent] PostEvent end");
            } else {
                HIVIEW_LOGW("[XperfPlugin::PostAppStartEvent] appEventHandler is null");
            }
        }

        void XperfPlugin::PostScrollJankEvent(const ScrollJankInfo& sji)
        {
            if (appEventHandler != nullptr) {
                ScrollJankEvtInfo scrollJankEvtInfo = EventObserverConverter::ConvertToScrollJankEvtInfo(sji);
                HIVIEW_LOGD("[XperfPlugin::PostScrollJankEvent] PostEvent begin");
                appEventHandler->PostEvent(scrollJankEvtInfo);
                HIVIEW_LOGD("[XperfPlugin::PostScrollJankEvent] PostEvent end");
            } else {
                HIVIEW_LOGW("[XperfPlugin::PostScrollJankEvent] appEventHandler is null");
            }
        }

    } // HiviewDFX
} // OHOS