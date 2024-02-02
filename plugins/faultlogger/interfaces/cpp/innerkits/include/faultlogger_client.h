/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#ifndef HIVIEW_FAULTLOGGER_CLIENT_INTERFACE_H
#define HIVIEW_FAULTLOGGER_CLIENT_INTERFACE_H
#include <cstdint>
#include <map>
#include <string>
#include "faultlog_query_result.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief  information of fault log
 *
*/
struct FaultLogInfoInner {
    /** the time of happening fault */
    int64_t time {0};
    /** the id of current user when fault happened  */
    int32_t id {-1};
    /** the id of process which fault happened*/
    int32_t pid {-1};
    /** the fd of pipe to transfer json string*/
    int32_t pipeFd {-1};
    /** type of fault */
    int32_t faultLogType {0};
    /** name of module which fault occurred */
    std::string module;
    /** the reason why fault occurred */
    std::string reason;
    /** the summary of fault information */
    std::string summary;
    /** file path of log */
    std::string logPath;
    /** key thread registers */
    std::string registers;
    /** information about faultlog using <key,value> */
    std::map<std::string, std::string> sectionMaps;
};
/**
 * @brief Add fault log
 *
 * The C style of API is used to save fault information to file
 * under the /data/log/fautlog/faultlogger directory and event database
 *
 * @param info  structure containing information about fault
*/
void AddFaultLog(FaultLogInfoInner* info);
/**
 * @brief report cpp crash event to Hiview
 *
 * report cpp crash event to Hiview and save it to event database
 *
 * @param info  structure containing information about fault
*/
void ReportCppCrashEvent(FaultLogInfoInner* info);
#ifdef __cplusplus
}
#endif

namespace OHOS {
namespace HiviewDFX {
/**
 * @brief Distinguish different types of fault
 */
enum FaultLogType {
    /** unspecific fault types */
    NO_SPECIFIC = 0,
    /** C/C++ crash at runtime*/
    CPP_CRASH = 2,
    /** js crash at runtime*/
    JS_CRASH,
    /** application happen freezing */
    APP_FREEZE,
    /** system happen freezing */
    SYS_FREEZE,
    /** rust crash at runtime */
    RUST_PANIC,
};

/**
 * @brief Check faultlogger service status
 *
 * This API is used to check the faultlogger service status
 *
 * @return When faultlogger service is available,it returns true. When not,it rerurns false.
*/
bool CheckFaultloggerStatus();

/**
 * @brief Add fault log
 *
 * This API is used to save fault information to file
 * under the /data/log/fautlog/faultlogger directory and event database
 *
 * @param info  structure containing information about fault
*/

void AddFaultLog(const FaultLogInfoInner &info);

/**
 * @brief Add fault log
 *
 * This API is used to save fault information to file
 * under the /data/log/fautlog/faultlogger directory and event database
 *
 * @param time  the time of happening fault(unix timestamp of Milliseconds)
 * @param logType  the type of fault log.
 * eg: CPP_CRASH,JS_CRASH,APP_FREEZE,SYS_FREEZE,RUST_PANIC
 * @param module name of module which happened fault
 * @param summary the summary of fault information
*/
void AddFaultLog(int64_t time, int32_t logType, const std::string &module, const std::string &summary);

/**
 * @brief query self fault log from event database
 *
 * This API is used to query fault log
 * which belong to current pid and uid from event database
 *
 * @param faultType type of fault log.
 * eg: NO_SPECIFIC,CPP_CRASH,JS_CRASH,APP_FREEZE
 * @param maxNum max number of faultlog entries
 * @return when success query return unique_ptr of FaultLogQueryResult, otherwise return nullptr
*/
std::unique_ptr<FaultLogQueryResult> QuerySelfFaultLog(FaultLogType faultType, int32_t maxNum);
}  // namespace HiviewDFX
}  // namespace OHOS
#endif  // HIVIEW_FAULTLOGGER_CLIENT_INTERFACE_H
