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

#ifndef OHOS_HIVIEWDFX_RUNNIG_STATUS_LOGGER_H
#define OHOS_HIVIEWDFX_RUNNIG_STATUS_LOGGER_H

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include "singleton.h"

namespace OHOS {
namespace HiviewDFX {
using LogWritingTask = std::pair<std::string, std::function<void(const std::string&)>>;
class RunningStatusLogger : public OHOS::DelayedRefSingleton<RunningStatusLogger> {
public:
    void Log(const std::string& logInfo);
    std::string FormatTimeStamp(bool simpleMode = false);

private:
    std::string GenerateNewestFileName(const std::string& suffix);
    std::string GetLogDir();
    std::string GetLogWroteDestFile(const std::string& content);
    void ImmediateWrite(bool needPop = false);

private:
    std::atomic<bool> inWriting = false;
    std::mutex writingMutex;
    std::queue<LogWritingTask> logWritingTasks;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_RUNNIG_STATUS_LOGGER_H
