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

#include "panic_report_recovery.h"

#include <regex>
#include <cstdlib>

#include "file_util.h"
#include "hiview_logger.h"
#include "hiview_zip_util.h"
#include "hisysevent.h"
#include "parameters.h"
#include "string_util.h"
#include "tbox.h"

namespace OHOS {
namespace HiviewDFX {
namespace PanicReport {
DEFINE_LOG_LABEL(0xD002D11, "PanicReport");

constexpr const char* BBOX_PARAM_PATH = "/log/bbox/bbox.save.log.flags";
constexpr const char* FACTORY_RECOVERY_TIME_PATH = "/log/bbox/factory.recovery.time";
constexpr const char* CMD_LINE = "/proc/cmdline";
constexpr const char* PANIC_LOG_PATH = "/log/bbox/panic_log/";
constexpr const char* LAST_FASTBOOT_LOG = "/ap_log/last_fastboot_log";
constexpr const char* HM_KLOG = "/ap_log/hm_klog.txt";
constexpr const char* HM_SNAPSHOT = "/ap_log/hm_snapshot.txt";

constexpr const char* BOOTEVENT_BOOT_COMPLETED = "bootevent.boot.completed";
constexpr const char* LAST_BOOTUP_KEYPOINT = "last_bootup_keypoint";

constexpr const char* HAPPEN_TIME = "happentime";
constexpr const char* FACTORY_RECOVERY_TIME = "factoryRecoveryTime";
constexpr const char* IS_PANIC_UN_UPLOADED = "isPanicUnUploaded";
constexpr const char* IS_STARTUP_SHORT = "isStartUpShort";
constexpr const char* IS_LAST_STARTUP_SHORT = "isLastStartUpShort";

constexpr const char* REGEX_FORMAT = R"(=((".*?")|(\S*)))";

constexpr int COMPRESSION_RATION = 9;
constexpr int ZIP_FILE_SIZE_LIMIT = 5 * 1024 * 1024; //5MB

bool InitPanicConfigFile()
{
    if (!FileUtil::FileExists(BBOX_PARAM_PATH)) {
        return FileUtil::CreateFile(BBOX_PARAM_PATH) && SetParamValueToFile(BBOX_PARAM_PATH, IS_STARTUP_SHORT, "true");
    }
    std::string paramsContent;
    FileUtil::LoadStringFromFile(BBOX_PARAM_PATH, paramsContent);
    std::string lastStartUpShort = GetParamValueFromString(paramsContent, IS_STARTUP_SHORT);
    paramsContent = SetParamValueToString(paramsContent, IS_STARTUP_SHORT, "true");
    paramsContent = SetParamValueToString(paramsContent, IS_LAST_STARTUP_SHORT, lastStartUpShort);
    return FileUtil::SaveStringToFile(BBOX_PARAM_PATH, paramsContent);
}

bool InitPanicReport()
{
    auto fileSize = FileUtil::GetFolderSize(PANIC_LOG_PATH);
    if (fileSize > ZIP_FILE_SIZE_LIMIT) {
        HIVIEW_LOGW("zip file size: %{public}" PRIu64 " is over limit", fileSize);
        FileUtil::ForceRemoveDirectory(PANIC_LOG_PATH, false);
    }
    return InitPanicConfigFile() ;
}

bool IsBootCompleted()
{
    return OHOS::system::GetParameter(BOOTEVENT_BOOT_COMPLETED, "false") == "true";
}

bool IsLastShortStartUp()
{
    if (GetParamValueFromFile(BBOX_PARAM_PATH, IS_LAST_STARTUP_SHORT) == "true") {
        return true;
    }
    std::string lastBootUpKeyPoint = GetParamValueFromFile(CMD_LINE, LAST_BOOTUP_KEYPOINT);
    if (lastBootUpKeyPoint.empty()) {
        return false;
    }
    constexpr int normalNum = 250;
    return atoi(lastBootUpKeyPoint.c_str()) < normalNum;
}

bool IsRecoveryPanicEvent(const std::shared_ptr<SysEvent>& sysEvent)
{
    std::string logPath = sysEvent->GetEventValue("LOG_PATH");
    return logPath.find(PANIC_LOG_PATH) == 0;
}

std::string GetLastRecoveryTime()
{
    std::string factoryRecoveryTime;
    FileUtil::LoadStringFromFile(FACTORY_RECOVERY_TIME_PATH, factoryRecoveryTime);
    return std::string("\"") + factoryRecoveryTime + "\"";
}

std::string GetAbsoluteBakeUpFilePathByTimeStr(const std::string& timeStr)
{
    return std::string(PANIC_LOG_PATH) + "panic_log_" + timeStr + ".zip";
}

std::string GetParamValueFromString(const std::string& content, const std::string& param)
{
    std::smatch matchStr;
    if (!std::regex_search(content, matchStr, std::regex(param + REGEX_FORMAT))) {
        return "";
    }
    return matchStr[1].str();
}

std::string GetParamValueFromFile(const std::string& filePath, const std::string& param)
{
    std::string content;
    if (!FileUtil::LoadStringFromFile(filePath, content)) {
        HIVIEW_LOGE("Failed to load file: %{public}s.", filePath.c_str());
        return "";
    }
    return GetParamValueFromString(content, param);
}

std::string SetParamValueToString(const std::string& content, const std::string& param, const std::string& value)
{
    std::string destValue = param + "=" + value;
    if (content.empty()) {
        return destValue;
    }
    if (!std::regex_search(content, std::regex(param + REGEX_FORMAT))) {
        return content + " " + destValue;
    }
    return std::regex_replace(content, std::regex(param + REGEX_FORMAT), destValue);
}

bool SetParamValueToFile(const std::string& filePath, const std::string& param, const std::string& value)
{
    std::string content;
    if (!FileUtil::LoadStringFromFile(filePath, content)) {
        HIVIEW_LOGE("Failed to load file: %{public}s.", filePath.c_str());
        return false;
    }
    return FileUtil::SaveStringToFile(filePath, SetParamValueToString(content, param, value));
}

void AddFileToZip(HiviewZipUnit& hiviewZipUnit, const std::string& path)
{
    if (hiviewZipUnit.AddFileInZip(path, ZipFileLevel::KEEP_NONE_PARENT_PATH) != 0) {
        HIVIEW_LOGW("Failed to add file: %{public}s to zip", path.c_str());
    }
}

bool CompressAndCopyLogFiles(const std::string& srcPath, const std::string& timeStr)
{
    if (!FileUtil::FileExists(PANIC_LOG_PATH)) {
        HIVIEW_LOGE("The path of target file: %{public}s is not existed", PANIC_LOG_PATH);
        return false;
    }
    FileUtil::ForceRemoveDirectory(PANIC_LOG_PATH, false);
    HiviewZipUnit hiviewZipUnit(GetAbsoluteBakeUpFilePathByTimeStr(timeStr));
    std::string lastFastBootLog = srcPath + LAST_FASTBOOT_LOG;
    AddFileToZip(hiviewZipUnit, lastFastBootLog);
    std::string hmKLog = srcPath + HM_KLOG;
    AddFileToZip(hiviewZipUnit, hmKLog);
    std::string hmSnapShot = srcPath + HM_SNAPSHOT;
    uint64_t sourceFileSize = FileUtil::GetFileSize(lastFastBootLog) + FileUtil::GetFileSize(hmKLog) +
        FileUtil::GetFileSize(hmSnapShot);
    if (sourceFileSize < COMPRESSION_RATION * ZIP_FILE_SIZE_LIMIT) {
        AddFileToZip(hiviewZipUnit, hmSnapShot);
    } else {
        HIVIEW_LOGW("sourceFileSize size: %{public}" PRIu64 " is over limit, dropping hmSnapShot.", sourceFileSize);
    }
    uint64_t zipFileSize = FileUtil::GetFolderSize(PANIC_LOG_PATH);
    if (zipFileSize > ZIP_FILE_SIZE_LIMIT) {
        HIVIEW_LOGW("zip file size: %{public}" PRIu64 " is over limit", zipFileSize);
        FileUtil::ForceRemoveDirectory(PANIC_LOG_PATH, false);
    }
    std::string content;
    if (!FileUtil::LoadStringFromFile(BBOX_PARAM_PATH, content)) {
        HIVIEW_LOGE("Failed to load file: %{public}s.", BBOX_PARAM_PATH);
        return false;
    }
    content = SetParamValueToString(content, HAPPEN_TIME, timeStr);
    content = SetParamValueToString(content, FACTORY_RECOVERY_TIME, GetLastRecoveryTime());
    content = SetParamValueToString(content, IS_PANIC_UN_UPLOADED, "true");
    if (!FileUtil::SaveStringToFile(BBOX_PARAM_PATH, content)) {
        HIVIEW_LOGE("Failed to save content to file: %{public}s.", BBOX_PARAM_PATH);
        return false;
    }
    return true;
}

void ReportPanicEventAfterRecovery(const std::string& content)
{
    std::string timeStr = GetParamValueFromString(content, HAPPEN_TIME);
    static constexpr char kernelVendor[] = "KERNEL_VENDOR";
    auto happenTime = Tbox::GetHappenTime(StringUtil::GetRleftSubstr(timeStr, "-"),
                                          "(\\d{4})(\\d{2})(\\d{2})(\\d{2})(\\d{2})(\\d{2})");
    HiSysEventWrite(
        kernelVendor,
        "PANIC",
        HiSysEvent::EventType::FAULT,
        "MODULE", "AP",
        "REASON", "HM_PANIC:HM_PANIC_SYSMGR",
        "LOG_PATH", GetAbsoluteBakeUpFilePathByTimeStr(timeStr),
        "SUB_LOG_PATH", timeStr,
        "HAPPEN_TIME", happenTime,
        "FIRST_FRAME", "RECOVERY_PANIC",
        "FINGERPRINT", Tbox::CalcFingerPrint(timeStr, 0, FP_BUFFER)
    );
}

bool TryToReportRecoveryPanicEvent()
{
    std::string content;
    if (!FileUtil::LoadStringFromFile(BBOX_PARAM_PATH, content)) {
        HIVIEW_LOGE("Failed to load file: %{public}s.", BBOX_PARAM_PATH);
        return false;
    }
    bool ret = false;
    if (GetParamValueFromString(content, IS_PANIC_UN_UPLOADED) == "true" &&
        GetParamValueFromString(content, FACTORY_RECOVERY_TIME) != GetLastRecoveryTime()) {
        ReportPanicEventAfterRecovery(content);
        ret = true;
    }
    content = SetParamValueToString(content, IS_STARTUP_SHORT, "false");
    if (!FileUtil::SaveStringToFile(BBOX_PARAM_PATH, content)) {
        HIVIEW_LOGE("Failed to save content to file: %{public}s.", BBOX_PARAM_PATH);
    }
    return ret;
}

void NotifyReportFinished()
{
    SetParamValueToFile(BBOX_PARAM_PATH, IS_PANIC_UN_UPLOADED, "false");
}
}
}
}