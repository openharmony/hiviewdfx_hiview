/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <sys/types.h>

#include <cerrno>
#include <thread>
#include <unistd.h>

#include "common_utils.h"
#include "constants.h"
#include "event.h"
#include "file_util.h"
#include "hiview_global.h"
#include "logger.h"
#include "parameter_ex.h"
#include "plugin_factory.h"
#include "string_util.h"
#include "sys_event_dao.h"
#include "sys_event.h"
#include "time_util.h"

#include "faultlog_formatter.h"
#include "faultlog_info.h"
#include "faultlog_query_result_inner.h"
#include "faultlog_util.h"
#include "faultlogger_adapter.h"
#include "log_analyzer.h"

#include "bundle_mgr_client.h"

#include "securec.h"
#include "sanitizerd_log.h"
#include "asan_collector.h"
#include "sanitizerd_collector.h"
#include "sanitizerd_monitor.h"
#include "reporter.h"
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
#ifndef UNIT_TEST
constexpr int RETRY_COUNT = 10;
constexpr int RETRY_DELAY = 10;
#endif
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
    std::string version = GetApplicationVersion(info.id, info.module);
    if (!version.empty()) {
        info.sectionMap["VERSION"] = version;
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
        event->eventName_ != "JS_ERROR") {
        return false;
    }

    return true;
}

bool Faultlogger::OnEvent(std::shared_ptr<Event> &event)
{
    if (!hasInit_ || event == nullptr) {
        return false;
    }

    if (event->eventName_ == "JS_ERROR") {
        if (event->jsonExtraInfo_.empty()) {
            return false;
        }

        HIVIEW_LOGI("Receive JS_ERROR Event:%{public}s.", event->jsonExtraInfo_.c_str());
        FaultLogInfo info;
        auto sysEvent = std::static_pointer_cast<SysEvent>(event);
        info.time = sysEvent->happenTime_;
        info.id = sysEvent->GetUid();
        info.pid = sysEvent->GetPid();
        info.faultLogType = FaultLogType::JS_CRASH;
        info.module = sysEvent->GetEventValue("PACKAGE_NAME");
        info.reason = sysEvent->GetEventValue("REASON");
        auto summary = sysEvent->GetEventValue("SUMMARY");
        info.summary = StringUtil::UnescapeJsonStringValue(summary);
        info.sectionMap = sysEvent->GetKeyValuePairs();
        AddFaultLog(info);

        auto eventQuery = EventStore::SysEventDao::BuildQuery(event->what_);
        std::vector<std::string> selections { EventStore::EventCol::TS };
        EventStore::ResultSet set = (*eventQuery).Select(selections)
            .Where(EventStore::EventCol::TS, EventStore::Op::EQ, static_cast<int64_t>(event->happenTime_))
            .And(EventStore::EventCol::DOMAIN, EventStore::Op::EQ, sysEvent->domain_)
            .And(EventStore::EventCol::NAME, EventStore::Op::EQ, sysEvent->eventName_)
            .Execute();
        if (set.GetErrCode() != 0) {
            HIVIEW_LOGE("failed to get db, error:%{public}d.", set.GetErrCode());
            return false;
        }
        if (set.HasNext()) {
            auto record = set.Next();
            if (record->GetSeq() == sysEvent->GetSeq()) {
                HIVIEW_LOGI("Seq match success, info.logPath %{public}s", info.logPath.c_str());
                sysEvent->SetEventValue("FAULT_TYPE", std::to_string(info.faultLogType));
                sysEvent->SetEventValue("MODULE", info.module);
                sysEvent->SetEventValue("LOG_PATH", info.logPath);
                sysEvent->SetEventValue("HAPPEN_TIME", sysEvent->happenTime_);
                sysEvent->SetEventValue("tz_", TimeUtil::GetTimeZone());
                sysEvent->SetEventValue("VERSION", info.sectionMap["VERSION"]);

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
                auto retCode = EventStore::SysEventDao::Update(sysEvent, false);
                if (retCode == 0) {
                    return true;
                }
            }
        }

        HIVIEW_LOGE("eventLog LogPath update to DB failed!");
        return false;
    }
    return true;
}

bool Faultlogger::CanProcessEvent(std::shared_ptr<Event> event)
{
    return true;
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

    if ((faultType < FaultLogType::ALL) || (faultType > FaultLogType::SYS_FREEZE)) {
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
    if ((info.faultLogType <= FaultLogType::ALL) || (info.faultLogType > FaultLogType::SYS_FREEZE)) {
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
    mgr_->SaveFaultLogToFile(info);
    if (info.faultLogType != FaultLogType::JS_CRASH) {
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
    FileUtil::GetDirFiles(TEMP_LOG_PATH, files);
    for (const auto& file : files) {
        auto info = ParseFaultLogInfoFromFile(file, true);
        if (mgr_->IsProcessedFault(info.pid, info.id, info.faultLogType)) {
            HIVIEW_LOGI("Skip processed fault.(%{public}d:%{public}d) ", info.pid, info.id);
            continue;
        }
        AddFaultLogIfNeed(info, nullptr);
    }
}
} // namespace HiviewDFX
} // namespace OHOS

