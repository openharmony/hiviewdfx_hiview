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

#include "active_key_event.h"

#include <vector>

#include "event_log_task.h"
#include "file_util.h"
#include "ffrt.h"
#include "logger.h"
#include "sys_event.h"
#include "time_util.h"
#include "trace_collector.h"
namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_LABEL(0xD002D01, "EventLogger-ActiveKeyEvent");
ActiveKeyEvent::ActiveKeyEvent()
{
    triggeringTime_ = 0;
    logStore_ = nullptr;
}

ActiveKeyEvent::~ActiveKeyEvent()
{
    for (auto it = subscribeIds_.begin(); it != subscribeIds_.end(); it = subscribeIds_.erase(it)) {
        if (*it >= 0) {
            MMI::InputManager::GetInstance()->UnsubscribeKeyEvent(*it);
            HIVIEW_LOGI("~ActiveKeyEvent subscribeId_: %{public}d", *it);
        }
    }
}

int64_t ActiveKeyEvent::SystemTimeMillisecond()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (int64_t)((t.tv_sec) * TimeUtil::SEC_TO_NANOSEC + t.tv_nsec) / TimeUtil::SEC_TO_MICROSEC;
}

void ActiveKeyEvent::InitSubscribe(int32_t preKey, int32_t finalKey, int32_t count)
{
    const int32_t maxCount = 5;
    if (++count > maxCount) {
        return;
    }
    std::shared_ptr<MMI::KeyOption> keyOption = std::make_shared<MMI::KeyOption>();
    if (keyOption == nullptr) {
        HIVIEW_LOGE("Invalid key option");
    }

    std::set<int32_t> preKeys;
    preKeys.insert(preKey);
    keyOption->SetPreKeys(preKeys);
    keyOption->SetFinalKey(finalKey);
    keyOption->SetFinalKeyDown(true);
    const int holdTime = 500;
    keyOption->SetFinalKeyDownDuration(holdTime);
    auto keyEventCallBack = std::bind(&ActiveKeyEvent::CombinationKeyCallback, this, std::placeholders::_1);
    int32_t subscribeId = MMI::InputManager::GetInstance()->SubscribeKeyEvent(keyOption, keyEventCallBack);
    if (subscribeId < 0) {
        HIVIEW_LOGE("SubscribeKeyEvent failed, %{public}d_%{public}d,"
            "subscribeId: %{public}d option failed.", preKey, finalKey, subscribeId);
        auto task = [this, &preKey, &finalKey, count] {
            this->InitSubscribe(preKey, finalKey, count);
            taskOutDeps++;
        };
        std::string taskName("initSubscribe" + std::to_string(finalKey) + "_" + std::to_string(count));
        ffrt::submit(task, {}, {&taskOutDeps}, ffrt::task_attr().name(taskName.c_str()));
    }
    subscribeIds_.emplace_back(subscribeId);
    HIVIEW_LOGI("CombinationKeyInit %{public}d_ %{public}d subscribeId_: %{public}d",
        preKey, finalKey, subscribeId);
}

void ActiveKeyEvent::Init(std::shared_ptr<LogStoreEx> logStore)
{
    HIVIEW_LOGI("CombinationKeyInit");
    logStore_ = logStore;

    auto initSubscribeDown = std::bind(&ActiveKeyEvent::InitSubscribe, this,
        MMI::KeyEvent::KEYCODE_VOLUME_UP, MMI::KeyEvent::KEYCODE_VOLUME_DOWN, 0);
    auto initSubscribePower = std::bind(&ActiveKeyEvent::InitSubscribe, this,
        MMI::KeyEvent::KEYCODE_VOLUME_DOWN, MMI::KeyEvent::KEYCODE_POWER, 0);
    ffrt::submit(initSubscribeDown, {}, {}, ffrt::task_attr().name("initSubscribeDown").qos(ffrt::qos_default));
    ffrt::submit(initSubscribePower, {}, {}, ffrt::task_attr().name("initSubscribePower").qos(ffrt::qos_default));
}

void ActiveKeyEvent::HitraceCapture()
{
    std::shared_ptr<UCollectUtil::TraceCollector> collector = UCollectUtil::TraceCollector::Create();
    UCollectUtil::TraceCollector::Caller caller = UCollectUtil::TraceCollector::Caller::BETACLUB;
    auto result = collector->DumpTrace(caller);
    if (result.retCode != 0) {
        HIVIEW_LOGE("get hitrace fail! error code: %{public}d", result.retCode);
        return;
    }
}

