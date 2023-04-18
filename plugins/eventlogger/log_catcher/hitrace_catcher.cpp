/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hitrace_catcher.h"

#include <fstream>
#include <sys/time.h>

#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability.h"

#include "securec.h"

#include "common_utils.h"
#include "file_util.h"
#include "time_util.h"
#include "logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_LABEL(0xD002D01, "EventLogger-HitraceCatcher");
namespace {
constexpr int XPEREF_SYS_TRACE_SERVICE_ABILITY_ID = 1208;
constexpr int64_t EXTRACE_TIME = 12;

class IHitraceService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.PerformanceDFX.IHierviceAbility");
    enum {
        TRANS_ID_PING_ABILITY = 1,
    };

    virtual int32_t DumpHitrace(const std::string &fullTracePath, int64_t beginTime) = 0;
};

int64_t GetSystemBootTime()
{
    struct timespec bts = {0, 0};
    clock_gettime(CLOCK_BOOTTIME, &bts);
    return static_cast<int64_t>(((static_cast<int64_t>(bts.tv_sec)) * TimeUtil::SEC_TO_NANOSEC + bts.tv_nsec)
                                / TimeUtil::SEC_TO_NANOSEC);
}
}

HitraceCatcher::HitraceCatcher() : EventLogCatcher()
{
    event_ = nullptr;
    name_ = "HitraceCatcher";
}

bool HitraceCatcher::Initialize(const std::string& strParam1, int intParam1, int intParam2)
{
    description_ = "";
    return true;
}

bool HitraceCatcher::Init(std::shared_ptr<SysEvent> event)
{
    event_ = event;
    return true;
}

int HitraceCatcher::Catch(int fd)
{
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        HIVIEW_LOGE("Get SystemAbilityManager failed!");
        return 0;
    }

    OHOS::sptr<OHOS::IRemoteObject> remoteObject = samgr->GetSystemAbility(XPEREF_SYS_TRACE_SERVICE_ABILITY_ID);
    if (remoteObject == nullptr) {
        HIVIEW_LOGE("Get xperf -Trace Manager failed!");
        return 0;
    }

    auto iHitraceService = iface_cast<IHitraceService>(remoteObject);
    if (iHitraceService == nullptr) {
        HIVIEW_LOGE("Get IHitraceService failed!");
        return 0;
    }

    int64_t currentTime = GetSystemBootTime();
    int64_t beginTime = currentTime - EXTRACE_TIME;
    auto logTime = TimeUtil::GetMilliseconds() / TimeUtil::SEC_TO_MILLISEC;
    const unsigned int bufSize25 = 25;
    char hitraceTimeBuf[bufSize25] = {0};
    int ret = snprintf_s(hitraceTimeBuf, bufSize25, bufSize25 - 1, "%s-%08lld",
                         TimeUtil::TimestampFormatToDate(logTime, "%Y%m%d%H%M%S").c_str(),
                         currentTime);
    if (ret < 0) {
        HIVIEW_LOGE("hitraceTime snprintf_s error %{public}d!", ret);
        return 0;
    }
    std::string hitraceTime = hitraceTimeBuf;
    std::string fullTracePath = FULL_DIR + "hitrace-" + hitraceTime + ".sys";

    HIVIEW_LOGI("start dumpHitrace beginTime:%{public}lld, Path:%{public}s", beginTime, fullTracePath.c_str());
    ret = iHitraceService->DumpHitrace(fullTracePath, beginTime);
    if (ret != 0) {
        HIVIEW_LOGE("Get iHitraceService DumpHitrace failed : %{public}d!", ret);
        return 0;
    }
    HIVIEW_LOGI("end dumpHitrace");

    if (event_ != nullptr) {
        event_->SetEventValue("HITRACE_TIME", hitraceTime);
    }
    auto originSize = GetFdSize(fd);
    FileUtil::SaveStringToFd(fd, "\nHitraceCatcher--fullTracePath:" + std::string(fullTracePath) + "\n");
    logSize_ = GetFdSize(fd) - originSize;
    return logSize_;
}
}
}