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

#ifndef FREEZE_RULE_CLUSTER_H
#define FREEZE_RULE_CLUSTER_H

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "watch_point.h"
namespace OHOS {
namespace HiviewDFX {
class FreezeResult {
public:
    FreezeResult() : window_(0), code_(0), scope_(""), samePackage_(""), domain_(""), stringId_("") {};
    FreezeResult(long window, const std::string& domain, const std::string& stringId)
        : window_(window), code_(0), scope_(""), samePackage_(""), domain_(domain), stringId_(stringId) {};
    FreezeResult(unsigned long code, const std::string& scope)
        : window_(0), code_(code), scope_(scope), samePackage_(""), domain_(""), stringId_("") {};
    ~FreezeResult() {};
    std::string GetDomain() const
    {
        return domain_;
    }

    std::string GetStringId() const
    {
        return stringId_;
    }

    unsigned long GetId() const
    {
        return code_;
    }

    void SetId(unsigned long code)
    {
        code_ = code;
    }

    std::string GetScope() const
    {
        return scope_;
    }

    void SetScope(const std::string& scope)
    {
        scope_ = scope;
    }

    long GetWindow() const
    {
        return window_;
    }

    std::string GetSamePackage() const
    {
        return samePackage_;
    }

    void SetSamePackage(const std::string& samePackage)
    {
        samePackage_ = samePackage;
    }

private:
    long window_;
    unsigned long code_;
    std::string scope_;
    std::string samePackage_;
    std::string domain_;
    std::string stringId_;
};

class FreezeRule {
public:
    FreezeRule() : domain_(""), stringId_("") {};
    FreezeRule(const std::string& domain, const std::string& stringId)
        : domain_(domain), stringId_(stringId) {};
    ~FreezeRule()
    {
        results_.clear();
    }

    std::string GetDomain() const
    {
        return domain_;
    }

    void SetDomain(const std::string& domain)
    {
        domain_ = domain;
    }

    std::string GetStringId() const
    {
        return stringId_;
    }

    void SetStringId(const std::string& stringId)
    {
        stringId_ = stringId;
    }

    std::map<std::string, FreezeResult> GetMap() const
    {
        return results_;
    }

    void AddResult(const std::string& domain, const std::string& stringId, const FreezeResult& result);
    bool GetResult(const std::string& domain, const std::string& stringId, FreezeResult& result);

private:
    std::string domain_;
    std::string stringId_;
    std::map<std::string, FreezeResult> results_;
};

class FreezeRuleCluster {
public:
    FreezeRuleCluster();
    ~FreezeRuleCluster();
    FreezeRuleCluster& operator=(const FreezeRuleCluster&) = delete;
    FreezeRuleCluster(const FreezeRuleCluster&) = delete;

    bool Init();
    bool CheckFileSize(const std::string& path);
    bool ParseRuleFile(const std::string& file);
    void ParseTagFreeze(xmlNode* tag);
    void ParseTagRules(xmlNode* tag);
    void ParseTagRule(xmlNode* tag);
    void ParseTagLinks(xmlNode* tag, FreezeRule& rule);
    void ParseTagEvent(xmlNode* tag, FreezeResult& result);
    void ParseTagResult(xmlNode* tag, FreezeResult& result);
    void ParseTagRelevance(xmlNode* tag, FreezeResult& result);
    template<typename T>
    T GetAttributeValue(xmlNode* node, const std::string& name);
    bool GetResult(const WatchPoint& watchPoint, std::vector<FreezeResult>& list);
    std::map<std::string, std::pair<std::string, bool>> GetApplicationPairs() const
    {
        return applicationPairs_;
    }

    std::map<std::string, std::pair<std::string, bool>> GetSystemPairs() const
    {
        return systemPairs_;
    }

private:
    static const inline std::string DEFAULT_RULE_FILE = "/system/etc/hiview/freeze_rules.xml";
    static const inline std::string TAG_FREEZE = "freeze";
    static const inline std::string TAG_RULES = "rules";
    static const inline std::string TAG_RULE = "rule";
    static const inline std::string TAG_LINKS = "links";
    static const inline std::string TAG_EVENT = "event";
    static const inline std::string TAG_RESULT = "result";
    static const inline std::string TAG_RELEVANCE = "relevance";
    static const inline std::string ATTRIBUTE_ID = "id";
    static const inline std::string ATTRIBUTE_WINDOW = "window";
    static const inline std::string ATTRIBUTE_DOMAIN = "domain";
    static const inline std::string ATTRIBUTE_STRINGID = "stringid";
    static const inline std::string ATTRIBUTE_TYPE = "type";
    static const inline std::string ATTRIBUTE_USER = "user";
    static const inline std::string ATTRIBUTE_WATCHPOINT = "watchpoint";
    static const inline std::string ATTRIBUTE_CODE = "code";
    static const inline std::string ATTRIBUTE_SCOPE = "scope";
    static const inline std::string ATTRIBUTE_SAME_PACKAGE = "samePackage";
    static const inline std::string ATTRIBUTE_APPLICATION = "application";
    static const inline std::string ATTRIBUTE_SYSTEM = "system";
    static const int MAX_FILE_SIZE = 512 * 1024;

    std::map<std::string, FreezeRule> rules_;
    std::map<std::string, std::pair<std::string, bool>> applicationPairs_;
    std::map<std::string, std::pair<std::string, bool>> systemPairs_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
