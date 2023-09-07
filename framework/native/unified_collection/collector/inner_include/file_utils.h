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
#ifndef FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_FILE_UTILS_H
#define FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_FILE_UTILS_H
#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include "hilog/log.h"
#include "hitrace_dump.h"
#include "securec.h"
#include "file_util.h"
#include "trace_collector.h"
namespace OHOS {
namespace HiviewDFX {
namespace UnifiedPath {
    const std::string UNIFIED_SHARE_PATH = "/data/log/hiview/unified_collection/trace/share/";
    const std::string UNIFIED_SPECIAL_PATH = "/data/log/hiview/unified_collection/trace/special/";
}

namespace TraceCaller {
    const std::string RELIABILITY = "Reliability";
    const std::string XPERF = "Xperf";
    const std::string XPOWER = "Xpower";
    const std::string BETACLUB = "BetaClub";
    const std::string OTHER = "Other";
}

namespace TraceCount {
    const uint32_t UNIFIED_SHARE_COUNTS = 20;
    const uint32_t UNIFIED_SPECIAL_XPERF = 3;
    const uint32_t UNIFIED_SPECIAL_OTHER = 5;
}

enum {
    share = 0,
    special = 1,
};

void FileRemove(UCollectUtil::TraceCollector::Caller &caller);
void CheckAndCreateDirectory(const std::string &tmpDirPath);
bool CreateMultiDirectory(const std::string &dirPath);
const std::string EnumToString(UCollectUtil::TraceCollector::Caller &caller);
std::vector<std::string> GetUnifiedFiles(Hitrace::TraceRetInfo ret, UCollectUtil::TraceCollector::Caller &caller);
bool IsTraceExists(const std::string &trace);
void CopyXperfToSpecialPath(const std::string &trace, const std::string &revRightStr);
std::vector<std::string> GetUnifiedShareFiles(Hitrace::TraceRetInfo ret, UCollectUtil::TraceCollector::Caller &caller);
std::vector<std::string> GetUnifiedSpecialFiles(Hitrace::TraceRetInfo ret,
    UCollectUtil::TraceCollector::Caller &caller);
} // HiViewDFX
} // OHOS
#endif // FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_FILE_UTILS_H
