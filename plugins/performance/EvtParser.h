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
#ifndef EVT_PARSER_H
#define EVT_PARSER_H

#include "XperfEvt.h"
#include "plugin.h"
#include "sys_event.h"

namespace OHOS {
namespace HiviewDFX {
const std::string KEY_PNAME = "PROCESS_NAME";
const std::string KEY_MESSAGE = "MSG";
const std::string KEY_PID = "PID";
const std::string KEY_UID = "UID";
const std::string KEY_APP_PID = "APP_PID";
const std::string KEY_APP_VERSION = "VERSION_NAME";
const std::string KEY_ABILITY_NAME = "ABILITY_NAME";
const std::string KEY_BUNDLE_NAME = "BUNDLE_NAME";
const std::string KEY_ABILITY_TYPE = "ABILITY_TYPE";

const std::string START_ABILITY = "START_ABILITY";
const std::string APP_ATTACH = "APP_ATTACH";
const std::string ABILITY_ONFOREGROUND = "ABILITY_ONFOREGROUND";
const std::string APP_FOREGROUND = "APP_FOREGROUND";
const std::string APP_BACKGROUND = "APP_BACKGROUND";
const std::string APP_TERMINATE = "APP_TERMINATE";
const std::string JANK_FRAME_SKIP = "JANK_FRAME_SKIP";
const std::string ABILITY_ONBACKGROUND = "ABILITY_ONBACKGROUND";
const std::string RS_COMPOSITION_TIMEOUT = "RS_COMPOSITION_TIMEOUT";
const std::string ABILITY_ONACTIVE = "ABILITY_ONACTIVE";
const std::string KERNEL_AWAKE = "KERNEL_AWAKE";
const std::string SCREENON_EVENT = "SCREENON_EVENT";
const std::string LCD_POWER_ON = "LCD_POWER_ON";
const std::string LCD_BACKLIGHT_ON = "LCD_BACKLIGHT_ON";
const std::string KEY_STARTUP_TIME = "STARTUP_TIME";

const std::string KEY_VERSION_CODE = "VERSION_CODE";
const std::string KEY_SOURCE_TYPE = "SOURCE_TYPE";
const std::string PAGE_URL = "PAGE_URL";
const std::string SCENE_ID = "SCENE_ID";
const std::string INPUT_TIME = "INPUT_TIME";
const std::string ANIMATION_START_TIME = "ANIMATION_START_TIME";
const std::string RENDER_TIME = "RENDER_TIME";
const std::string RESPONSE_LATENCY = "RESPONSE_LATENCY";
const std::string TIMER = "time_";
const std::string ANIMATION_END_LATENCY = "ANIMATION_END_LATENCY";
const std::string E2E_LATENCY = "E2E_LATENCY";

// for boot
const std::string REASON = "REASON";
const std::string ISFIRST = "ISFIRST";
const std::string DETAILED_TIME = "DETAILED_TIME";
const std::string TOTAL_TIME = "TOTAL_TIME";

//for app startup
const std::string KEY_BUNDLE_TYPE = "BUNDLE_TYPE";
const std::string KEY_MODULE_NAME = "MODULE_NAME";
const std::string KEY_STARTUP_ABILITY_TYPE = "STARTUP_ABILITY_TYPE";
const std::string KEY_STARTUP_EXTENSION_TYPE = "STARTUP_EXTENSION_TYPE";
const std::string KEY_CALLER_BUNDLE_NAME = "CALLER_BUNDLE_NAME";
const std::string KEY_CALLER_UID = "CALLER_UID";
const std::string KEY_CALLER_PROCESS_NAME = "CALLER_PROCESS_NAME";
const std::string KEY_WINDOW_NAME = "WINDOW_NAME";
const std::string KEY_ANIMATION_START_LATENCY = "ANIMATION_START_LATENCY";
const std::string KEY_E2E_LATENCY = "ANIMATION_E2E_LATENCY";
const std::string KEY_START_TYPE = "START_TYPE";
const std::string KEY_APP_UID = "APP_UID";

// for app associated start
const std::string KEY_CALLEE_PROCESS_NAME = "CALLEE_PROCESS_NAME";
const std::string KEY_CALLEE_BUNDLE_NAME = "CALLEE_BUNDLE_NAME";

// animator jank frame
const std::string UNIQUE_ID = "UNIQUE_ID";
const std::string STARTTIME = "STARTTIME";
const std::string DURITION = "DURITION";
const std::string TOTAL_FRAMES = "TOTAL_FRAMES";
const std::string TOTAL_MISSED_FRAMES = "TOTAL_MISSED_FRAMES";
const std::string MAX_SEQ_MISSED_FRAMES = "MAX_SEQ_MISSED_FRAMES";
const std::string MAX_FRAMETIME = "MAX_FRAMETIME";
const std::string AVERAGE_FRAMETIME = "AVERAGE_FRAMETIME";
const std::string IS_FOLD_DISP = "IS_FOLD_DISP";
const std::string IS_DISPLAY_ANIMATOR = "DISPLAY_ANIMATOR";
// for screen switch
const std::string KEY_SCREEN_ID = "SCREEN_ID";
const std::string KEY_SCENE_PANEL_NAME = "SCENE_PANEL_NAME";
const std::string KEY_ROTATION_START_TIME = "ROTATION_START_TIME";
const std::string KEY_ROTATION_DURATION = "ROTATION_DURATION";
//for app exit
const std::string KEY_EXIT_TIME = "EXIT_TIME";
const std::string KEY_EXIT_RESULT = "EXIT_RESULT";
const std::string KEY_EXIT_PID = "EXIT_PID";
const std::string KEY_NOTE = "NOTE";

// for power on
const std::string POWER_STATE = "STATE";

// for screen unlock
const std::string KEY_PACKAGE_NAME = "PACKAGE_NAME";
const std::string KEY_VERIFYSUCESS = "VERIFYSUCESS";
const std::string KEY_VERIFY_COSTTIME = "VERIFY_COSTTIME";

// for for PC quick wake
const std::string KEY_KERNEL_START_TIME = "KERNEL_START_TIME";
const std::string KEY_KERNEL_END_TIME = "KERNEL_END_TIME";
const std::string KEY_ANIM_BEGIN_TIME = "BEGIN_TIME";
const std::string KEY_ANIM_END_TIME = "END_TIME";

// for general skip frame
const std::string KEY_SKIPPED_FRAME_TIME = "SKIPPED_FRAME_TIME";

// for DH jank frame
const std::string DH_JANK_FRAME_HAPPEN_TIME = "HAPPEN_TIME";
const std::string DH_JANK_FRAME_TOTAL_TIME_SPENT = "TOTAL_TIME_SPENT";
const std::string DH_JANK_FRAME_PACKAGE_NAME = "PACKAGE_NAME";
const std::string DH_JANK_FRAME_VERSION = "VERSION";

// for DH compose jank frame
const std::string DH_COMPOSE_JANK_FRAME_START_TIME = "START_TIME";
const std::string DH_COMPOSE_JANK_FRAME_END_TIME = "END_TIME";
const std::string DH_COMPOSE_JANK_FRAME_SKIP_FRAME = "SKIP_FRAME";
const std::string DH_COMPOSE_JANK_FRAME_VSYNC_INTERVAL = "VSYNC_INTERVAL";

// for DH app start
const std::string DH_APP_START_MISSION_ID = "MISSION_ID";
const std::string DH_APP_START_TASK_ID = "TASK_ID";
const std::string DH_APP_START_TIMESTAMP = "TIMESTAMP";
// for screen Fold open and close
const std::string KEY_POWER_OFF_SCREEN = "POWER_OFF_SCREEN";
const std::string KEY_POWER_ON_SCREEN = "POWER_ON_SCREEN";

const std::string KEY_HAPPEN_TIME = "HAPPEN_TIME";

// for perf factory test mode
const std::string PERF_FACTORY_TEST_TITLE = "TEST_TITLE";

// for frequency limit
const std::string KEY_CLIENT_ID = "CLIENT_ID";
const std::string KEY_RES_ID = "RES_ID";
const std::string KEY_CONFIG = "CONFIG";
// for boost limit
const std::string KEY_ON_OFF_TAG = "ON_OFF_TAG";
// for shader malfunction
const std::string KEY_SHADER_MALFUNCTION_PID = "PID";
const std::string KEY_SHADER_MALFUNCTION_PROCESS_NAME = "PROCESS_NAME";
const std::string KEY_SHADER_MALFUNCTION_HASH_CODE = "HASH_CODE";
const std::string KEY_SHADER_MALFUNCTION_COMPILE_TIME = "COMPILE_TIME";
const std::string KEY_SHADER_MALFUNCTION_TIME_STAMP = "TIME_STAMP";
const std::string KEY_SHADER_MALFUNCTION_PART_FLAG = "PART_FLAG";
// for shader stats
const std::string KEY_SHADER_STATS_PID = "PID";
const std::string KEY_SHADER_STATS_PROCESS_NAME = "PROCESS_NAME";
const std::string KEY_SHADER_STATS_TIME_STAMP = "TIME_STAMP";
const std::string KEY_SHADER_STATS_PART_FLAG = "PART_FLAG";
// for webview page load
const std::string NAVIGATION_ID = "NAVIGATION_ID";
const std::string NAVIGATION_START = "NAVIGATION_START";
const std::string REDIRECT_COUNT = "REDIRECT_COUNT";
const std::string REDIRECT_START = "REDIRECT_START";
const std::string REDIRECT_END = "REDIRECT_END";
const std::string FETCH_START = "FETCH_START";
const std::string WORKER_START = "WORKER_START";
const std::string DOMAIN_LOOKUP_START = "DOMAIN_LOOKUP_START";
const std::string DOMAIN_LOOKUP_END = "DOMAIN_LOOKUP_END";
const std::string CONNECT_START = "CONNECT_START";
const std::string SECURE_CONNECT_START = "SECURE_CONNECT_START";
const std::string CONNECT_END = "CONNECT_END";
const std::string REQUEST_START = "REQUEST_START";
const std::string RESPONSE_START = "RESPONSE_START";
const std::string RESPONSE_END = "RESPONSE_END";
const std::string DOM_INTERACTIVE = "DOM_INTERACTIVE";
const std::string DOM_CONTENT_LOADED_EVENT_START = "DOM_CONTENT_LOADED_EVENT_START";
const std::string DOM_CONTENT_LOADED_EVENT_END = "DOM_CONTENT_LOADED_EVENT_END";
const std::string LOAD_EVENT_START = "LOAD_EVENT_START";
const std::string LOAD_EVENT_END = "LOAD_EVENT_END";
const std::string FIRST_PAINT = "FIRST_PAINT";
const std::string FIRST_CONTENTFUL_PAINT = "FIRST_CONTENTFUL_PAINT";
const std::string LARGEST_CONTENTFUL_PAINT = "LARGEST_CONTENTFUL_PAINT";
// for webview dynamic frame drop
const std::string DURATION = "DURATION";
const std::string TOTAL_APP_FRAMES = "TOTAL_APP_FRAMES";
const std::string TOTAL_APP_MISSED_FRAMES = "TOTAL_APP_MISSED_FRAMES";
const std::string MAX_APP_FRAMETIME = "MAX_APP_FRAMETIME";
// for webview audio frame drop
const std::string AUDIO_BLANK_FRAME_COUNT = "AUDIO_BLANK_FRAME_COUNT";
// for webview video frame drop
const std::string VIDEO_FRAME_DROPPED_COUNT = "VIDEO_FRAME_DROPPED_COUNT";
const std::string VIDEO_FRAME_DROPPED_DURATION = "VIDEO_FRAME_DROPPED_DURATION";
// for hitch time ratio
const std::string UI_START_TIME = "UI_START_TIME";
const std::string RS_START_TIME = "RS_START_TIME";
const std::string HITCH_TIME = "HITCH_TIME";
const std::string KEY_HITCH_TIME_RATIO = "HITCH_TIME_RATIO";

#define US_TO_S 1000000

class EvtParser {
    using Event = OHOS::HiviewDFX::Event;
    using SysEvent = OHOS::HiviewDFX::SysEvent;

public:
    static const std::string separator;
    static const std::map<std::string, unsigned int> logIdMap;

