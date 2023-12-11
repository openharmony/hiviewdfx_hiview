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

#ifndef FREEZE_JSON_GENERATOR_H
#define FREEZE_JSON_GENERATOR_H

#include <string>

namespace OHOS {
namespace HiviewDFX {

class FreezeJsonException {
public:
    class Builder {
    public:
        Builder() {};
        ~Builder() {};
        Builder& InitName(const std::string& name);
        Builder& InitMessage(const std::string& name);
        FreezeJsonException Build() const;
    
    private:
        std::string name_ = "";
        std::string message_ = "";
        friend class FreezeJsonException;
    };

    explicit FreezeJsonException(const FreezeJsonException::Builder& builder);
    ~FreezeJsonException() {};
    std::string JsonStr() const;

private:
    static const inline std::string jsonExceptionName = "name";
    static const inline std::string jsonExceptionMessage = "message";

    std::string name_;
    std::string message_;
};

class FreezeJsonMemory {
public:
    class Builder {
    public:
        Builder() {};
        ~Builder() {};
        Builder& InitRss(unsigned long long rss);
        Builder& InitVss(unsigned long long vss);
        Builder& InitPss(unsigned long long pss);
        Builder& InitSysFreeMem(unsigned long long sysFreeMem);
        Builder& InitSysAvailMem(unsigned long long sysAvailMem);
        Builder& InitSysTotalMem(unsigned long long sysTotalMem);
        FreezeJsonMemory Build() const;
    
    private:
        unsigned long long rss_ = 0;
        unsigned long long vss_ = 0;
        unsigned long long pss_ = 0;
        unsigned long long sysFreeMem_ = 0;
        unsigned long long sysAvailMem_ = 0;
        unsigned long long sysTotalMem_ = 0;
        friend class FreezeJsonMemory;
    };

    explicit FreezeJsonMemory(const FreezeJsonMemory::Builder& builder);
    ~FreezeJsonMemory() {};
    std::string JsonStr() const;

private:
    static const inline std::string jsonMemoryRss = "rss";
    static const inline std::string jsonMemoryVss = "vss";
    static const inline std::string jsonMemoryPss = "pss";
    static const inline std::string jsonMemorySysFreeMem = "sys_free_mem";
    static const inline std::string jsonMemorySysAvailMem = "sys_avail_mem";
    static const inline std::string jsonMemorySysTotalMem = "sys_total_mem";
    unsigned long long rss_;
    unsigned long long vss_;
    unsigned long long pss_;
    unsigned long long sysFreeMem_;
    unsigned long long sysAvailMem_;
    unsigned long long sysTotalMem_;
};

class FreezeJsonParams {
public:
    class Builder {
    public:
        Builder() {};
        ~Builder() {};
        Builder& InitTime(unsigned long long time);
        Builder& InitUuid(const std::string& uuid);
        Builder& InitFreezeType(const std::string& freezeType);
        Builder& InitForeground(const std::string& foreground);
        Builder& InitBundleVersion(const std::string& bundleVersion);
        Builder& InitBundleName(const std::string& bundleName);
        Builder& InitProcessName(const std::string& processName);
        Builder& InitPid(long pid);
        Builder& InitUid(long uid);
        Builder& InitException(const std::string& exception);
        Builder& InitHilog(const std::string& hilog);
        Builder& InitEventHandler(const std::string& eventHandler);
        Builder& InitEventHandlerSize3s(const std::string& eventHandlerSize3s);
        Builder& InitEventHandlerSize6s(const std::string& eventHandlerSize6s);
        Builder& InitPeerBinder(const std::string& peerBinder);
        Builder& InitThreads(const std::string& threads);
        Builder& InitMemory(const std::string& memory);
        FreezeJsonParams Build() const;
    
    private:
        unsigned long long time_ = 0;
        std::string uuid_ = "";
        std::string freezeType_ = "";
        std::string foreground_ = "";
        std::string bundleVersion_ = "";
        std::string bundleName_ = "";
        std::string processName_ = "";
        long pid_ = 0;
        long uid_ = 0;
        std::string exception_ = "{}";
        std::string hilog_ = "[]";
        std::string eventHandler_ = "[]";
        std::string eventHandlerSize3s_ = "";
        std::string eventHandlerSize6s_ = "";
        std::string peerBinder_ = "[]";
        std::string threads_ = "[]";
        std::string memory_ = "{}";
        friend class FreezeJsonParams;
    };

    explicit FreezeJsonParams(const FreezeJsonParams::Builder& builder);
    ~FreezeJsonParams() {};
    std::string JsonStr() const;

private:
    static const inline std::string jsonParamsTime = "time";
    static const inline std::string jsonParamsUuid = "uuid";
    static const inline std::string jsonParamsFreezeType = "freeze_type";
    static const inline std::string jsonParamsForeground = "foreground";
    static const inline std::string jsonParamsBundleVersion = "bundle_version";
    static const inline std::string jsonParamsBundleName = "bundle_name";
    static const inline std::string jsonParamsProcessName = "process_name";
    static const inline std::string jsonParamsPid = "pid";
    static const inline std::string jsonParamsUid = "uid";
    static const inline std::string jsonParamsException = "exception";
    static const inline std::string jsonParamsHilog = "hilog";
    static const inline std::string jsonParamsEventHandler = "event_handler";
    static const inline std::string jsonParamsEventHandlerSize3s = "event_handler_size_3s";
    static const inline std::string jsonParamsEventHandlerSize6s = "event_handler_size_6s";
    static const inline std::string jsonParamsPeerBinder = "peer_binder";
    static const inline std::string jsonParamsThreads = "threads";
    static const inline std::string jsonParamsMemory = "memory";
    
    unsigned long long time_;
    std::string uuid_;
    std::string freezeType_;
    std::string foreground_;
    std::string bundleVersion_;
    std::string bundleName_;
    std::string processName_;
    long pid_ = 0;
    long uid_ = 0;
    std::string exception_;
    std::string hilog_;
    std::string eventHandler_;
    std::string eventHandlerSize3s_;
    std::string eventHandlerSize6s_;
    std::string peerBinder_;
    std::string threads_;
    std::string memory_;
};

} // namespace HiviewDFX
} // namespace OHOS
#endif