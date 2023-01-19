/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "event_thread_pool.h"

#include <memory>

#include "logger.h"
#include "memory_util.h"
#include "thread_util.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-EventThreadPool");
EventThreadPool::EventThreadPool(int maxCout, const std::string& name): maxCout_(maxCout), name_(name), runing_(false)
{
    pool_ = std::vector<std::thread>(maxCout);
}

EventThreadPool::~EventThreadPool()
{
    Stop();
}

void EventThreadPool::Start()
{
    if (runing_) {
        return;
    }
    runing_ = true;
    for (int i = 0; i < maxCout_; ++i) {
        pool_[i] = std::thread(std::bind(&EventThreadPool::TaskCallback, this));
    }
}

void EventThreadPool::Stop()
{
    if (!runing_) {
        return;
    }
    runing_ = false;
    cvSync_.notify_all();
    for (int i = 0; i < maxCout_; ++i) {
        pool_[i].join();
    }
}

void EventThreadPool::AddTask(Task task, const std::string &name, uint64_t delay, uint8_t priority)
{
    std::unique_lock<std::mutex> lock(mutex_);
    uint64_t targetTime = TimeUtil::GetMilliseconds() + delay;
    HIVIEW_LOGD("AddEvent: targetTime is %{public}s\n", std::to_string(targetTime).c_str());
    taskQueue_.push(TaskEvent(priority, targetTime, task, name));
    taskQueue_.ShrinkIfNeedLocked();
    cvSync_.notify_all();
    if (taskQueue_.size() > 1000) { // 1000: 积压超过1000条预警
        HIVIEW_LOGW("%{public}s AddTask. runTask size is %{public}d", name_.c_str(), taskQueue_.size());
    }
}

TaskEvent EventThreadPool::ObtainTask(uint64_t &targetTime)
{
    if (taskQueue_.empty()) {
        targetTime = UINT64_MAX;
        return TaskEvent(Priority::IDLE_PRIORITY, targetTime, nullptr, "nullptr");
    }
    auto tmp = taskQueue_.top();
    targetTime = tmp.targetTime_;
    return tmp;
}

void EventThreadPool::TaskCallback()
{
    if (MemoryUtil::DisableThreadCache() != 0 || MemoryUtil::DisableDelayFree() != 0) {
        HIVIEW_LOGW("Failed to optimize memory for current thread");
    }

    std::string tid = std::to_string(Thread::GetTid());
    const int maxLength = 10;
    if (name_.length() > maxLength) {
        HIVIEW_LOGW("%{public}s is too long for thread, please change to a shorter one.", name_.c_str());
        name_ = name_.substr(0, maxLength - 1);
    }
    std::string name = name_ + "@" + tid;
    Thread::SetThreadDescription(name);
    while (runing_) {
        Task task = nullptr;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            uint64_t targetTime = UINT64_MAX;
            TaskEvent taskEvent = ObtainTask(targetTime);
            uint64_t now = TimeUtil::GetMilliseconds();
            HIVIEW_LOGD("name is %{public}s, targetTime is %{public}s, now is %{public}s\n",
                taskEvent.name_.c_str(), std::to_string(targetTime).c_str(), std::to_string(now).c_str());
            if (!runing_) {
                break;
            }
            if (targetTime == UINT64_MAX) {
                cvSync_.wait(lock);
                continue;
            } else if (targetTime > now) {
                cvSync_.wait_for(lock, std::chrono::milliseconds(targetTime - now));
                continue;
            }
            task = taskEvent.task_;
            taskQueue_.pop();
            if (task == nullptr) {
                HIVIEW_LOGW("task == nullptr. %{public}s runTask size is %{public}d", name.c_str(), taskQueue_.size());
                continue;
            }
            Thread::SetThreadDescription(taskEvent.name_);
        }
        task();
        Thread::SetThreadDescription(name);
    }
    HIVIEW_LOGI("%{public}s exit.", name.c_str());
}
}  // namespace HiviewDFX
}  // namespace OHOS