    static unsigned int ExtractLogIdFromEvt(const SysEvent &sysEvent)
    {
        std::string logIdStr = std::string(sysEvent.domain_ + separator + sysEvent.eventName_);
        unsigned int logId = logIdMap.at(logIdStr); // this might throw std::out_of_range
        return logId;
    }

    static std::shared_ptr <XperfEvt> FromHivewEvt(const SysEvent &e)
    {
        Validate(e);
        XperfEvt evt = ConvertToXperfEvent(e);
        std::shared_ptr <XperfEvt> ret = std::make_shared<XperfEvt>(evt);
        return ret;
    }

private:
    static XperfEvt ConvertToXperfEvent(const SysEvent &event)
    {
        SysEvent &sysEvent = (SysEvent &) event;
        XperfEvt evt;
        evt.logId = ExtractLogIdFromEvt(sysEvent);
        evt.domain = sysEvent.domain_;
        evt.eventName = std::string(sysEvent.eventName_);
        evt.pid = sysEvent.GetPid();
        evt.versionCode = static_cast<int32_t>(sysEvent.GetEventIntValue(KEY_VERSION_CODE));
        evt.appPid = static_cast<int32_t>(sysEvent.GetEventIntValue(KEY_APP_PID));
        evt.sourceType = sysEvent.GetEventValue(KEY_SOURCE_TYPE);
        evt.pageUrl = sysEvent.GetEventValue(PAGE_URL);
        evt.sceneId = sysEvent.GetEventValue(SCENE_ID);
        evt.eventTime = static_cast<uint64_t>(sysEvent.GetEventIntValue(TIMER));
        evt.inputTime = static_cast<uint64_t>(sysEvent.GetEventIntValue(INPUT_TIME));
        evt.renderTime = static_cast<uint64_t>(sysEvent.GetEventIntValue(RENDER_TIME));
        evt.responseLatency = static_cast<uint64_t>(sysEvent.GetEventIntValue(RESPONSE_LATENCY));
        evt.e2eLatency = static_cast<uint64_t>(sysEvent.GetEventIntValue(E2E_LATENCY));
        evt.bundleName = sysEvent.GetEventValue(KEY_BUNDLE_NAME);
        evt.bundleType = static_cast<int32_t>(sysEvent.GetEventIntValue(KEY_BUNDLE_TYPE));
        evt.abilityName = sysEvent.GetEventValue(KEY_ABILITY_NAME);
        evt.processName = sysEvent.GetEventValue(KEY_PNAME);
        evt.moduleName = sysEvent.GetEventValue(KEY_MODULE_NAME);
        evt.versionName = sysEvent.GetEventValue(KEY_APP_VERSION);
        evt.startType = static_cast<int32_t>(sysEvent.GetEventIntValue(KEY_START_TYPE));
        evt.startupTime = static_cast<uint64_t>(sysEvent.GetEventIntValue(KEY_STARTUP_TIME));
        evt.startupAbilityType = static_cast<int32_t>(sysEvent.GetEventIntValue(KEY_STARTUP_ABILITY_TYPE));
        evt.startupExtensionType = static_cast<int32_t>(sysEvent.GetEventIntValue(KEY_STARTUP_EXTENSION_TYPE));
        evt.callerBundleName = sysEvent.GetEventValue(KEY_CALLER_BUNDLE_NAME);
        evt.callerUid = static_cast<int32_t>(sysEvent.GetEventIntValue(KEY_CALLER_UID));
        evt.callerProcessName = sysEvent.GetEventValue(KEY_CALLER_PROCESS_NAME);
        evt.windowName = sysEvent.GetEventValue(KEY_WINDOW_NAME);
        evt.time = evt.eventTime;
        evt.animationStartTime = evt.animStartTime;
        evt.animationStartLatency = static_cast<uint64_t>(sysEvent.GetEventIntValue(KEY_ANIMATION_START_LATENCY));
        evt.animationEndLatency = evt.animationLatency;
        evt.appUid = static_cast<int32_t>(sysEvent.GetEventIntValue(KEY_APP_UID));
        ConvertToXperfAnimatorEvent(evt, sysEvent);
        evt.exitTime = static_cast<uint64_t>(sysEvent.GetEventIntValue(KEY_EXIT_TIME));
        evt.exitResult = static_cast<int32_t>(sysEvent.GetEventIntValue(KEY_EXIT_RESULT));
        evt.exitPid = static_cast<int32_t>(sysEvent.GetEventIntValue(KEY_EXIT_PID));
        evt.note = sysEvent.GetEventValue(KEY_NOTE);
        return evt;
    }

