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

#ifndef OHOS_HIVIEWDFX_QUERY_STATUS_LOG_UTIL
#define OHOS_HIVIEWDFX_QUERY_STATUS_LOG_UTIL

#include <ctime>
#include <string>

namespace OHOS {
namespace HiviewDFX {
class QueryStatusLogUtil {
public:
    static void LogTooManyQueryRules(const std::string sql);
    static void LogTooManyConcurrentQueries(const int limit, bool innerQuery = true);
    static void LogQueryOverTime(time_t costTime, const std::string sql, bool innerQuery = true);
    static void LogQueryCountOverLimit(const int32_t queryCount, const std::string& sql,
        bool innerQuery = true);
    static void LogQueryTooFrequently(const std::string& sql, const std::string& processName = std::string(""),
        bool innerQuery = true);

private:
    static void Logging(const std::string& detail);
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_QUERY_STATUS_LOG_UTIL