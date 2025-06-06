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

#ifndef FREEZE_WATCHPOINT_H
#define FREEZE_WATCHPOINT_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace HiviewDFX {
class WatchPoint {
public:
    class Builder {
    public:
        Builder();
        ~Builder();
        Builder& InitSeq(long seq);
        Builder& InitTimestamp(unsigned long long timestamp);
        Builder& InitPid(long pid);
        Builder& InitTid(long tid);
        Builder& InitUid(long uid);
        Builder& InitTerminalThreadStack(const std::string& terminalThreadStack);
        Builder& InitTelemetryId(const std::string& telemetryId);
        Builder& InitTraceName(const std::string& traceName);
        Builder& InitDomain(const std::string& domain);
        Builder& InitStringId(const std::string& stringId);
        Builder& InitMsg(const std::string& msg);
        Builder& InitPackageName(const std::string& packageName);
        Builder& InitProcessName(const std::string& processName);
        Builder& InitForeGround(const std::string& foreGround);
        Builder& InitLogPath(const std::string& logPath);
        Builder& InitHitraceTime(const std::string& hitraceTime);
        Builder& InitSysrqTime(const std::string& sysrqTime);
        Builder& InitHitraceIdInfo(const std::string& hitraceIdInfo);
        WatchPoint Build() const;

    private:
        long seq_;
        unsigned long long timestamp_;
        long pid_;
        long tid_;
        long uid_;
        std::string terminalThreadStack_;
        std::string telemetryId_;
        std::string traceName_;
        std::string domain_;
        std::string stringId_;
        std::string msg_;
        std::string packageName_;
        std::string processName_;
        std::string foreGround_;
        std::string logPath_;
        std::string hitraceTime_;
        std::string sysrqTime_;
        std::string hitraceIdInfo_;
        friend class WatchPoint;
    };

    WatchPoint();
    explicit WatchPoint(const WatchPoint::Builder& builder);
    ~WatchPoint() {};

    long GetSeq() const;
    unsigned long long GetTimestamp() const;
    long GetPid() const;
    long GetTid() const;
    long GetUid() const;
    std::string GetTerminalThreadStack() const;
    std::string GetTelemetryId() const;
    std::string GetTraceName() const;
    std::string GetDomain() const;
    std::string GetStringId() const;
    std::string GetMsg() const;
    std::string GetPackageName() const;
    std::string GetProcessName() const;
    std::string GetForeGround() const;
    std::string GetLogPath() const;
    std::string GetHitraceTime() const;
    std::string GetSysrqTime() const;
    std::string GetHitraceIdInfo() const;
    void SetLogPath(const std::string& logPath);
    void SetTerminalThreadStack(const std::string& terminalThreadStack);
    void SetSeq(long seq);
    bool operator<(const WatchPoint& node) const;
    bool operator==(const WatchPoint& node) const;

private:
    long seq_;
    unsigned long long timestamp_;
    long pid_;
    long tid_;
    long uid_;
    std::string terminalThreadStack_;
    std::string telemetryId_;
    std::string traceName_;
    std::string domain_;
    std::string stringId_;
    std::string msg_;
    std::string packageName_;
    std::string processName_;
    std::string foreGround_;
    std::string logPath_;
    std::string hitraceTime_;
    std::string sysrqTime_;
    std::string hitraceIdInfo_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
