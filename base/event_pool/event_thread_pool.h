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
#ifndef HIVIEW_EVENT_LOGGER_EVENT_THREAD_POOL_H
#define HIVIEW_EVENT_LOGGER_EVENT_THREAD_POOL_H
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "event_priority_queue.h"
#include "nocopyable.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
class TaskEvent {
public:
    TaskEvent(uint8_t priority, uint64_t targetTime, Task task, const std::string &name)
        : priority_(priority), targetTime_(targetTime), task_(task), name_(name)
    {
        seq = TimeUtil::GetNanoTime();
    }

    ~TaskEvent() {}
    uint8_t priority_;
    uint64_t targetTime_;
    Task task_;
    std::string name_;
    uint64_t seq;

    bool operator<(const TaskEvent &obj) const
    {
        if (this->targetTime_ > obj.targetTime_) {
            return true;
        } else if (this->targetTime_ == obj.targetTime_) {
            if (this->priority_ > obj.priority_) {
                return true;
            }
        }
        return false;
    }
};


class EventThreadPool {
public:
    DISALLOW_COPY_AND_MOVE(EventThreadPool);
    EventThreadPool(int maxCout, const std::string& name);
    ~EventThreadPool();

    enum Priority {
        HIGHEST_PRIORITY = 1,
        HIGH_PRIORITY = 10,
        LOW_PRIORITY = 20,
        LOWEST_PRIORITY = 30,
        IDLE_PRIORITY = 50,
    };

    void Start();
    void Stop();
    void AddTask(Task task, const std::string &name,
        uint64_t delay = 0, uint8_t priority = Priority::IDLE_PRIORITY);

private:
    int maxCout_;
    std::string name_;
    std::atomic<bool> runing_;
    std::vector<std::thread> pool_;
    std::mutex mutex_;
    std::condition_variable cvSync_;
    EventPriorityQueue<TaskEvent> taskQueue_;

    TaskEvent ObtainTask(uint64_t &targetTime);
    void TaskCallback();
};
}  // namespace HiviewDFX
}  // namespace OHOS
#endif  // HIVIEW_EVENT_LOGGER_EVENT_THREAD_POOL_H