    static void ConvertToXperfAnimatorEvent(XperfEvt &evt, SysEvent &sysEvent)
    {
        evt.animatorInfo.basicInfo.uniqueId = static_cast<int32_t>(sysEvent.GetEventIntValue(UNIQUE_ID));
        evt.animatorInfo.basicInfo.sceneId = sysEvent.GetEventValue(SCENE_ID);
        evt.animatorInfo.basicInfo.moduleName = sysEvent.GetEventValue(KEY_MODULE_NAME);
        evt.animatorInfo.basicInfo.processName = sysEvent.GetEventValue(KEY_PNAME);
        evt.animatorInfo.basicInfo.abilityName = sysEvent.GetEventValue(KEY_ABILITY_NAME);
        evt.animatorInfo.basicInfo.pageUrl = sysEvent.GetEventValue(PAGE_URL);
        evt.animatorInfo.basicInfo.sourceType = sysEvent.GetEventValue(KEY_SOURCE_TYPE);
        evt.animatorInfo.basicInfo.versionCode = static_cast<int32_t>(sysEvent.GetEventIntValue(KEY_VERSION_CODE));
        evt.animatorInfo.basicInfo.versionName = sysEvent.GetEventValue(KEY_APP_VERSION);
        evt.animatorInfo.basicInfo.bundleNameEx = sysEvent.GetEventValue(KEY_NOTE);
        evt.animatorInfo.commonInfo.appPid = sysEvent.GetPid();
        evt.animatorInfo.commonInfo.durition = static_cast<uint64_t>(sysEvent.GetEventIntValue(DURITION));
        evt.animatorInfo.commonInfo.startTime = static_cast<uint64_t>(sysEvent.GetEventIntValue(STARTTIME));
        evt.animatorInfo.commonInfo.totalFrames = static_cast<int32_t>(sysEvent.GetEventIntValue(TOTAL_FRAMES));
        evt.animatorInfo.commonInfo.totalMissedFrames
            = static_cast<int32_t>(sysEvent.GetEventIntValue(TOTAL_MISSED_FRAMES));
        evt.animatorInfo.commonInfo.maxSeqMissedFrames
            = static_cast<int32_t>(sysEvent.GetEventIntValue(MAX_SEQ_MISSED_FRAMES));
        evt.animatorInfo.commonInfo.maxFrameTime = static_cast<uint64_t>(sysEvent.GetEventIntValue(MAX_FRAMETIME));
        evt.animatorInfo.commonInfo.averageFrameTime
            = static_cast<int32_t>(sysEvent.GetEventIntValue(AVERAGE_FRAMETIME));
        evt.animatorInfo.commonInfo.isFoldDisp = static_cast<bool>(sysEvent.GetEventIntValue(IS_FOLD_DISP));
        evt.animatorInfo.commonInfo.isDisplayAnimator
            = static_cast<bool>(sysEvent.GetEventIntValue(IS_DISPLAY_ANIMATOR));
        evt.animatorInfo.commonInfo.happenTime = static_cast<uint64_t>(sysEvent.GetEventIntValue(TIMER));
    }

    static void Validate(const SysEvent &event)
    {
        SysEvent &sysEvent = (SysEvent &) event;
        if (sysEvent.eventName_ == JANK_FRAME_SKIP) {
            std::string appName = sysEvent.GetEventValue(KEY_ABILITY_NAME);
            if (appName == "") {
                throw std::invalid_argument("invalid sysEvent JANK_FRAME_SKIP");
            }
        }
        if (sysEvent.eventName_ == ABILITY_ONACTIVE) {
            int32_t type = static_cast<int32_t>(sysEvent.GetEventIntValue(KEY_ABILITY_TYPE));
            if (type != 1) {
                throw std::invalid_argument("invalid sysEvent ABILITY_ONACTIVE");
            }
        }
        if (sysEvent.eventName_ == KEY_STARTUP_TIME) {
            std::string detailedTime = sysEvent.GetEventValue(DETAILED_TIME);
            if (detailedTime == "") {
                throw std::invalid_argument("invalid sysEvent STARTUP_TIME");
            }
        }
        // what follows cannot be understood
    }
};
} // HiviewDFX
} // OHOS
#endif
