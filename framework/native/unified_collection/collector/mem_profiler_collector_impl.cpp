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

#include <memory>
#include <thread>
#include <common.h>
#include "logger.h"
#include "mem_profiler_collector.h"
#include "native_memory_profiler_sa_client_manager.h"
#include "native_memory_profiler_sa_config.h"
#include "parameters.h"

using namespace OHOS::Developtools::NativeDaemon;

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
DEFINE_LOG_TAG("UCollectUtil-MemProfilerCollector");

const std::string NATIVE_DAEMON_NAME("native_daemon");
int g_nativeDaemonPid = 0;
constexpr int WAIT_EXIT_MILLS = 1000;

class MemProfilerCollectorImpl : public MemProfilerCollector {
public:
    MemProfilerCollectorImpl() = default;
    virtual ~MemProfilerCollectorImpl() = default;

public:
    virtual void start(Developtools::NativeDaemon::NativeMemoryProfilerSaClientManager::NativeMemProfilerType type,
                       int pid, int duration, int sampleInterval) override;
    virtual void stop(int pid) override;
};

void MemProfilerCollectorImpl::start(Developtools::NativeDaemon::NativeMemoryProfilerSaClientManager::NativeMemProfilerType type,
           int pid, int duration, int sampleInterval)
{
    OHOS::system::SetParameter("hiviewdfx.hiprofiler.memprofiler.start", "1");
    while (!COMMON::IsProcessExist(NATIVE_DAEMON_NAME, g_nativeDaemonPid)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_EXIT_MILLS));
    }
    NativeMemoryProfilerSaClientManager::Start(type, pid, duration, sampleInterval);
}

void MemProfilerCollectorImpl::stop(int pid)
{
    OHOS::system::SetParameter("hiviewdfx.hiprofiler.memprofiler.start", "0");
    while (COMMON::IsProcessExist(NATIVE_DAEMON_NAME, g_nativeDaemonPid)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_EXIT_MILLS));
    }
    NativeMemoryProfilerSaClientManager::Stop(pid);
}

void MemProfilerCollectorImpl::Dump(int fd, Developtools::NativeDaemon::NativeMemoryProfilerSaClientManager::NativeMemProfilerType type,
           int pid, int duration, int sampleInterval)
{
    OHOS::system::SetParameter("hiviewdfx.hiprofiler.memprofiler.start", "0");
    while (COMMON::IsProcessExist(NATIVE_DAEMON_NAME, g_nativeDaemonPid)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_EXIT_MILLS));
    }
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    if (type == Developtools::NativeDaemon::NativeMemoryProfilerSaClientManager::NativeMemProfilerType::MEM_PROFILER_LIBRARY) {
        config->responseLibraryMode_ = true;
    } else if (type == Developtools::NativeDaemon::NativeMemoryProfilerSaClientManager::NativeMemProfilerType::MEM_PROFILER_CALL_STACK) {
        config->responseLibraryMode_ = false;
    }
    config->pid_ = pid;
    config->duration_ = duration;
    config->statisticsInterval_ = (uint32_t)sampleInterval;
    NativeMemoryProfilerSaClientManager::DumpData(fd, config);
}

std::shared_ptr<MemProfilerCollector> MemProfilerCollector::Create()
{
    static std::shared_ptr<MemProfilerCollector> instance_ = std::make_shared<MemProfilerCollectorImpl>();
    return instance_;
}

} // UCollectUtil
} // HiViewDFX
} // OHOS
