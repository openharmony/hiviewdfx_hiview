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

#include "query_status_log_util.h"

#include "running_status_logger.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr char SPACE_CONCAT[] = " ";
}

void QueryStatusLogUtil::LogTooManyQueryRules(const std::string sql)
{
    std::string info { "PLUGIN TOOMANYQUERYCONDITION SQL=" };
    info.append(sql);
    Logging(info);
}

void QueryStatusLogUtil::LogTooManyConcurrentQueries(const int limit, bool innerQuery)
{
    std::string info { innerQuery ? "HIVIEW TOOMANYCOCURRENTQUERIES " : "TOOMANYCOCURRENTQUERIES " };
    info.append("COUNT > ");
    info.append(std::to_string(limit));
    Logging(info);
}

void QueryStatusLogUtil::LogQueryOverTime(time_t costTime, const std::string sql, bool innerQuery)
{
    std::string info { innerQuery ? "PLUGIN QUERYOVERTIME " : "QUERYOVERTIME " };
    info.append(std::to_string(costTime)).append(SPACE_CONCAT).append("SQL=").append(sql);
    Logging(info);
}

void QueryStatusLogUtil::LogQueryCountOverLimit(const int32_t queryCount, const std::string& sql,
    bool innerQuery)
{
    std::string info { innerQuery ? "PLUGIN QUERYCOUNTOVERLIMIT " : "QUERYCOUNTOVERLIMIT " };
    info.append(std::to_string(queryCount)).append(SPACE_CONCAT).append("SQL=").append(sql);
    Logging(info);
}

void QueryStatusLogUtil::LogQueryTooFrequently(const std::string& sql, const std::string& processName,
    bool innerQuery)
{
    std::string info { innerQuery ? "HIVIEW QUERYTOOFREQUENTLY " : "QUERYTOOFREQUENTLY " };
    if (!processName.empty()) {
        info.append(processName).append(SPACE_CONCAT);
    }
    info.append("SQL=").append(sql);
    Logging(info);
}

void QueryStatusLogUtil::Logging(const std::string& detail)
{
    std::string info = RunningStatusLogger::GetInstance().FormatTimeStamp();
    info.append(SPACE_CONCAT).append(detail);
    RunningStatusLogger::GetInstance().Log(info);
}
} // namespace HiviewDFX
} // namespace OHOS