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
#ifndef HIVIEW_PLUGIN_EVENT_LOG_COLLECTOR_H
#define HIVIEW_PLUGIN_EVENT_LOG_COLLECTOR_H

#include <ctime>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

#include "event.h"
#include "event_loop.h"
#include "ffrt.h"
#include "log_store_ex.h"
#include "logger.h"
#include "plugin.h"
#include "sys_event.h"

#include "active_key_event.h"
#include "event_logger_config.h"

namespace OHOS {
namespace HiviewDFX {
struct BinderInfo {
    int client;
    int server;
    unsigned long wait;
};

class EventLogger : public EventListener, public Plugin {
public:
    EventLogger() : logStore_(std::make_shared<LogStoreEx>(LOGGER_EVENT_LOG_PATH, true)),
        startTime_(time(nullptr)) {};
    ~EventLogger() {};
    bool OnEvent(std::shared_ptr<Event> &event) override;
    void OnLoad() override;
    void OnUnload() override;
    bool IsInterestedPipelineEvent(std::shared_ptr<Event> event) override;
    std::string GetListenerName() override;
    void OnUnorderedEvent(const Event& msg) override;
private:
    static const inline std::string LOGGER_EVENT_LOG_PATH = "/data/log/eventlog";
    static const inline std::string MONITOR_STACK_LOG_PATH = "/data/log/faultlog/temp";
    static const inline std::string LONG_PRESS = "LONG_PRESS";
    static const inline std::string AP_S_PRESS6S = "AP_S_PRESS6S";
    static const inline std::string REBOOT_REASON = "reboot_reason";
    static const inline std::string NORMAL_RESET_TYPE = "normal_reset_type";
    static const inline std::string PATTERN_WITHOUT_SPACE = "\\s*=\\s*([^ \\n]*)";
    static const inline std::string DOMAIN_LONGPRESS = "KERNEL_VENDOR";
    static const inline std::string STRINGID_LONGPRESS = "COM_LONG_PRESS";
    static const inline std::string MONITOR_STACK_FLIE_NAME[] = {
        "jsstack",
    };
    static const inline std::string MONITOR_LOG_PATH[] = {
        MONITOR_STACK_LOG_PATH,
    };
    static constexpr int EVENT_MAX_ID = 1000000;
    static constexpr int MAX_FILE_NUM = 500;
    static constexpr int MAX_FOLDER_SIZE = 50 * 1024 * 1024;

    std::shared_ptr<LogStoreEx> logStore_;
    uint64_t startTime_;
    std::unordered_map<std::string, std::time_t> eventTagTime_;
    std::unordered_map<int, std::string> fileMap_;
    std::unordered_map<std::string, EventLoggerConfig::EventLoggerConfigData> eventLoggerConfig_;
    std::shared_ptr<EventLoop> threadLoop_ = nullptr;
    std::unordered_set<std::shared_ptr<SysEvent>> sysEventSet_;
    int const maxEventPoolCount = 5;
    ffrt::mutex intervalMutex_;
    std::mutex finishMutex_;
    std::unique_ptr<ActiveKeyEvent> activeKeyEvent_;
    std::string cmdlinePath_ = "/proc/cmdline";
    std::string cmdlineContent_ = "";
    std::vector<std::string> rebootReasons_;

    void StartLogCollect(std::shared_ptr<SysEvent> event);
    int Getfile(std::shared_ptr<SysEvent> event, std::string& logFile);
    bool JudgmentRateLimiting(std::shared_ptr<SysEvent> event);
    bool WriteCommonHead(int fd, int jsonFd, std::shared_ptr<SysEvent> event);
    bool UpdateDB(std::shared_ptr<SysEvent> event, std::string logFile);
    void CreateAndPublishEvent(std::string& dirPath, std::string& fileName);
    bool IsHandleAppfreeze(std::shared_ptr<SysEvent> event);
    void CheckEventOnContinue();
    bool CanProcessRebootEvent(const Event& event);
    void ProcessRebootEvent();
    std::string GetRebootReason() const;
    void GetCmdlineContent();
    void GetRebootReasonConfig();
    bool GetMatchString(const std::string& src, std::string& dst, const std::string& pattern) const;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGIN_EVENT_LOG_COLLECTOR_H
