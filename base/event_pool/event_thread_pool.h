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
#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "nocopyable.h"
namespace OHOS {
namespace HiviewDFX {
using runTask = std::function<void()>;
class EventThreadPool {
public:
    DISALLOW_COPY_AND_MOVE(EventThreadPool);
    EventThreadPool(int maxCout, const std::string& name);
    ~EventThreadPool();
    void Start();
    void Stop();
    void AddTask(runTask task);

private:
    int maxCout_;
    std::string name_;
    std::atomic<bool> runing_;
    std::vector<std::thread> pool_;
    std::mutex mutex_;
    std::condition_variable cvSync_;
    std::list<runTask> runTask_;

    void TaskCallback();
};
}  // namespace HiviewDFX
}  // namespace OHOS
#endif  // HIVIEW_EVENT_LOGGER_EVENT_THREAD_POOL_H