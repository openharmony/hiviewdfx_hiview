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
#ifndef EVENT_LOGGER_PEER_BINDER_LOG_CATCHER
#define EVENT_LOGGER_PEER_BINDER_LOG_CATCHER
#include <fstream>
#include <string>
#include <memory>
#include <vector>

#include "sys_event.h"

#include "event_log_catcher.h"
namespace OHOS {
namespace HiviewDFX {
class PeerBinderCatcher : public EventLogCatcher {
public:
    explicit PeerBinderCatcher();
    ~PeerBinderCatcher() override{};
    bool Initialize(const std::string& strParam1, int pid, int layer) override;
    int Catch(int fd) override;
    void Init(std::shared_ptr<SysEvent> event, const std::string& filePath);

    static const inline std::string LOGGER_EVENT_PEERBINDER = "PeerBinder";
    static const inline std::string LOGGER_BINDER_DEBUG_PROC_PATH = "/proc/transaction_proc";
private:
    struct BinderInfo {
        int client;
        int server;
        int wait;
    };
    enum {
        LOGGER_BINDER_STACK_ONE = 0,
        LOGGER_BINDER_STACK_ALL = 1,
    };

    int pid_ = 0;
    int layer_ = 0;
    std::string binderPath_ = LOGGER_BINDER_DEBUG_PROC_PATH;
    std::shared_ptr<SysEvent> event_ = nullptr;

    std::map<int, std::list<PeerBinderCatcher::BinderInfo>> BinderInfoParser(std::ifstream& fin, int fd) const;
    void ParseBinderCallChain(std::map<int, std::list<PeerBinderCatcher::BinderInfo>>& manager,
    std::set<int>& pids, int pid) const;
    std::set<int> GetBinderPeerPids(int fd) const;
    void CatcherStacktrace(int fd, int pid) const;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // EVENT_LOGGER_PEER_BINDER_LOG_CATCHER