void ActiveKeyEvent::SysMemCapture(int fd)
{
    FileUtil::SaveStringToFd(fd, "\n\ncatcher cmd : /proc/meminfo\n");
    std::string content;
    FileUtil::LoadStringFromFile("/proc/meminfo", content);
    FileUtil::SaveStringToFd(fd, content);
}

void ActiveKeyEvent::DumpCapture(int fd)
{
    SysEventCreator sysEventCreator("HIVIEWDFX", "ACTIVE_KEY", SysEventCreator::FAULT);
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("ActiveKeyEvent", nullptr, sysEventCreator);
    std::unique_ptr<EventLogTask> logTask = std::make_unique<EventLogTask>(fd, sysEvent);
    for (const std::string& cmd : CMD_LIST) {
        logTask->AddLog(cmd);
    }

    auto ret = logTask->StartCompose();
    if (ret != EventLogTask::TASK_SUCCESS) {
        HIVIEW_LOGE("capture fail %{public}d", ret);
    }
    SysMemCapture(fd);
}

void ActiveKeyEvent::CombinationKeyHandle(std::shared_ptr<MMI::KeyEvent> keyEvent)
{
    HIVIEW_LOGI("Receive CombinationKeyHandle.");
    if (logStore_ == nullptr) {
        return;
    }

    std::string logFile = "ACTIVE_KEY_EVENT-0-" +
        TimeUtil::TimestampFormatToDate(TimeUtil::GetMilliseconds() / TimeUtil::SEC_TO_MILLISEC,
        "%Y%m%d%H%M%S") + ".log";
    if (FileUtil::FileExists("/data/log/eventlog/" + logFile)) {
        HIVIEW_LOGW("filename: %{public}s is existed, direct use.", logFile.c_str());
        return;
    }
    int fd = logStore_->CreateLogFile(logFile);

    auto sysStart = ActiveKeyEvent::SystemTimeMillisecond();
    const uint32_t placeholder = 3;
    auto start = TimeUtil::GetMilliseconds();
    uint64_t startTime = start / TimeUtil::SEC_TO_MILLISEC;
    std::ostringstream startTimeStr;
    startTimeStr << "start time: " << TimeUtil::TimestampFormatToDate(startTime, "%Y/%m/%d-%H:%M:%S");
    startTimeStr << ":" << std::setw(placeholder) << std::setfill('0') <<
        std::to_string(start % TimeUtil::SEC_TO_MILLISEC) << std::endl;
    std::vector<int32_t> keys = keyEvent->GetPressedKeys();
    for (auto& i : keys) {
        startTimeStr << "CombinationKeyCallback key : ";
        startTimeStr << MMI::KeyEvent::KeyCodeToString(i) << std::endl;
    }
    FileUtil::SaveStringToFd(fd, startTimeStr.str());

    auto hitraceCapture = std::bind(&ActiveKeyEvent::HitraceCapture, this);
    ffrt::submit(hitraceCapture, {}, {}, ffrt::task_attr().name("HitraceCapture").qos(ffrt::qos_user_initiated));

    DumpCapture(fd);
    auto end = ActiveKeyEvent::SystemTimeMillisecond();
    std::string totalTime = "\n\nCatcher log total time is " + std::to_string(end - sysStart) + "ms\n";
    FileUtil::SaveStringToFd(fd, totalTime);
    close(fd);
}

void ActiveKeyEvent::CombinationKeyCallback(std::shared_ptr<MMI::KeyEvent> keyEvent)
{
    HIVIEW_LOGI("Receive CombinationKeyCallback key id: %{public}d.", keyEvent->GetId());
    uint64_t now = (uint64_t)ActiveKeyEvent::SystemTimeMillisecond();
    const uint64_t interval = 10000;
    if (now - triggeringTime_ < interval) {
        return;
    }
    triggeringTime_ = now;
    auto combinationKeyHandle = std::bind(&ActiveKeyEvent::CombinationKeyHandle, this, keyEvent);
    ffrt::submit(combinationKeyHandle, {}, {},
        ffrt::task_attr().name("ActiveKeyEvent").qos(ffrt::qos_user_initiated));
}
} // namesapce HiviewDFX
} // namespace OHOS