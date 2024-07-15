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
#ifndef EVENT_LOGGER_SHELL_CATCHER
#define EVENT_LOGGER_SHELL_CATCHER

#include <map>
#include <string>
#include <vector>

#include "event_log_catcher.h"
#include "sys_event.h"
namespace OHOS {
namespace HiviewDFX {
class ShellCatcher : public EventLogCatcher {
public:
    explicit ShellCatcher();
    ~ShellCatcher() override {};
    bool Initialize(const std::string& cmd, int type, int intParam2) override;
    int Catch(int fd, int jsonFd) override;
    void SetEvent(std::shared_ptr<SysEvent> event);

    enum CATCHER_TYPE {
        CATCHER_UNKNOWN = -1,
        CATCHER_CPU,
        CATCHER_WMS,
        CATCHER_AMS,
        CATCHER_PMS,
        CATCHER_DPMS,
        CATCHER_RS,
        CATCHER_MMI,
        CATCHER_DMS,
        CATCHER_SNAPSHOT,
        CATCHER_HILOG,
        CATCHER_LIGHT_HILOG,
        CATCHER_SCBSESSION,
        CATCHER_SCBVIEWPARAM,
        CATCHER_SCBWMS,
        CATCHER_SCBWMSEVT,
        CATCHER_DAM,
        CATCHER_INPUT_EVENT_HILOG,
        CATCHER_INPUT_HILOG,
    };
private:
    static const inline int32_t DEFAULT_WINDOW_ID = 14;
    std::string catcherCmd_ = "";
    int pid_ = -1;
    CATCHER_TYPE catcherType_ = CATCHER_TYPE::CATCHER_UNKNOWN;
    std::shared_ptr<SysEvent> event_ = nullptr;

    int InInputProcesscatcher(int writeFd);
    int CaDoInChildProcesscatcher(int writeFd);
    void DoChildProcess(int writeFd);
    bool ReadShellToFile(int fd, const std::string& cmd);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // EVENT_LOGGER_SHELL_CATCHER
