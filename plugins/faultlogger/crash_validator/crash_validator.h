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
#ifndef CRASH_VALIDATOR_H
#define CRASH_VALIDATOR_H

#include "plugin.h"
#include "sys_event.h"

#include <memory>
#include <mutex>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
class CrashEvent {
public:
    uint64_t time {0};
    uint64_t pid {0};
    uint64_t uid {0};
    std::string path {""};
    std::string name {""};
    bool isCppCrash {false};
};
// every CPP_CRASH should have corresponding PROCESS_EXIT event and
// every PROCESS_EXIT event that triggered by crash signal should have corresponding CPP_CRASH events
// check the existence of these event to judge whether we have loss some crash log
class CrashValidator : public EventListener, public Plugin {
public:
    CrashValidator();
    ~CrashValidator();

    bool ReadyToLoad() override;
    void OnLoad() override;
    void OnUnload() override;
    bool OnEvent(std::shared_ptr<Event>& event) override;
    void OnUnorderedEvent(const Event &event) override;
    bool CanProcessEvent(std::shared_ptr<Event> event) override;
    std::string GetListenerName() override;
    void Dump(int fd, const std::vector<std::string>& cmds) override;

private:
    bool ValidateLogContent(const CrashEvent& event);
    bool RemoveSimilarEvent(const CrashEvent& event);
    void HandleCppCrashEvent(SysEvent& sysEvent);
    void HandleProcessExitEvent(SysEvent& sysEvent);
    void PrintEvents(int fd, const std::vector<CrashEvent>& events);
    void ReadServiceCrashStatus();
    void CheckOutOfDateEvents();
    bool stopReadKmsg_;
    uint32_t totalEventCount_;
    uint32_t normalEventCount_;
    std::unique_ptr<std::thread> kmsgReaderThread_;
    std::vector<CrashEvent> pendingEvents_;
    std::vector<CrashEvent> noLogEvents_;
    std::vector<CrashEvent> matchedEvents_;
    std::mutex lock_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // CRASH_VALIDATOR_H