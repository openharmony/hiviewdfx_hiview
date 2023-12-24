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

#include "trace_decorator.h"

#include "file_util.h"
#include "trace_collector_impl.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
const std::string TRACE_COLLECTOR_NAME = "TraceCollector";
const std::string UC_HITRACE_API_STAT_TITLE = "Hitrace API detail statistics:";
const std::string UC_HITRACE_API_STAT_ITEM =
    "Caller FailCall OverCall TotalCall AvgLatency(us) MaxLatency(us) TotalTimeSpent(us)";
const std::string UC_HITRACE_COMPRESS_RATIO = "Hitrace Traffic Compress Ratio:";
const std::string UC_HITRACE_TRAFFIC_STAT_TITLE = "Hitrace Traffic statistics:";
const std::string UC_HITRACE_TRAFFIC_STAT_ITEM =
    "Caller TraceFile TimeSpent(us) RawSize(b) UsedSize(b) TimeStamp(us)";

StatInfoWrapper TraceDecorator::statInfoWrapper_;
TraceStatWrapper TraceDecorator::traceStatWrapper_;

std::shared_ptr<TraceCollector> TraceCollector::Create()
{
    std::shared_ptr<TraceDecorator> instance = std::make_shared<TraceDecorator>();
    return instance;
}

TraceDecorator::TraceDecorator()
{
    traceCollector_ = std::make_shared<TraceCollectorImpl>();
}

CollectResult<std::vector<std::string>> TraceDecorator::DumpTrace(TraceCollector::Caller &caller)
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::vector<std::string>> result = traceCollector_->DumpTrace(caller);
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    traceStatWrapper_.UpdateTraceStatInfo(startTime, endTime, caller, result);
    return result;
}

CollectResult<int32_t> TraceDecorator::TraceOn()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<int32_t> result = traceCollector_->TraceOn();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = TRACE_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

CollectResult<std::vector<std::string>> TraceDecorator::TraceOff()
{
    uint64_t startTime = TimeUtil::GenerateTimestamp();
    CollectResult<std::vector<std::string>> result = traceCollector_->TraceOff();
    uint64_t endTime = TimeUtil::GenerateTimestamp();
    const std::string classFuncName  = TRACE_COLLECTOR_NAME + UC_SEPARATOR + __func__;
    statInfoWrapper_.UpdateStatInfo(startTime, endTime, classFuncName, result.retCode == UCollect::UcError::SUCCESS);
    return result;
}

void TraceDecorator::SaveStatSpecialInfo()
{
    WriteLinesToFile({""}, false); // a blank line after common stat info
    std::map<std::string, TraceStatInfo> traceStatInfo = traceStatWrapper_.GetTraceStatInfo();
    std::vector<std::string> traceFormattedStatInfo = {UC_HITRACE_API_STAT_TITLE, UC_HITRACE_API_STAT_ITEM};
    for (const auto& record : traceStatInfo) {
        traceFormattedStatInfo.push_back(record.second.ToString());
    }
    WriteLinesToFile(traceFormattedStatInfo, true);

    std::vector<std::string> compressRatio = {UC_HITRACE_COMPRESS_RATIO, std::to_string(TRACE_COMPRESS_RATIO)};
    WriteLinesToFile(compressRatio, true);
    
    std::map<std::string, std::vector<std::string>> traceTrafficInfo = traceStatWrapper_.GetTrafficStatInfo();
    std::vector<std::string> trafficFormattedInfo = {UC_HITRACE_TRAFFIC_STAT_TITLE, UC_HITRACE_TRAFFIC_STAT_ITEM};
    for (const auto& record : traceTrafficInfo) {
        trafficFormattedInfo.insert(trafficFormattedInfo.end(), record.second.begin(), record.second.end());
    }
    WriteLinesToFile(trafficFormattedInfo, false);
}

void TraceDecorator::SaveStatCommonInfo()
{
    std::map<std::string, StatInfo> statInfo = statInfoWrapper_.GetStatInfo();
    std::vector<std::string> formattedStatInfo;
    for (const auto& record : statInfo) {
        formattedStatInfo.push_back(record.second.ToString());
    }
    WriteLinesToFile(formattedStatInfo, false);
}

void TraceDecorator::ResetStatInfo()
{
    statInfoWrapper_.ResetStatInfo();
    traceStatWrapper_.ResetStatInfo();
}

