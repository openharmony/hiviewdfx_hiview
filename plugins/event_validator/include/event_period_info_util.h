/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef HIVIEW_PLUGINS_EVENT_PERIOD_INFO_UTIL_H
#define HIVIEW_PLUGINS_EVENT_PERIOD_INFO_UTIL_H

#include "sys_event.h"
#include "period_file_operator.h"
#include "plugin.h"

namespace OHOS {
namespace HiviewDFX {
constexpr uint64_t DEFAULT_PERIOD_SEQ = 1; // period seq begins with 1

struct EventPeriodInfo {
    // format: YYYYMMDDHH
    std::string timeStamp;

    // count of event which will be preserve into db file in 1 hour
    uint64_t preserveCnt = 0;

    // count of event which will be exported in 1 hour
    uint64_t exportCnt = 0;

    EventPeriodInfo(const std::string& timeStamp, uint64_t preserveCnt, uint64_t exportCnt)
        : timeStamp(timeStamp), preserveCnt(preserveCnt), exportCnt(exportCnt) {}
};

class EventPeriodInfoUtil {
public:
    void Init(HiviewContext* context);
    void UpdatePeriodInfo(const std::shared_ptr<SysEvent> event);

private:
    void ClearPeriodInfoList();
    void RecordEventPeriodInfo();

private:
    uint64_t periodSeq_ = DEFAULT_PERIOD_SEQ;
    std::list<std::shared_ptr<EventPeriodInfo>> periodInfoList_;
    std::unique_ptr<PeriodInfoFileOperator> periodFileOpt_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGINS_EVENT_PERIOD_INFO_UTIL_H
