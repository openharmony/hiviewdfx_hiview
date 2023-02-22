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

#ifndef SANITIZERD_REPORTER_H
#define SANITIZERD_REPORTER_H
#include <map>
#include <string>

constexpr static unsigned MAX_SANITIZERD_TYPE = 20;

namespace OHOS {
namespace HiviewDFX {
enum SanitizerdType {
    ASAN_LOG_RPT,
    KASAN_LOG_RPT,
    UBSAN_LOG_RPT,
    LSAN_LOG_RPT,
    MAX_LOG_RPT
};
enum FieldTypeOfSanitizerd {
    ORISANITIZERTYPE,
    HISYSEVENTTYPE,
    PREFIXFILENAME,
    MAX_FIELD_TYPE
};

/**
 * {"match origin sanitizer", "match hisysevent", "match prefix of filename"},
 */
const char SANITIZERD_TYPE_STR[][MAX_FIELD_TYPE][MAX_SANITIZERD_TYPE] = {
    {"AddressSanitizer", "ADDR_SANITIZER",      "asan"},
    {"KASAN",            "KERN_ADDR_SANITIZER", "kasan"},
    {"UBSAN",            "UND_SANITIZER",       "ubsan"},
    {"LeakSanitizer",    "LEAK_SANITIZER",      "leak"},
};

using T_SANITIZERD_PARAMS = struct {
    SanitizerdType type;
    std::string hash;
    int32_t     pid;
    int32_t     uid;
    std::string procName;
    std::string appVersion;
    std::string errType;
    std::string errTypeInShort;
    std::string logName;
    std::string description;
    uint64_t    happenTime;
    std::string func;
};

void Upload(T_SANITIZERD_PARAMS *params);
int Init(SanitizerdType type);
} // namespace HiviewDFX
} // namespace OHOS

#endif // SANITIZERD_REPORTER_H

