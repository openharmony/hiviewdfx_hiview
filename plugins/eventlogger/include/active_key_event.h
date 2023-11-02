/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef HIVIEW_EVENT_LOG_ACTIVE_KEY_EVENT_H
#define HIVIEW_EVENT_LOG_ACTIVE_KEY_EVENT_H

#include <memory>
#include <string>

#include "event_thread_pool.h"
#include "log_store_ex.h"

#include "input_manager.h"
namespace OHOS {
namespace HiviewDFX {
class ActiveKeyEvent {
public:
    ActiveKeyEvent();
    ~ActiveKeyEvent();
    ActiveKeyEvent& operator=(const ActiveKeyEvent&) = delete;
    ActiveKeyEvent(const ActiveKeyEvent&) = delete;

    void init(std::shared_ptr<EventThreadPool> eventPool, std::shared_ptr<LogStoreEx> logStore);
    static int64_t SystemTimeMillisecond();

private:
    void CombinationKeyHandle();
    void CombinationKeyCallback(std::shared_ptr<MMI::KeyEvent> keyEvent);
    void HitraceCapture();
    void SysMemCapture(int fd);
    void DumpCapture(int fd);
    void GeneratingLogs();

    int32_t subscribeId_;
    std::shared_ptr<EventThreadPool> eventPool_;
    std::shared_ptr<LogStoreEx> logStore_;
    uint64_t triggeringTime_;

    static const inline std::string CMD_LIST[] = {
        "cmd:w",
        "cmd:a",
        "cmd:p",
        "cmd:d",
        "cmd:rs",
        "cmd:c",
        "cmd:ss",
    };
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_EVENT_LOG_ACTIVE_KEY_EVENT_H