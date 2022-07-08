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
#include "event_thread_pool.h"

#include "logger.h"
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
    for (int i = 0; i < maxCout_; ++i) {
        pool_[i].join();
    }
}

void EventThreadPool::AddEvent(runTask task, uint64_t delay, uint8_t priority)
{
    std::unique_lock<std::mutex> lock(mutex_);
    uint64_t targetTime = TimeUtil::GetNanoTime() + delay * TimeUtil::SEC_TO_NANOSEC;
    runTask_.push(TaskEvent(priority, targetTime, task));
    runTask_.ShrinkIfNeedLocked();
    if (runTask_.size() > 1000) { // 1000: 积压超过1000条预警
        HIVIEW_LOGW("%{public}s AddTask. runTask size is %{public}d", name_.c_str(), runTask_.size());
    }
}

void EventThreadPool::TaskCallback()
{
    std::string name = name_ + "@" + std::to_string(Thread::GetTid());
    const int maxLength = 16;
    if (name.length() >= maxLength) {
        HIVIEW_LOGW("%{public}s is too long for thread, please change to a shorter one.", name.c_str());
        name = name.substr(0, maxLength - 1);
    }
    Thread::SetThreadDescription(name);
    while (runing_) {
        runTask task = nullptr;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (runTask_.empty()) {
                cvSync_.wait(lock);
            }
            if (!runing_) {
                break;
            }

            if (runTask_.size() > 0) {
                auto tmp = runTask_.top();
                uint64_t now = TimeUtil::GetNanoTime();
                if (tmp.targetTime_ > now) {
                    cvSync_.wait_until(lock, std::chrono::time_point<>())
                }
                std::chrono::duration<uint64_t, std::ratio<1, tmp.targetTime_>> dt;
                cvSync_.wait_for(lock, dt);
                task = tmp.task_;
                runTask_.pop();
            }
            if (task == nullptr) {
                HIVIEW_LOGW("task == nullptr. %{public}s runTask size is %{public}d", name.c_str(), runTask_.size());
                continue;
            }
        }
        task();
    }
    HIVIEW_LOGI("%{public}s exit.", name.c_str());
}
}  // namespace HiviewDFX
}  // namespace OHOS