/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include "uc_telemetry_listener.h"

#include "cjson_util.h"
#include "hiview_logger.h"
#include "time_util.h"
#include "hisysevent.h"
#include "uc_telemetry_callback.h"

namespace OHOS::HiviewDFX {
DEFINE_LOG_TAG("HiView-UnifiedCollector");
namespace {

const int32_t DURATION_DEFAULT = 3600 * SECONDS_TO_MS; // ms
const uint32_t DEFAULT_RATIO = 7;
const uint32_t BT_M_UNIT = 1024 * 1024;
constexpr char TELEMETRY_DOMAIN[] = "TELEMETRY";
constexpr char TAGS[] = "tags";
constexpr char BUFFER_SIZE[] = "bufferSize";

const std::string SWITCH_ON = "on";
const std::string SWITCH_OFF = "off";
const std::string POLICY_POWER = "power";
const std::string POLICY_MANUAL = "manual";

// Default quota of flow control
const int64_t DEFAULT_XPERF_SIZE = 140 * 1024 * 1024;
const int64_t DEFAULT_XPOWER_SIZE = 140 * 1024 * 1024;
const int64_t DEFAULT_RELIABILITY_SIZE = 140 * 1024 * 1024;
const int64_t DEFAULT_TOTAL_SIZE = 350 * 1024 * 1024;
const int64_t MAX_TOTAL_SIZE = 500;

std::vector<std::string> ParseAndFilterTraceArgs(const std::unordered_set<std::string> &filterList,
    cJSON* root, const std::string &key)
{
    if (!cJSON_IsObject(root)) {
        HIVIEW_LOGE("trace jsonArgs parse error");
        return {};
    }
    std::vector<std::string> traceArgs;
    CJsonUtil::GetStringArray(root, key, traceArgs);
    auto new_end = std::remove_if(traceArgs.begin(), traceArgs.end(), [&filterList](const std::string& tag) {
        return filterList.find(tag) == filterList.end();
    });
    traceArgs.erase(new_end, traceArgs.end());
    return traceArgs;
}
}

void TelemetryListener::OnUnorderedEvent(const Event &msg)
{
    bool isCloseMsg = false;
    TelemetryParams params;
    std::string errorMsg = CheckValidParam(msg, params, isCloseMsg);
    HIVIEW_LOGI("isClose:%{public}d", isCloseMsg);
    if (!errorMsg.empty()) {
        return WriteErrorEvent(errorMsg, params);
    }
    if (isCloseMsg) {
        HandleStop();
        return;
    }
    params.appFilterName = msg.GetValue(Telemetry::KEY_FILTER_NAME);
    params.traceDuration = msg.GetInt64Value(Telemetry::KEY_DURATION) * SECONDS_TO_MS;
    if (params.traceDuration <= 0 || params.traceDuration > DURATION_DEFAULT) {
        params.traceDuration = DURATION_DEFAULT;
    }
    GetSaNames(msg, params);

    bool isTimeOut = false;
    if (!InitTelemetryDbData(msg, isTimeOut, params)) {
        return WriteErrorEvent("init telemetry time table fail", params);
    }
    if (isTimeOut) {
        HIVIEW_LOGE("%{public}s", "trace already time out");
        return;
    }
    auto delaySeconds = params.beginTime - TimeUtil::GetSeconds();
    if (delaySeconds <= 0) {
        HandleStart(params);
    } else {
        if (taskQueue_ == nullptr) {
            taskQueue_ = std::make_unique<ffrt::queue>("telemetry_queue");
        }
        startTaskHandle_ = taskQueue_->submit_h([this, params] { this->HandleStart(params); },
            ffrt::task_attr().delay(delaySeconds * SECONDS_TO_MS * MS_TO_US));
    }
}

std::string TelemetryListener::CheckValidParam(const Event &msg, TelemetryParams &params, bool &isCloseMsg)
{
    std::string errorMsg;
    if (!CheckTelemetryId(msg, params, errorMsg)) {
        return errorMsg;
    }
    if (!CheckTraceTags(msg, params, errorMsg)) {
        return errorMsg;
    }
    if (!CheckTracePolicy(msg, params, errorMsg)) {
        return errorMsg;
    }
    if (!CheckSwitchValid(msg, isCloseMsg, errorMsg)) {
        return errorMsg;
    }
    if (!CheckBeginTime(msg, params, errorMsg)) {
        return errorMsg;
    }
    return errorMsg;
}

bool TelemetryListener::InitTelemetryDbData(const Event &msg, bool &isTimeOut, const TelemetryParams &params)
{
    std::map<std::string, int64_t> flowControlQuotas {
        {CallerName::XPERF, DEFAULT_XPERF_SIZE },
        {CallerName::XPOWER, DEFAULT_XPOWER_SIZE},
        {CallerName::RELIABILITY, DEFAULT_RELIABILITY_SIZE},
        {Telemetry::TOTAL, DEFAULT_TOTAL_SIZE}
    };
    int32_t traceCompressRatio = msg.GetIntValue(Telemetry::KEY_FLOW_RATE);
    if (traceCompressRatio <= 0) {
        traceCompressRatio = DEFAULT_RATIO;
    }
    auto xperfTraceQuota = msg.GetInt64Value(Telemetry::KEY_XPERF_QUOTA);
    if (xperfTraceQuota > 0) {
        flowControlQuotas[CallerName::XPERF] = xperfTraceQuota * traceCompressRatio * BT_M_UNIT;
    }
    auto xpowerTraceQuota = msg.GetInt64Value(Telemetry::KEY_XPOWER_QUOTA);
    if (xpowerTraceQuota > 0) {
        flowControlQuotas[CallerName::XPOWER] = xpowerTraceQuota * traceCompressRatio * BT_M_UNIT;
    }
    auto reliabilityTraceQuota = msg.GetInt64Value(Telemetry::KEY_RELIABILITY_QUOTA);
    if (reliabilityTraceQuota > 0) {
        flowControlQuotas[CallerName::RELIABILITY] = reliabilityTraceQuota * traceCompressRatio * BT_M_UNIT;
    }
    auto totalTraceQuota = msg.GetInt64Value(Telemetry::KEY_TOTAL_QUOTA);
    if (totalTraceQuota > 0 && totalTraceQuota <= MAX_TOTAL_SIZE) {
        flowControlQuotas[Telemetry::TOTAL] = totalTraceQuota * traceCompressRatio * BT_M_UNIT;
    } else if (totalTraceQuota > MAX_TOTAL_SIZE) {
        flowControlQuotas[Telemetry::TOTAL] = MAX_TOTAL_SIZE * traceCompressRatio * BT_M_UNIT;
    } else {
        HIVIEW_LOGI("default total quota size");
    }

    int64_t running_time = 0;
    auto ret = TraceFlowController(BusinessName::TELEMETRY).InitTelemetryData(params.telemetryId, running_time,
        flowControlQuotas);
    if (ret == TelemetryRet::EXIT) {
        return false;
    }
    isTimeOut = running_time >= params.traceDuration;
    return true;
}

void TelemetryListener::HandleStart(const TelemetryParams &params)
{
    auto ret = TraceStateMachine::GetInstance().OpenTelemetryTrace(params.traceTag, params.tracePolicy);
    if (ret.IsSuccess()) {
        std::shared_ptr<TelemetryCallback> callback;
        switch (params.tracePolicy) {
            case TelemetryPolicy::POWER:
                callback = std::make_shared<PowerCallback>(params);
                break;
            case TelemetryPolicy::MANUAL:
                callback = std::make_shared<ManualCallback>(params);
                break;
            default:
                callback = std::make_shared<UcTelemetryCallback>(params);
                break;
        }
        bool isSuccess = TraceStateMachine::GetInstance().RegisterTelemetryCallback(callback);
        HIVIEW_LOGI("register callback result:%{public}d, traceDuration%{public}" PRId64 "", isSuccess,
            params.traceDuration);
    } else {
        WriteErrorEvent("trace state error", params);
    }
}

void TelemetryListener::HandleStop()
{
    TraceStateMachine::GetInstance().CloseTrace(TraceScenario::TRACE_TELEMETRY);
    TraceFlowController controller(BusinessName::TELEMETRY);
    controller.ClearTelemetryData();
    if (taskQueue_ != nullptr && taskQueue_->cancel(startTaskHandle_) < 0) {
        HIVIEW_LOGW("%{public}s", "telemetstartTaskHandle_ry trace already start");
    }
}

void TelemetryListener::WriteErrorEvent(const std::string &error, const TelemetryParams &params)
{
    HIVIEW_LOGE("%{public}s", error.c_str());
    HiSysEventWrite(TELEMETRY_DOMAIN, "TASK_INFO", HiSysEvent::EventType::STATISTIC,
        "ID", params.telemetryId,
        "STAGE", "TRACE_BEGIN",
        "ERROR", error);
}

bool TelemetryListener::ProcessTraceTag(std::string &traceTag)
{
    cJSON* root = cJSON_Parse(traceTag.c_str());
    if (root == nullptr) {
        return false;
    }
    auto tags = ParseAndFilterTraceArgs(Telemetry::TRACE_TAG_FILTER_LIST, root, TAGS);
    if (tags.empty()) {
        cJSON_Delete(root);
        return false;
    }
    auto bufferSize = CJsonUtil::GetIntValue(root, BUFFER_SIZE);
    cJSON_Delete(root);
    if (bufferSize <= 0) {
        HIVIEW_LOGE("jsonArgs parse trace bufferSize error");
        return false;
    }
    bool isFirst = true;
    std::string result("tags:");
    for (const auto &tag: tags) {
        if (!isFirst) {
            result.append(", ").append(tag);
            continue;
        }
        result.append(tag);
        isFirst = false;
    }
    result.append(" bufferSize:").append(std::to_string(bufferSize));
    traceTag = std::move(result);
    return true;
}

bool TelemetryListener::CheckTelemetryId(const Event &msg, TelemetryParams &params, std::string &errorMsg)
{
    std::string telemetryId = msg.GetValue(Telemetry::KEY_ID);
    if (telemetryId.empty()) {
        errorMsg.append("telemetryId get empty");
        return false;
    }
    params.telemetryId = telemetryId;
    return true;
}

void TelemetryListener::GetSaNames(const Event &msg, TelemetryParams &params)
{
    std::string saJsonNames = msg.GetValue(Telemetry::KEY_SA_NAMES);
    if (!saJsonNames.empty()) {
        cJSON* root = cJSON_Parse(saJsonNames.c_str());
        if (root == nullptr) {
            return;
        }
        auto saNames = ParseAndFilterTraceArgs(Telemetry::TRACE_SA_FILTER_LIST, root, Telemetry::KEY_SA_NAMES);
        for (const auto &saName : saNames) {
            auto param = "startup.service.ctl." + saName + ".pid";
            params.saParams.emplace_back(param);
        }
        cJSON_Delete(root);
    }
}

bool TelemetryListener::CheckTraceTags(const Event &msg, TelemetryParams &params, std::string &errorMsg)
{
    auto traceTag = msg.GetValue(Telemetry::KEY_TRACE_TAG);
    if (!traceTag.empty() && !ProcessTraceTag(traceTag)) {
        errorMsg.append("process trace tag fail");
        return false;
    }
    params.traceTag = traceTag;
    return true;
}

bool TelemetryListener::CheckTracePolicy(const Event &msg, TelemetryParams &params, std::string &errorMsg)
{
    auto tracePolicy = msg.GetValue(Telemetry::KEY_TRACE_POLICY);
    if (tracePolicy.empty()) {
        params.tracePolicy = TelemetryPolicy::DEFAULT;
        return true;
    }
    if (tracePolicy == POLICY_POWER) {
        params.tracePolicy = TelemetryPolicy::POWER;
    } else if (tracePolicy == POLICY_MANUAL) {
        params.tracePolicy = TelemetryPolicy::MANUAL;
    } else {
        errorMsg.append("trace policy get empty");
        return false;
    }
    return true;
}

bool TelemetryListener::CheckSwitchValid(const Event &msg, bool &isCloseMsg, std::string &errorMsg)
{
    auto switchStatus = msg.GetValue(Telemetry::KEY_SWITCH_STATUS);
    if (switchStatus.empty()) {
        errorMsg.append("switchStatus get empty");
        return false;
    }
    if (switchStatus == SWITCH_OFF) {
        isCloseMsg = true;
        return false;
    }
    if (switchStatus != SWITCH_ON) {
        errorMsg.append("switchStatus param get error");
        return false;
    }
    return true;
}

bool TelemetryListener::CheckBeginTime(const Event &msg, TelemetryParams &params, std::string &errorMsg)
{
    // Get begin time of telemetry trace unit seconds
    int64_t beginTime = msg.GetInt64Value(Telemetry::KEY_OPEN_TIME);
    if (beginTime < 0) {
        errorMsg.append("begin time get failed");
        return false;
    }
    params.beginTime = beginTime;
    return true;
}
}
