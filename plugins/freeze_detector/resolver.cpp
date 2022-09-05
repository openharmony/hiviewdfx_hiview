/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "resolver.h"

#include <sys/time.h>

#include "file_util.h"
#include "hiview_event_report.h"
#include "logger.h"
#include "string_util.h"
#include "sys_event.h"
#include "sys_event_dao.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("FreezeDetector");
bool FreezeResolver::Init()
{
    if (freezeCommon_ == nullptr) {
        return false;
    }
    freezeRuleCluster_ = freezeCommon_->GetFreezeRuleCluster();
    if (freezeRuleCluster_ == nullptr) {
        return false;
    }
    dBHelper_ = std::make_unique<DBHelper>(freezeCommon_);
    vendor_ = std::make_unique<Vendor>(freezeCommon_);
    return vendor_->Init();
}

void FreezeResolver::MatchEvent(const WatchPoint& watchPoint,
    const std::list<WatchPoint>& wpList, std::vector<WatchPoint>& list, const FreezeResult& result) const
{
    std::string domain = watchPoint.GetDomain();
    std::string stringId = watchPoint.GetStringId();
    std::string package = watchPoint.GetPackageName();
    for (auto &item : wpList) {
        if ((result.GetDomain() == item.GetDomain()) && (result.GetStringId() == item.GetStringId())) {
            if (result.GetSamePackage() == "true" && package != item.GetPackageName()) {
                HIVIEW_LOGE("failed to match the same package,"
                    " domain:%{public}s stringid:%{public}s pacakgeName:%{public}s"
                    " and domain:%{public}s stringid:%{public}s pacakgeName:%{public}s.",
                    domain.c_str(), stringId.c_str(), package.c_str(),
                    item.GetDomain().c_str(), item.GetStringId().c_str(), item.GetPackageName().c_str());
                continue;
            }
            list.push_back(item); // take watchpoint back
            break;
        }
    }
}

bool FreezeResolver::ResolveEvent(const WatchPoint& watchPoint,
    std::vector<WatchPoint>& list, std::vector<FreezeResult>& result) const
{
    if (freezeRuleCluster_ == nullptr) {
        return false;
    }
    if (!freezeRuleCluster_->GetResult(watchPoint, result)) {
        return false;
    }
    unsigned long long timestamp = watchPoint.GetTimestamp();
    for (auto& i : result) {
        int window = i.GetWindow();
        std::list<WatchPoint> wpList;
        if (window == 0) {
            list.push_back(watchPoint);
        } else if (window > 0) {
            unsigned long long start = timestamp;
            unsigned long long end = timestamp + (window * MILLISECOND);
            if (dBHelper_ != nullptr) {
                dBHelper_->SelectEventFromDB(false, start, end, wpList);
            }
        } else {
            unsigned long long start = timestamp + (window * MILLISECOND);
            unsigned long long end = timestamp;
            if (dBHelper_ != nullptr) {
                dBHelper_->SelectEventFromDB(false, start, end, wpList);
            }
        }
        MatchEvent(watchPoint, wpList, list, i);
    }

    HIVIEW_LOGI("list size %{public}zu", list.size());
    return true;
}

bool FreezeResolver::JudgmentResult(const WatchPoint& watchPoint,
    const std::vector<WatchPoint>& list, const std::vector<FreezeResult>& result) const
{
    if (watchPoint.GetDomain() == "ACE" && watchPoint.GetStringId() == "UI_BLOCK_6S") {
        if (list.size() == result.size()) {
            HIVIEW_LOGI("ACE UI_BLOCK has UI_BLOCK_3S UI_BLOCK_6S UI_BLOCK_RECOVERED as UI_JANK");
            return false;
        }

        if (list.size() != (result.size() - 1)) {
            return false;
        }

        for (auto& i : list) {
            if (i.GetStringId() == "UI_BLOCK_RECOVERED") {
                return false;
            }
        }
        return true;
    }

    if (list.size() == result.size()) {
        return true;
    }
    return false;
}

int FreezeResolver::ProcessEvent(const WatchPoint &watchPoint) const
{
    HIVIEW_LOGI("process event [%{public}s, %{public}s]",
        watchPoint.GetDomain().c_str(), watchPoint.GetStringId().c_str());
    if (vendor_ == nullptr) {
        return -1;
    }
    std::vector<WatchPoint> list;
    std::vector<FreezeResult> result;
    if (!ResolveEvent(watchPoint, list, result)) {
        HIVIEW_LOGW("no rule for event [%{public}s, %{public}s]",
            watchPoint.GetDomain().c_str(), watchPoint.GetStringId().c_str());
        return -1;
    }

    if (!JudgmentResult(watchPoint, list, result)) {
        HIVIEW_LOGW("no match event for event [%{public}s, %{public}s]",
            watchPoint.GetDomain().c_str(), watchPoint.GetStringId().c_str());
        return -1;
    }

    std::string logPath = vendor_->MergeEventLog(watchPoint, list, result);

    for (auto node : list) {
        if (dBHelper_ != nullptr) {
            dBHelper_->UpdateEventIntoDB(node, result[0].GetId());
        }
    }
    return 0;
}

std::string FreezeResolver::GetTimeZone() const
{
    std::string timeZone = "";
    struct timeval tv;
    struct timezone tz;
    if (gettimeofday(&tv, &tz) != 0) {
        HIVIEW_LOGE("failed to gettimeofday");
        return timeZone;
    }

    int hour = (-tz.tz_minuteswest) / MINUTES_IN_HOUR;
    timeZone = (hour >= 0) ? "+" : "-";

    int absHour = std::abs(hour);
    if (absHour < 10) {
        timeZone.append("0");
    }
    timeZone.append(std::to_string(absHour));

    int minute = (-tz.tz_minuteswest) % MINUTES_IN_HOUR;
    int absMinute = std::abs(minute);
    if (absMinute < 10) {
        timeZone.append("0");
    }
    timeZone.append(std::to_string(absMinute));

    return timeZone;
}
} // namespace HiviewDFX
} // namespace OHOS
