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

#include <sys/prctl.h>
#include "ThrExecutor.h"
#include "hiview_logger.h"

DEFINE_LOG_LABEL(0xD002D66, "Hiview-XPerformance");

const static std::string MAIN_THREAD_NAME = "XperfMainThr";

ThrExecutor::ThrExecutor()
{
    ThrTaskContainer* contMain = new ThrTaskContainer();
    contMain->StartLoop(MAIN_THREAD_NAME.c_str());
    containers.insert(std::pair<int, ThrTaskContainer*>(MAIN_THR, contMain));
}

ThrExecutor::~ThrExecutor()
{
    for (std::map<int, ThrTaskContainer*>::iterator it = containers.begin(); it != containers.end(); it++) {
        ThrTaskContainer* con = it->second;
        if (con != nullptr) {
            con->StopLoop();
        }
    }
}

void ThrExecutor::ValidateNonNull(void* task)
{
    if (task == nullptr) {
        throw std::invalid_argument("null task");
    }
}

class ProcessTimoutWrapperTask : public ITask {
public:
    ProcessTimoutWrapperTask(ITimeoutExecutor::ITimeoutHandler* evtProcessor, std::string name)
    {
        this->evtProcessor = evtProcessor;
        this->name = name;
    }

    void Run()
    {
        if (evtProcessor != nullptr) {
            evtProcessor->HandleTimeoutInMainThr(this->name);
        }
        delete this;
    }

    std::string GetTaskInfo()
    {
        return this->taskName;
    }
private:
    const std::string taskName = "ProcessTimoutWrapperTask";
    ITimeoutExecutor::ITimeoutHandler* evtProcessor;
    std::string name;
};

void ThrExecutor::ExecuteTimeoutInMainThr(ITimeoutHandler* task, std::string name)
{
    ValidateNonNull(task);
    ThrTaskContainer* con = containers[MAIN_THR];
    if (con != nullptr) {
        ITask* evtTask = new ProcessTimoutWrapperTask(task, name);
        con->PostTask(evtTask);
    } else {
        HIVIEW_LOGE("ThrExecutor::ExecuteTimeoutInMainThr main thread task container is null");
    }
}

class ProcessAppEvtWrapperTask : public ITask {
public:
    ProcessAppEvtWrapperTask(IAppThrExecutor::IProcessAppEvtTask* task, const IAppThrExecutor::AppEvtData data)
    {
        this->evtProcessor = task;
        this->data = data;
    }

    void Run()
    {
        if (evtProcessor != nullptr) {
            evtProcessor->ExecuteProcessAppEvtTaskInMainThr(this->data);
        }
        delete this;
    }

    std::string GetTaskInfo()
    {
        return this->taskName + ", " + data.eventName;
    }
private:
    const std::string taskName = "ProcessAppEvtWrapperTask";
    IAppThrExecutor::IProcessAppEvtTask* evtProcessor;
    IAppThrExecutor::AppEvtData data;
};

void ThrExecutor::ExecuteHandleEvtInMainThr(IProcessAppEvtTask* task, const AppEvtData& data)
{
    ValidateNonNull(task);
    ThrTaskContainer* con = containers[MAIN_THR];
    if (con != nullptr) {
        ITask* evtTask = new ProcessAppEvtWrapperTask(task, data);
        con->PostTask(evtTask);
    } else {
        HIVIEW_LOGE("ThrExecutor::ExecuteHandleEvtInMainThr main thread task container is null");
    }
}

class ProcessMonitorEvtWrapperTask : public ITask {
public:
    ProcessMonitorEvtWrapperTask(
            IMonitorThrExecutor::IHandleMonitorEvt* handleTask, const std::shared_ptr <XperfEvt> evt)
    {
        this->handleTask = handleTask;
        this->evt = evt;
    }

    void Run()
    {
        if (handleTask != nullptr) {
            handleTask->HandleMainThrEvt(this->evt);
        }
        delete this;
    }

    std::string GetTaskInfo()
    {
        return this->taskName;
    }
private:
    const std::string taskName = "ProcessMonitorEvtWrapperTask";
    IMonitorThrExecutor::IHandleMonitorEvt* handleTask;
    std::shared_ptr <XperfEvt> evt;
};

void ThrExecutor::ExecuteMonitorInMainThr(IHandleMonitorEvt* task, std::shared_ptr <XperfEvt> evt)
{
    ValidateNonNull(task);
    ThrTaskContainer* con = containers[MAIN_THR];
    if (con != nullptr) {
        ITask* evtTask = new ProcessMonitorEvtWrapperTask(task, evt);
        con->PostTask(evtTask);
    } else {
        HIVIEW_LOGE("ThrExecutor::ExecuteMonitorInMainThr main thread task container is null");
    }
}