void TraceStatWrapper::UpdateTraceStatInfo(uint64_t startTime, uint64_t endTime, TraceCollector::Caller& caller,
    const CollectResult<std::vector<std::string>>& result)
{
    bool isCallSucc = (result.retCode == UCollect::UcError::SUCCESS);
    bool isOverCall = (result.retCode == UCollect::UcError::TRACE_OVER_FLOW);
    uint64_t latency = (endTime - startTime > 0) ? (endTime - startTime) : 0;
    std::string callerStr;
    if (CallerMap.find(caller) != CallerMap.end()) {
        callerStr = CallerMap.at(caller);
    } else {
        callerStr = "UNKNOWN";
    }
    TraceStatItem item = {.caller = callerStr, .isCallSucc = isCallSucc,
        .isOverCall = isOverCall, .latency = latency};
    UpdateAPIStatInfo(item);
    UpdateTrafficInfo(callerStr, latency, result);
}

void TraceStatWrapper::UpdateAPIStatInfo(const TraceStatItem& item)
{
    std::lock_guard<std::mutex> lock(traceMutex_);
    if (traceStatInfos_.find(item.caller) == traceStatInfos_.end()) {
        TraceStatInfo statInfo = {
            .caller = item.caller,
            .totalCall = 1,
            .failCall = (item.isCallSucc || item.isOverCall) ? 0 : 1,
            .overCall = item.isOverCall ? 1 : 0,
            .avgLatency = item.latency,
            .maxLatency = item.latency,
            .totalTimeSpend = item.latency,
        };
        traceStatInfos_.insert(std::make_pair(item.caller, statInfo));
        return;
    }
    
    TraceStatInfo& statInfo = traceStatInfos_[item.caller];
    statInfo.totalCall += 1;
    statInfo.failCall += ((item.isCallSucc || item.isOverCall) ? 0 : 1);
    statInfo.overCall += (item.isOverCall ? 1 : 0);
    statInfo.totalTimeSpend += item.latency;
    statInfo.totalTimeSpend += item.latency;
    if (statInfo.maxLatency < item.latency) {
        statInfo.maxLatency = item.latency;
    }
    uint32_t succCall = statInfo.totalCall;
    if (succCall > 0) {
        statInfo.avgLatency = statInfo.totalTimeSpend / succCall;
    }
}

void TraceStatWrapper::UpdateTrafficInfo(const std::string& caller, uint64_t latency,
    const CollectResult<std::vector<std::string>>& result)
{
    std::lock_guard<std::mutex> lock(traceMutex_);
    std::vector<std::string> traceFiles = result.data;
    if (latency <= 0 || traceFiles.empty()) { // dumptrace fail, no need to update traffic
        return;
    }
    
    uint64_t timeStamp = TimeUtil::GenerateTimestamp();
    uint64_t avgLatency = latency / traceFiles.size();
    for (const auto& file : traceFiles) {
        uint64_t fileSize = FileUtil::GetFileSize(file);
        TraceTrafficInfo statInfo;
        statInfo.caller = caller;
        statInfo.traceFile = file;
        statInfo.timeSpent = avgLatency;
        statInfo.rawSize = fileSize;
        statInfo.usedSize = fileSize * TRACE_COMPRESS_RATIO;
        statInfo.timeStamp = timeStamp;
        
        if (trafficStatInfos_.find(caller) == trafficStatInfos_.end()) {
            trafficStatInfos_[caller] = {statInfo.ToString()};
        } else {
            std::vector<std::string>& tmp = trafficStatInfos_[caller];
            tmp.push_back(statInfo.ToString());
        }
    }
}

std::map<std::string, TraceStatInfo> TraceStatWrapper::GetTraceStatInfo()
{
    std::lock_guard<std::mutex> lock(traceMutex_);
    return traceStatInfos_;
}

std::map<std::string, std::vector<std::string>> TraceStatWrapper::GetTrafficStatInfo()
{
    std::lock_guard<std::mutex> lock(traceMutex_);
    return trafficStatInfos_;
}

void TraceStatWrapper::ResetStatInfo()
{
    std::lock_guard<std::mutex> lock(traceMutex_);
    traceStatInfos_.clear();
    trafficStatInfos_.clear();
}
} // namespace UCollectUtil
} // namespace HiviewDFX
} // namespace OHOS
