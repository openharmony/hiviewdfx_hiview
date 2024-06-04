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

#include "ThrTaskContainer.h"
#include <sys/prctl.h>
#include <thread>
#include "hiview_logger.h"

DEFINE_LOG_LABEL(0xD002D66, "Hiview-XPerformance");

/* ThrTaskContainer */
void ThrTaskContainer::StartLoop(const std::string& threadName)
{
    std::thread startLoopThread(&ThrTaskContainer::Entry, this, threadName);
    startLoopThread.detach();
}

void ThrTaskContainer::StopLoop()
{
}

void ThrTaskContainer::PostTask(ITask* task)
{
    if (task == nullptr) {
        throw std::invalid_argument("null task");
    }
    std::unique_lock <std::mutex> uniqueLock(mut);
    if (!IsTaskOverLimit()) {
        tasks.push_back(task);
    }
    cv.notify_one();
}

bool ThrTaskContainer::IsTaskOverLimit()
{
    if (tasks.size() < maxTaskSize) {
        return false;
    }
    HIVIEW_LOGI("CheckTaskVectorLimit over limit");
    for (auto& task: tasks) {
        delete task;
        task = nullptr;
    }
    tasks.clear();
    return true;
}

void ThrTaskContainer::Entry(const std::string& threadName)
{
    std::unique_lock <std::mutex> uniqueLock(mut);
    prctl(PR_SET_NAME, threadName.c_str(), nullptr, nullptr, nullptr);
    while (true) {
        while (tasks.empty()) {
            cv.wait(uniqueLock);
            continue;
        }
        ITask* task = tasks.front();
        if (task == nullptr) {
            HIVIEW_LOGE("Entry task is null");
            continue;
        }
        std::string taskInfo = task->GetTaskInfo();
        tasks.erase(tasks.begin());
        uniqueLock.unlock();
        task->Run();
        uniqueLock.lock();
    }
    // delete this;?
}