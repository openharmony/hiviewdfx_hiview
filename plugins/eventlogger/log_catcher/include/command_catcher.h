/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef EVENT_LOGGER_COMMAND_CATCHER
#define EVENT_LOGGER_COMMAND_CATCHER

#include <map>
#include <string>
#include <vector>

#include "event_log_catcher.h"
namespace OHOS {
namespace HiviewDFX {
class CommandCatcher : public EventLogCatcher {
public:
    explicit CommandCatcher();
    ~CommandCatcher() override {};
    bool Initialize(const std::string& packageNam, int pid, int intParam2) override;
    int Catch(int fd) override;
    void AddCmd(const std::vector<std::string>& cmd);
    int GetPid() const
    {
        return pid_;
    }
private:
    int pid_ = -1;
    std::string packageName_ = "";
    std::vector<std::vector<std::string>> cmdString_;

    int HiDumper(int fd, const std::vector<std::string> &args);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // EVENT_LOGGER_BINDER_LOG_CATCHER