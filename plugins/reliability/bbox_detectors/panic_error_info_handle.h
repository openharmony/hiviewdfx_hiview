/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PANIC_ERROR_INFO_HANDLE_H_
#define PANIC_ERROR_INFO_HANDLE_H_

#include <string>
#include <memory>

namespace OHOS {
namespace HiviewDFX {
namespace PanicErrorInfoHandle {

constexpr int EVENT_MAX_LEN = 32;
constexpr int CATEGORY_MAX_LEN = 32;
constexpr int MODULE_MAX_LEN = 32;
constexpr int TIMESTAMP_MAX_LEN = 24;
constexpr int ERROR_DESC_MAX_LEN = 512;

struct ErrorInfo {
    char event[EVENT_MAX_LEN];
    char category[CATEGORY_MAX_LEN];
    char module[MODULE_MAX_LEN];
    char errorTime[TIMESTAMP_MAX_LEN];
    char errorDesc[ERROR_DESC_MAX_LEN];
};

/**
 * Trans bbox data to history, trans pstore data to kmsg log when panic.
 */
void RKTransData(std::string bboxData, std::string bboxSysreset);

/**
 * Save history log.
 */
void SaveHistoryLog(std::string bboxTime, std::string bboxSysreset, ErrorInfo* info);

/**
 * Copy pstore file to history log.
 */
void CopyPstoreFileToHistoryLog(std::ifstream &fin);

/**
 * Get Top category when painc.
 */
const char *GetTopCategory(const char *module, const char *event);

/**
 * Get category when painc.
 */
const char *GetCategory(const char *module, const char *event);

/**
 * Get kmsg date yyyymmdd-hhmmss when panic.
 */
std::string GetKmsgDate();
}
}
}

#endif // PANIC_ERROR_INFO_HANDLE_H_
