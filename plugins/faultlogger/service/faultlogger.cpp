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
#include "faultlogger.h"

#include <climits>
#include <cstdint>
#include <ctime>
#include <memory>
#include <regex>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <cerrno>
#include <thread>
#include <unistd.h>

#include "accesstoken_kit.h"
#include "asan_collector.h"
#include "bundle_mgr_client.h"
#include "common_utils.h"
#include "constants.h"
#include "event.h"
#include "event_publish.h"
#include "faultlog_formatter.h"
#include "faultlog_info.h"
#include "faultlog_query_result_inner.h"
#include "faultlog_util.h"
#include "faultlogger_adapter.h"
#include "file_util.h"
#include "hisysevent.h"
#include "hiview_global.h"
#include "ipc_skeleton.h"
#include "json/json.h"
#include "log_analyzer.h"
#include "logger.h"
#include "parameter_ex.h"
#include "plugin_factory.h"
#include "process_status.h"
#include "reporter.h"
#include "sanitizerd_collector.h"
#include "sanitizerd_monitor.h"
#include "securec.h"
#include "string_util.h"
#include "sys_event_dao.h"
#include "time_util.h"
#include "zip_helper.h"

namespace OHOS {
namespace HiviewDFX {
REGISTER(Faultlogger);
DEFINE_LOG_TAG("Faultlogger");
using namespace FaultLogger;
using namespace OHOS::AppExecFwk;
#ifndef UNIT_TEST
static std::unordered_map<std::string, std::string> g_stacks;
static AsanCollector g_collector(g_stacks);
#endif
namespace {
constexpr char FILE_SEPERATOR[] = "******";
constexpr uint32_t DUMP_MAX_NUM = 100;
constexpr int32_t MAX_QUERY_NUM = 100;
constexpr int MIN_APP_UID = 10000;
constexpr int DUMP_PARSE_CMD = 0;
constexpr int DUMP_PARSE_FILE_NAME = 1;
constexpr int DUMP_PARSE_TIME = 2;
constexpr int DUMP_START_PARSE_MODULE_NAME = 3;
constexpr uint32_t MAX_NAME_LENGTH = 4096;
constexpr char TEMP_LOG_PATH[] = "/data/log/faultlog/temp";
constexpr time_t FORTYEIGHT_HOURS = 48 * 60 * 60;
#ifndef UNIT_TEST
constexpr int RETRY_COUNT = 10;
constexpr int RETRY_DELAY = 10;
#endif
constexpr int READ_HILOG_BUFFER_SIZE = 1024;
constexpr char APP_CRASH_TYPE[] = "APP_CRASH";
DumpRequest InitDumpRequest()
{
    DumpRequest request;
    request.requestDetail = false;
    request.requestList = false;
    request.fileName = "";
    request.moduleName = "";
    request.time = -1;
    return request;
}

bool IsLogNameValid(const std::string& name)
{
    if (name.empty() || name.size() > MAX_NAME_LENGTH) {
        HIVIEW_LOGI("invalid log name.");
        return false;
    }

    std::vector<std::string> out;
    StringUtil::SplitStr(name, "-", out, true, false);
    if (out.size() != 4) { // FileName LogType-ModuleName-uid-YYYYMMDDHHMMSS, thus contains 4 sections
        return false;
    }

    std::regex reType("^[a-z]+$");
    if (!std::regex_match(out[0], reType)) { // 0 : type section
        HIVIEW_LOGI("invalid type.");
        return false;
    }

    if (!IsModuleNameValid(out[1])) { // 1 : module section
        HIVIEW_LOGI("invalid module name.");
        return false;
    }

    std::regex reDigits("^[0-9]*$");
    if (!std::regex_match(out[2], reDigits)) { // 2 : uid section
        HIVIEW_LOGI("invalid uid.");
        return false;
    }

    if (!std::regex_match(out[3], reDigits)) { // 3 : time section
        HIVIEW_LOGI("invalid digits.");
        return false;
    }
    return true;
}

bool FillDumpRequest(DumpRequest &request, int status, const std::string &item)
{
    switch (status) {
        case DUMP_PARSE_FILE_NAME:
            if (!IsLogNameValid(item)) {
                return false;
            }
            request.fileName = item;
            break;
        case DUMP_PARSE_TIME:
            if (item.size() == 14) { // 14 : BCD time size
                request.time = TimeUtil::StrToTimeStamp(item, "%Y%m%d%H%M%S");
            } else {
                StringUtil::ConvertStringTo<time_t>(item, request.time);
            }
            break;
        case DUMP_START_PARSE_MODULE_NAME:
            if (!IsModuleNameValid(item)) {
                return false;
            }
            request.moduleName = item;
            break;
        default:
            HIVIEW_LOGI("Unknown status.");
            break;
    }
    return true;
}

std::string GetSummaryFromSectionMap(int32_t type, const std::map<std::string, std::string>& maps)
{
    std::string key = "";
    switch (type) {
        case CPP_CRASH:
            key = "KEY_THREAD_INFO";
            break;
        default:
            break;
    }

    if (key.empty()) {
        return "";
    }

    auto value = maps.find(key);
    if (value == maps.end()) {
        return "";
    }
    return value->second;
}
} // namespace

void Faultlogger::AddPublicInfo(FaultLogInfo &info)
{
    info.sectionMap["DEVICE_INFO"] = Parameter::GetString("const.product.name", "Unknown");
    info.sectionMap["BUILD_INFO"] = Parameter::GetString("const.product.software.version", "Unknown");
    info.sectionMap["UID"] = std::to_string(info.id);
    info.sectionMap["PID"] = std::to_string(info.pid);
    info.module = RegulateModuleNameIfNeed(info.module);
    info.sectionMap["MODULE"] = info.module;
    DfxBundleInfo bundleInfo;
    if (info.id >= MIN_APP_USERID && GetDfxBundleInfo(info.module, bundleInfo)) {
        if (!bundleInfo.versionName.empty()) {
            info.sectionMap["VERSION"] = bundleInfo.versionName;
            info.sectionMap["VERSION_CODE"] = std::to_string(bundleInfo.versionCode);
        }

        if (bundleInfo.isPreInstalled) {
            info.sectionMap["PRE_INSTALL"] = "Yes";
        } else {
            info.sectionMap["PRE_INSTALL"] = "No";
        }
    }

    if (info.id >= MIN_APP_USERID) {
        if (UCollectUtil::ProcessStatus::GetInstance().GetProcessState(info.pid) ==
            UCollectUtil::FOREGROUND) {
            info.sectionMap["FOREGROUND"] = "Yes";
        } else if (UCollectUtil::ProcessStatus::GetInstance().GetProcessState(info.pid) ==
            UCollectUtil::BACKGROUND){
            int64_t lastFgTime = static_cast<int64_t>(UCollectUtil::ProcessStatus::GetInstance()
                .GetProcessLastForegroundTime(info.pid));
            if (lastFgTime > info.time) {
                info.sectionMap["FOREGROUND"] = "Yes";
            } else {
                info.sectionMap["FOREGROUND"] = "No";
            }
        }
    }

    if (info.reason.empty()) {
        info.reason = info.sectionMap["REASON"];
    } else {
        info.sectionMap["REASON"] = info.reason;
    }

    if (info.summary.empty()) {
        info.summary = GetSummaryFromSectionMap(info.faultLogType, info.sectionMap);
    } else {
        info.sectionMap["SUMMARY"] = info.summary;
    }

    // parse fingerprint by summary or temp log for native crash
    AnalysisFaultlog(info, info.parsedLogInfo);
    info.sectionMap.insert(info.parsedLogInfo.begin(), info.parsedLogInfo.end());
}

void Faultlogger::AddCppCrashInfo(FaultLogInfo& info)
{
    if (!info.registers.empty()) {
        info.sectionMap["KEY_THREAD_REGISTERS"] = info.registers;
    }

    if (!info.otherThreadInfo.empty()) {
        info.sectionMap["OTHER_THREAD_INFO"] = info.otherThreadInfo;
    }

    info.sectionMap["APPEND_ORIGIN_LOG"] = GetCppCrashTempLogName(info);
}

bool Faultlogger::VerifiedDumpPermission()
{
    using namespace Security::AccessToken;
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (AccessTokenKit::VerifyAccessToken(tokenId, "ohos.permission.DUMP") != PermissionState::PERMISSION_GRANTED) {
        return false;
    }
    return true;
}

void Faultlogger::Dump(int fd, const std::vector<std::string> &cmds)
{
    auto request = InitDumpRequest();
    int32_t status = DUMP_PARSE_CMD;
    for (auto it = cmds.begin(); it != cmds.end(); it++) {
        if ((*it) == "-f") {
            status = DUMP_PARSE_FILE_NAME;
            continue;
        } else if ((*it) == "-l") {
            request.requestList = true;
            continue;
        } else if ((*it) == "-t") {
            status = DUMP_PARSE_TIME;
            continue;
        } else if ((*it) == "-m") {
            status = DUMP_START_PARSE_MODULE_NAME;
            continue;
        } else if ((*it) == "-d") {
            request.requestDetail = true;
            continue;
        } else if ((*it) == "Faultlogger") {
            // skip first params
            continue;
        } else if ((!(*it).empty()) && ((*it).at(0) == '-')) {
            dprintf(fd, "Unknown command.\n");
            return;
        }

        if (!FillDumpRequest(request, status, *it)) {
            dprintf(fd, "invalid parameters.\n");
            return;
        }
        status = DUMP_PARSE_CMD;
    }

    if (status != DUMP_PARSE_CMD) {
        dprintf(fd, "empty parameters.\n");
        return;
    }

    HIVIEW_LOGI("DumpRequest: detail:%d, list:%d, file:%s, name:%s, time:%ld",
                request.requestDetail, request.requestList, request.fileName.c_str(), request.moduleName.c_str(),
                request.time);
    Dump(fd, request);
}

void Faultlogger::Dump(int fd, const DumpRequest &request) const
{
    if (!request.fileName.empty()) {
        std::string content;
        if (mgr_->GetFaultLogContent(request.fileName, content)) {
            dprintf(fd, "%s\n", content.c_str());
        } else {
            dprintf(fd, "Fail to dump the log.\n");
        }
        return;
    }

    auto fileList = mgr_->GetFaultLogFileList(request.moduleName, request.time, -1, 0, DUMP_MAX_NUM);
    if (fileList.empty()) {
        dprintf(fd, "No fault log exist.\n");
        return;
    }

    dprintf(fd, "Fault log list:\n");
    dprintf(fd, "%s\n", FILE_SEPERATOR);
    for (auto &file : fileList) {
        std::string fileName = FileUtil::ExtractFileName(file);
        dprintf(fd, "%s\n", fileName.c_str());
        if (request.requestDetail) {
            std::string content;
            if (FileUtil::LoadStringFromFile(file, content)) {
                dprintf(fd, "%s\n", content.c_str());
            } else {
                dprintf(fd, "Fail to dump detail log.\n");
            }
            dprintf(fd, "%s\n", FILE_SEPERATOR);
        }
    }
    dprintf(fd, "%s\n", FILE_SEPERATOR);
}

bool Faultlogger::JudgmentRateLimiting(std::shared_ptr<Event> event)
{
    int interval = 60; // 60s time interval
    auto sysEvent = std::static_pointer_cast<SysEvent>(event);
    long pid = sysEvent->GetPid();
    std::string eventPid = std::to_string(pid);

    std::time_t now = std::time(0);
    for (auto it = eventTagTime_.begin(); it != eventTagTime_.end();) {
        if ((now - it->second) >= interval) {
            it = eventTagTime_.erase(it);
            continue;
        }
        ++it;
    }

    auto it = eventTagTime_.find(eventPid);
    if (it != eventTagTime_.end()) {
        if ((now - it->second) < interval) {
            HIVIEW_LOGW("event: id:0x%{public}d, eventName:%{public}s pid:%{public}s. \
                interval:%{public}ld There's not enough interval",
                sysEvent->eventId_, sysEvent->eventName_.c_str(), eventPid.c_str(), interval);
            return false;
        }
    }
    eventTagTime_[eventPid] = now;
    HIVIEW_LOGI("event: id:0x%{public}d, eventName:%{public}s pid:%{public}s. \
        interval:%{public}ld normal interval",
        sysEvent->eventId_, sysEvent->eventName_.c_str(), eventPid.c_str(), interval);
    return true;
}

bool Faultlogger::IsInterestedPipelineEvent(std::shared_ptr<Event> event)
{
    if (!hasInit_ || event == nullptr) {
        return false;
    }

    if (event->eventName_ != "PROCESS_EXIT" &&
        event->eventName_ != "JS_ERROR" &&
        event->eventName_ != "RUST_PANIC") {
        return false;
    }

    return true;
}

bool Faultlogger::OnEvent(std::shared_ptr<Event> &event)
{
    if (!hasInit_ || event == nullptr) {
        return false;
    }
    if (event->eventName_ != "JS_ERROR" && event->eventName_ != "RUST_PANIC") {
        return true;
    }
    if (event->rawData_ == nullptr) {
        return false;
    }
    bool isJsError = event->eventName_ == "JS_ERROR";
    auto sysEvent = std::static_pointer_cast<SysEvent>(event);
    HIVIEW_LOGI("Receive %{public}s Event:%{public}s.", event->eventName_.c_str(),
        sysEvent->AsJsonStr().c_str());
    FaultLogInfo info;
    info.time = sysEvent->happenTime_;
    info.id = sysEvent->GetUid();
    info.pid = sysEvent->GetPid();
    info.faultLogType = isJsError ? FaultLogType::JS_CRASH : FaultLogType::RUST_PANIC;
    info.module = isJsError ? sysEvent->GetEventValue("PACKAGE_NAME") : sysEvent->GetEventValue("MODULE");
    info.reason = sysEvent->GetEventValue("REASON");
    auto summary = sysEvent->GetEventValue("SUMMARY");
    info.summary = StringUtil::UnescapeJsonStringValue(summary);
    info.sectionMap = sysEvent->GetKeyValuePairs();
    AddFaultLog(info);
    sysEvent->SetEventValue("FAULT_TYPE", std::to_string(info.faultLogType));
    sysEvent->SetEventValue("MODULE", info.module);
    sysEvent->SetEventValue("LOG_PATH", info.logPath);
    sysEvent->SetEventValue("HAPPEN_TIME", sysEvent->happenTime_);
    sysEvent->SetEventValue("tz_", TimeUtil::GetTimeZone());
    sysEvent->SetEventValue("VERSION", info.sectionMap["VERSION"]);
    sysEvent->SetEventValue("PRE_INSTALL", info.sectionMap["PRE_INSTALL"]);
    sysEvent->SetEventValue("FOREGROUND", info.sectionMap["FOREGROUND"]);
    std::map<std::string, std::string> eventInfos;
    if (AnalysisFaultlog(info, eventInfos)) {
        sysEvent->SetEventValue("PNAME", eventInfos["PNAME"].empty() ? "/" : eventInfos["PNAME"]);
        sysEvent->SetEventValue("FIRST_FRAME", eventInfos["FIRST_FRAME"].empty() ? "/" :
                                StringUtil::EscapeJsonStringValue(eventInfos["FIRST_FRAME"]));
        sysEvent->SetEventValue("SECOND_FRAME", eventInfos["SECOND_FRAME"].empty() ? "/" :
                                StringUtil::EscapeJsonStringValue(eventInfos["SECOND_FRAME"]));
        sysEvent->SetEventValue("LAST_FRAME", eventInfos["LAST_FRAME"].empty() ? "/" :
                                StringUtil::EscapeJsonStringValue(eventInfos["LAST_FRAME"]));
    }
    sysEvent->SetEventValue("FINGERPRINT", eventInfos["fingerPrint"]);
    if (isJsError) {
        ReportJsErrorToAppEvent(sysEvent);
    }
    return true;
}

bool Faultlogger::CanProcessEvent(std::shared_ptr<Event> event)
{
    return true;
}

void Faultlogger::ReportJsErrorToAppEvent(std::shared_ptr<SysEvent> sysEvent) const
{
    std::regex rel("[\\s\\S]*Error message:([\\s\\S]*)SourceCode[\\s\\S]*Stacktrace:([\\s\\S]*)[\\s\\S]*");
    std::smatch m;
    std::string summary = sysEvent->GetEventValue("SUMMARY");
    HIVIEW_LOGD("ReportAppEvent:summary:%{public}s.", summary.c_str());
    std::regex_search(summary, m, rel);

    Json::Value params;
    params["time"] = sysEvent->happenTime_;
    params["crash_type"] = "JsError";
    params["foreground"] = sysEvent->GetEventValue("FOREGROUND");
    params["bundle_version"] = sysEvent->GetEventValue("VERSION");
    params["bundle_name"] = sysEvent->GetEventValue("PACKAGE_NAME");
    params["pid"] = sysEvent->GetPid();
    params["uid"] = sysEvent->GetUid();
    params["uuid"] = sysEvent->GetEventValue("FINGERPRINT");
    Json::Value exception;
    exception["name"] = sysEvent->GetEventValue("REASON");
    std::string message = m[1]; // 1: is message
    std::string stack = m[2]; // 2: is stack
    message.replace(message.end() - 2, message.end(), ""); // 2: to relace tail '\n' to ""
    stack.replace(stack.begin(), stack.begin() + 6, ""); // 6: to relace head '\n     ' to ""
    stack.replace(stack.end() - 2, stack.end(), ""); // 2: to relace tail '\n' to ""
    exception["message"] = message;
    exception["stack"] = stack;
    params["exception"] = exception;

    // add hilog
    std::string log;
    GetHilog(sysEvent->GetPid(), log);
    if (log.length() == 0) {
        HIVIEW_LOGE("Get hilog is empty");
    } else {
        Json::Value hilog;
        std::stringstream logStream(log);
        std::string oneLine;
        while (getline(logStream, oneLine)) {
            hilog.append(oneLine);
        }
        params["hilog"] = hilog;
    }

    std::string paramsStr = Json::FastWriter().write(params);
    HIVIEW_LOGD("ReportAppEvent: uid:%{public}d, json:%{public}s.",
        sysEvent->GetUid(), paramsStr.c_str());
    EventPublish::GetInstance().PushEvent(sysEvent->GetUid(), APP_CRASH_TYPE, HiSysEvent::EventType::FAULT, paramsStr);
}

bool Faultlogger::ReadyToLoad()
{
    return true;
}

void Faultlogger::OnLoad()
{
    mgr_ = std::make_unique<FaultLogManager>(GetHiviewContext()->GetSharedWorkLoop());
    mgr_->Init();
    hasInit_ = true;
#ifndef UNITTEST
    FaultloggerAdapter::StartService(this);
#endif

    // some crash happened before hiview start, ensure every crash event is added into eventdb
    auto eventloop = GetHiviewContext()->GetSharedWorkLoop();
    if (eventloop != nullptr) {
        auto task = std::bind(&Faultlogger::StartBootScan, this);
        eventloop->AddTimerEvent(nullptr, nullptr, task, 10, false); // delay 10 seconds
        workLoop_ = eventloop;
    }
#ifndef UNIT_TEST
    std::thread sanitizerdThread(&Faultlogger::RunSanitizerd);
    pthread_setname_np(sanitizerdThread.native_handle(), "RunSanitizerd");
    sanitizerdThread.detach();
#endif
}

#ifndef UNIT_TEST
void Faultlogger::HandleNotify(int32_t type, const std::string& fname)
{
    HIVIEW_LOGE("HandleNotify file:[%{public}s]\n", fname.c_str());
    // start sanitizerd work thread if log ready
    std::thread collector(&AsanCollector::Collect, &g_collector, fname);
    collector.detach();
    // Work done.
}

int Faultlogger::RunSanitizerd()
{
    SanitizerdMonitor sanMonitor;

    // Init the monitor first.
    bool ready = false;
    int rcount = RETRY_COUNT;
    for (; rcount > 0; rcount--) {
        if (sanMonitor.Init(&Faultlogger::HandleNotify) != 0) {
            sleep(RETRY_DELAY); // 10s
            continue;
        } else {
            ready = true;
            break;
        }
    }

    if (!ready) {
        HIVIEW_LOGE("sanitizerd: failed to initialize monitor");
        return -1;
    }

    // Waits on notify callback and resume the collector.
    HIVIEW_LOGE("sanitizerd: starting\n");

    // Waiting for notify event.
    while (true) {
        std::string rfile;
        // Let the monitor check if log ready.
        if (sanMonitor.RunMonitor(&rfile, -1) == 0) {
            HIVIEW_LOGE("sanitizerd ready file:[%s]\n", rfile.c_str());
        } else {
            break;
        }
    }

    sanMonitor.Uninit();
    return 0;
}
#endif

void Faultlogger::AddFaultLog(FaultLogInfo& info)
{
    if (!hasInit_) {
        return;
    }

    AddFaultLogIfNeed(info, nullptr);
}

std::unique_ptr<FaultLogInfo> Faultlogger::GetFaultLogInfo(const std::string &logPath)
{
    if (!hasInit_) {
        return nullptr;
    }

    auto info = std::make_unique<FaultLogInfo>(FaultLogger::ParseFaultLogInfoFromFile(logPath));
    info->logPath = logPath;
    return info;
}

std::unique_ptr<FaultLogQueryResultInner> Faultlogger::QuerySelfFaultLog(int32_t id,
    int32_t pid, int32_t faultType, int32_t maxNum)
{
    if (!hasInit_) {
        return nullptr;
    }

    if ((faultType < FaultLogType::ALL) || (faultType > FaultLogType::RUST_PANIC)) {
        HIVIEW_LOGW("Unsupported fault type");
        return nullptr;
    }

    if (maxNum < 0 || maxNum > MAX_QUERY_NUM) {
        maxNum = MAX_QUERY_NUM;
    }

    std::string name = "";
    if (id >= MIN_APP_UID) {
        name = GetApplicationNameById(id);
    }

    if (name.empty()) {
        name = CommonUtils::GetProcNameByPid(pid);
    }
    return std::make_unique<FaultLogQueryResultInner>(mgr_->GetFaultInfoList(name, id, faultType, maxNum));
}

void Faultlogger::AddFaultLogIfNeed(FaultLogInfo& info, std::shared_ptr<Event> event)
{
    if ((info.faultLogType <= FaultLogType::ALL) || (info.faultLogType > FaultLogType::RUST_PANIC)) {
        HIVIEW_LOGW("Unsupported fault type");
        return;
    }
    HIVIEW_LOGI("Start saving Faultlog of Process:%{public}d, Name:%{public}s, Reason:%{public}s.",
        info.pid, info.module.c_str(), info.reason.c_str());

    std::string appName = GetApplicationNameById(info.id);
    if (!appName.empty()) {
        info.module = appName;
    }

    HIVIEW_LOGD("nameProc %{public}s", info.module.c_str());
    if ((info.module.empty()) || (!IsModuleNameValid(info.module))) {
        HIVIEW_LOGW("Invalid module name %{public}s", info.module.c_str());
        return;
    }
    AddPublicInfo(info);
    if (info.faultLogType == FaultLogType::CPP_CRASH) {
        AddCppCrashInfo(info);
        ReportCppCrashToAppEvent(info);
    }

    mgr_->SaveFaultLogToFile(info);
    if (info.faultLogType != FaultLogType::JS_CRASH && info.faultLogType != FaultLogType::RUST_PANIC) {
        mgr_->SaveFaultInfoToRawDb(info);
    }
    HIVIEW_LOGI("\nSave Faultlog of Process:%{public}d\n"
                "ModuleName:%{public}s\n"
                "Reason:%{public}s\n"
                "Summary:%{public}s\n",
                info.pid,
                info.module.c_str(),
                info.reason.c_str(),
                info.summary.c_str());
}

void Faultlogger::OnUnorderedEvent(const Event &msg)
{
}

std::string Faultlogger::GetListenerName()
{
    return "FaultLogger";
}

void Faultlogger::StartBootScan()
{
    std::vector<std::string> files;
    time_t now = time(nullptr);
    FileUtil::GetDirFiles(TEMP_LOG_PATH, files);
    for (const auto& file : files) {
        time_t lastAccessTime = GetFileLastAccessTimeStamp(file);
        if (now > lastAccessTime && now - lastAccessTime > FORTYEIGHT_HOURS) {
            HIVIEW_LOGI("Skip this file(%{public}s) that were created 48 hours ago.", file.c_str());
            continue;
        }
        auto info = ParseFaultLogInfoFromFile(file, true);
        if (info.summary.find("#00") == std::string::npos) {
            HIVIEW_LOGI("Skip this file(%{public}s) which stack is empty.", file.c_str());
            HiSysEventWrite(HiSysEvent::Domain::RELIABILITY, "CPP_CRASH_NO_LOG", HiSysEvent::EventType::FAULT,
                "PID", info.pid,
                "UID", info.id,
                "PROCESS_NAME", info.module,
                "HAPPEN_TIME", std::to_string(info.time)
            );
            if (remove(file.c_str()) != 0) {
                HIVIEW_LOGE("Failed to remove file(%{public}s) which stack is empty", file.c_str());
            }
            continue;
        }
        if (mgr_->IsProcessedFault(info.pid, info.id, info.faultLogType)) {
            HIVIEW_LOGI("Skip processed fault.(%{public}d:%{public}d) ", info.pid, info.id);
            continue;
        }
        AddFaultLogIfNeed(info, nullptr);
    }
}

void Faultlogger::ReportCppCrashToAppEvent(const FaultLogInfo& info) const
{
    std::string stackInfo;
    GetStackInfo(info, stackInfo);
    if (stackInfo.length() == 0) {
        HIVIEW_LOGE("stackInfo is empty");
        return;
    }
    HIVIEW_LOGI("report cppcrash to appevent, pid:%{public}d len:%{public}d", info.pid, stackInfo.length());
    EventPublish::GetInstance().PushEvent(info.id, APP_CRASH_TYPE, HiSysEvent::EventType::FAULT, stackInfo);
}

void Faultlogger::GetStackInfo(const FaultLogInfo& info, std::string& stackInfo) const
{
    if (info.sectionMap.count("stackInfo") == 0) {
        HIVIEW_LOGE("stackInfo is not exist");
        return;
    }
    std::string stackInfoOriginal = info.sectionMap.at("stackInfo");
    if (stackInfoOriginal.length() == 0) {
        HIVIEW_LOGE("stackInfo original is empty");
        return;
    }

    Json::Reader reader;
    Json::Value stackInfoObj;
    if (!reader.parse(stackInfoOriginal, stackInfoObj)) {
        HIVIEW_LOGE("parse stackInfo failed");
        return;
    }

    stackInfoObj["bundle_name"] = info.module;
    if (info.sectionMap.count("VERSION") == 1) {
        stackInfoObj["bundle_version"] = info.sectionMap.at("VERSION");
    }
    if (info.sectionMap.count("FOREGROUND") == 1) {
        stackInfoObj["foreground"] = info.sectionMap.at("FOREGROUND");
    }
    if (info.sectionMap.count("FINGERPRINT") == 1) {
        stackInfoObj["uuid"] = info.sectionMap.at("FINGERPRINT");
    }

    std::string log;
    GetHilog(info.pid, log);
    if (log.length() == 0) {
        HIVIEW_LOGE("Get hilog is empty");
        return;
    }
    Json::Value hilog;
    std::stringstream logStream(log);
    std::string oneLine;
    while (getline(logStream, oneLine)) {
        hilog.append(oneLine);
    }
    stackInfoObj["hilog"] = hilog;
    stackInfo.append(Json::FastWriter().write(stackInfoObj));
}

void Faultlogger::DoGetHilogProcess(int32_t pid, int writeFd) const
{
    HIVIEW_LOGD("Start do get hilog process, pid:%{public}d", pid);
    if (writeFd < 0 || dup2(writeFd, STDOUT_FILENO) == -1 ||
        dup2(writeFd, STDERR_FILENO) == -1) {
        HIVIEW_LOGE("dup2 writeFd fail");
        _exit(-1);
    }

    int ret = -1;
    ret = execl("/system/bin/hilog", "hilog", "-z", "100", "-P", std::to_string(pid).c_str(), nullptr);
    if (ret < 0) {
        HIVIEW_LOGE("execl %{public}d, errno: %{public}d", ret, errno);
        syscall(SYS_close, writeFd);
        _exit(-1);
    }
}

bool Faultlogger::GetHilog(int32_t pid, std::string& log) const
{
    int fds[2] = {-1, -1}; // 2: one read pipe, one write pipe
    if (pipe(fds) != 0) {
        HIVIEW_LOGE("Failed to create pipe for get log.");
        return false;
    }
    int childPid = fork();
    if (childPid < 0) {
        HIVIEW_LOGE("fork fail");
        return false;
    } else if (childPid == 0) {
        syscall(SYS_close, fds[0]);
        DoGetHilogProcess(pid, fds[1]);
    } else {
        syscall(SYS_close, fds[1]);

        // read log from fds[0]
        char buffer[READ_HILOG_BUFFER_SIZE] = {0};
        while (true) {
            (void)memset_s(buffer, sizeof(buffer), 0, sizeof(buffer));
            ssize_t nread = read(fds[0], buffer, sizeof(buffer) - 1);
            if (nread <= 0) {
                HIVIEW_LOGI("read hilog finished");
                break;
            }
            log.append(buffer);
        }
        syscall(SYS_close, fds[0]);

        if (waitpid(childPid, nullptr, 0) != childPid) {
            HIVIEW_LOGE("waitpid fail, pid: %{public}d, errno: %{public}d", childPid, errno);
            return false;
        }
        HIVIEW_LOGI("get hilog waitpid %{public}d success", childPid);
        return true;
    }
    return false;
}
} // namespace HiviewDFX
} // namespace OHOS

