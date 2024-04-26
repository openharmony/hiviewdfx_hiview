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
#ifndef HIVIEW_PLUGINS_UNIFIED_COLLECTOR_DYNTRACE_INCLUDE_TRACE_CONTEXT_H
#define HIVIEW_PLUGINS_UNIFIED_COLLECTOR_DYNTRACE_INCLUDE_TRACE_CONTEXT_H
#include <cinttypes>
#include <memory>

#include "app_caller_event.h"
#include "sys_event.h"
#include "plugin.h"

namespace OHOS {
namespace HiviewDFX {
class AppTraceState;
class AppTraceContext {
public:
    AppTraceContext(std::shared_ptr<AppTraceState> state);
    ~AppTraceContext() = default;

public:
    int32_t TransferTo(std::shared_ptr<AppTraceState> state);
    void PublishStackEvent(SysEvent& sysEvent);
    void Reset();

    friend class StartTraceState;
    friend class DumpTraceState;
    friend class StopTraceState;
private:
    int32_t pid_;
    int64_t traceBegin_;
    bool isOpenTrace_;
    bool isTraceOn_;
    bool isDumpTrace_;
    std::shared_ptr<AppTraceState> state_;
};

class AppTraceState {
public:
    AppTraceState(std::shared_ptr<AppTraceContext> context,
        std::shared_ptr<AppCallerEvent> appCallerEvent) : appTraceContext_(context), appCallerEvent_(appCallerEvent) {};
    virtual ~AppTraceState() {};

public:
    virtual int32_t GetState() = 0;
    virtual bool Accept(std::shared_ptr<AppTraceState> preState) = 0;
    virtual int32_t CaptureTrace() = 0;

    friend class StartTraceState;
    friend class DumpTraceState;
    friend class StopTraceState;
protected:
    std::shared_ptr<AppTraceContext> appTraceContext_;
    std::shared_ptr<AppCallerEvent> appCallerEvent_;
};

class StartTraceState : public AppTraceState {
public:
    StartTraceState(std::shared_ptr<AppTraceContext> context,
        std::shared_ptr<AppCallerEvent> appCallerEvent,
        std::shared_ptr<Plugin> plugin) : AppTraceState(context, appCallerEvent), plugin_(plugin) {};
    ~StartTraceState() {};

public:
    bool Accept(std::shared_ptr<AppTraceState> preState) override;
    int32_t CaptureTrace() override;
    int32_t GetState() override;

private:
    int32_t DoCaptureTrace();

private:
    std::shared_ptr<Plugin> plugin_;
};

class DumpTraceState : public AppTraceState {
public:
    DumpTraceState(std::shared_ptr<AppTraceContext> context,
        std::shared_ptr<AppCallerEvent> appCallerEvent,
        std::shared_ptr<Plugin> plugin) : AppTraceState(context, appCallerEvent), plugin_(plugin) {};
    ~DumpTraceState() {};

public:
    bool Accept(std::shared_ptr<AppTraceState> preState) override;
    int32_t CaptureTrace() override;
    int32_t GetState() override;

private:
    int32_t DoCaptureTrace();

private:
    std::shared_ptr<Plugin> plugin_;
};

class StopTraceState : public AppTraceState {
public:
    StopTraceState(std::shared_ptr<AppTraceContext> context,
        std::shared_ptr<AppCallerEvent> appCallerEvent) : AppTraceState(context, appCallerEvent) {};
    ~StopTraceState() {};

public:
    bool Accept(std::shared_ptr<AppTraceState> state) override;
    int32_t CaptureTrace() override;
    int32_t GetState() override;

private:
    int32_t DoCaptureTrace();
};
}; // HiviewDFX
}; // HiviewDFX
#endif // HIVIEW_PLUGINS_UNIFIED_COLLECTOR_DYNTRACE_INCLUDE_TRACE_CONTEXT_H