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

#ifndef PANIC_REPORT_RECOVERY_H_
#define PANIC_REPORT_RECOVERY_H_

#include <memory>
#include <string>

#include "sys_event.h"

namespace OHOS {
namespace HiviewDFX {
namespace PanicReport {

/**
 * Initialize the configuration file.
 *
 * @return whether init completed.
 */
bool InitPanicConfigFile();

/**
 * Initialize the panic report.
 *
 * @return whether init completed.
 */
bool InitPanicReport();

/**
 * The function to determine whether the system boot completed.
 *
 * @return whether whether the system starts normally.
 */
bool IsBootCompleted();

/**
 * The function to determine whether the system is crashed within 10 minutes after boot completed.
 *
 * @return whether the system is crashed within 10 minutes after boot completed.
 */
bool IsLastShortStartUp();

/**
 * The function to determine whether the sysEvent is recovery panic.
 *
 * @return whether the event is recovery panic event.
 */
bool IsRecoveryPanicEvent(const std::shared_ptr<SysEvent>& sysEvent);

/**
 * The function to get the absolute file path by the timeStr given.
 *
 * @param timeStr timeStr.
 * @return the absolute path of bakeFile.
 */
std::string GetBackupFilePath(const std::string& timeStr);

/**
 * The function to get a value from the content given by param name.
 *
 * @param content  content.
 * @param param the param.
 * @return the value.
 */
std::string GetParamValueFromString(const std::string& content, const std::string& param);

/**
 * The function to get a value from the file given by param name.
 *
 * @param filePath the path of the file given.
 * @param param the param.
 * @return the value.
 */
std::string GetParamValueFromFile(const std::string& filePath, const std::string& param);

/**
 * The function to save a value to the content given.
 *
 * @param content content.
 * @param param the param.
 * @param value the value.
 * @return newly generated strings with added information.
 */
std::string SetParamValueToString(const std::string& content, const std::string& param, const std::string& value);

/**
 * The function to save a value to the file given.
 *
 * @param filePath the path of the file given.
 * @param param the param.
 * @param value the value.
 * @return
 */
bool SetParamValueToFile(const std::string& filePath, const std::string& param, const std::string& value);

/**
 * The function to compress log files.
 *
 * @param srcPath filePath.
 * @param timeStr time in string format.
 */
void CompressAndCopyLogFiles(const std::string& srcPath, const std::string& timeStr);

/**
 * The function to report panic system event after recovery.
 *
 * @param content content.
 */
void ReportPanicEventAfterRecovery(const std::string& content);

/**
 * Try to report recovery panic event.
 *
 * @return whether the recovery panic event was reported
 */
bool TryToReportRecoveryPanicEvent();

/**
 * Confirm the result of recovery report.
 *
 * @return whether the panic event is reported.
 */
bool ConfirmReportResult();
}
}
}

#endif // PANIC_REPORT_RECOVERY_H_
