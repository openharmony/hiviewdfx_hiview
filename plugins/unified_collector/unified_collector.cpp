/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include "unified_collector.h"

#include "file_util.h"
#include "logger.h"
#include "plugin_factory.h"

namespace OHOS {
namespace HiviewDFX {
REGISTER(UnifiedCollector);
DEFINE_LOG_TAG("HiView-UnifiedCollector");

void UnifiedCollector::OnLoad()
{
    HIVIEW_LOGI("start to load UnifiedCollector plugin");
    Init();
}

void UnifiedCollector::OnUnload()
{
    HIVIEW_LOGI("start to unload UnifiedCollector plugin");
}

void UnifiedCollector::Init()
{
    if (GetHiviewContext() == nullptr) {
        HIVIEW_LOGE("hiview context is null");
        return;
    }
    InitWorkLoop();
    InitWorkPath();
    RunCpuCollectionTask();
}

void UnifiedCollector::InitWorkLoop()
{
    workLoop_ = GetHiviewContext()->GetSharedWorkLoop();
}

void UnifiedCollector::InitWorkPath()
{
    std::string hiviewWorkDir = GetHiviewContext()->GetHiViewDirectory(HiviewContext::DirectoryType::WORK_DIRECTORY);
    const std::string uCollectionDirName = "unified_collection";
    std::string tempWorkPath = FileUtil::IncludeTrailingPathDelimiter(hiviewWorkDir.append(uCollectionDirName));
    if (!FileUtil::IsDirectory(tempWorkPath) && !FileUtil::ForceCreateDirectory(tempWorkPath)) {
        HIVIEW_LOGE("failed to create dir=%{public}s", tempWorkPath.c_str());
        return;
    }
    workPath_ = tempWorkPath;
}

void UnifiedCollector::RunCpuCollectionTask()
{
    if (workLoop_ == nullptr) {
        HIVIEW_LOGE("workLoop is null");
        return;
    }
    if (workPath_.empty()) {
        HIVIEW_LOGE("workPath is null");
        return;
    }
    cpuCollectionTask_ = std::make_shared<CpuCollectionTask>(workPath_);
    const uint64_t taskInterval = 10; // 10s
    workLoop_->AddTimerEvent(
        nullptr,
        nullptr,
        std::bind(&CpuCollectionTask::Collect, cpuCollectionTask_.get()),
        taskInterval,
        true);
}
}  // namespace HiviewDFX
}  // namespace OHOS
