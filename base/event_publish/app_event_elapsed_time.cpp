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

#include "app_event_elapsed_time.h"

#include "hiview_logger.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
DEFINE_LOG_TAG("HiView-AppEventElapsedTime");
}

ElapsedTime::ElapsedTime()
{
    begin_ = TimeUtil::GetMilliseconds();
    lastMarkTime_ = begin_;
}

ElapsedTime::ElapsedTime(uint64_t limitCostMilliseconds, std::string printContent)
    :limitCostMilliseconds_(limitCostMilliseconds), printContent_(std::move(printContent))
{
    begin_ = TimeUtil::GetMilliseconds();
    lastMarkTime_ = begin_;
}

ElapsedTime::~ElapsedTime()
{
    uint64_t costTime = TimeUtil::GetMilliseconds() - begin_;
    if (costTime >= limitCostMilliseconds_) {
        HIVIEW_LOGI("%{public}s running %{public}" PRId64 " ms", printContent_.c_str(), costTime);
    }
}

void ElapsedTime::MarkElapsedTime(const std::string& markContent)
{
    uint64_t curMarkTime = TimeUtil::GetMilliseconds();
    if (curMarkTime - lastMarkTime_ >= limitCostMilliseconds_) {
        HIVIEW_LOGI("%{public}s cost %{public}" PRId64 " ms", markContent.c_str(), curMarkTime - lastMarkTime_);
    }
    lastMarkTime_ = curMarkTime;
}
} // namespace HiviewDFX
} // namespace OHOS