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

#include "sanitizerd_collector.h"

#include <fcntl.h>
#include <map>
#include <sys/stat.h>

#include "reporter.h"
#include "sanitizerd_log.h"

namespace OHOS {
namespace HiviewDFX {
SanitizerdCollector::SanitizerdCollector(std::unordered_map<std::string, std::string> &stkmap) : stacks_(stkmap)
{
}

SanitizerdCollector::~SanitizerdCollector()
{
}

bool SanitizerdCollector::IsDuplicate(const std::string hash) const
{
    auto backIter = stacks_.find(hash);
    return (backIter != stacks_.end());
}

void SanitizerdCollector::Collect(const std::string& szfile) const
{
    HILOG_INFO(LOG_CORE, "sanCollecting(%{public}s)\n", szfile.c_str());
}
} // namespace HiviewDFX
} // namespace OHOS

