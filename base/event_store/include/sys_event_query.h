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

#ifndef HIVIEW_BASE_EVENT_STORE_INCLUDE_SYS_EVENT_QUERY_H
#define HIVIEW_BASE_EVENT_STORE_INCLUDE_SYS_EVENT_QUERY_H

#ifndef DllExport
#define DllExport
#endif // DllExport

#include <functional>
#include <queue>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "base_def.h"
#include "doc_query.h"
#include "sys_event.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
enum DbQueryStatus { SUCCEED = 0, CONCURRENT, OVER_TIME, OVER_LIMIT, TOO_FREQENTLY };
using DbQueryTag = struct {
    bool isInnerQuery;
    bool needFrequenceCheck;
};
using DbQueryCallback = std::function<void(DbQueryStatus)>;
using QueryProcessInfo = std::pair<pid_t, std::string>; // first: pid of process, second: process name

using CompareFunc = bool(*)(const Entry&, const Entry&);
using EntryQueue = std::priority_queue<Entry, std::vector<Entry>, CompareFunc>;

constexpr pid_t INNER_PROCESS_ID = -1;

enum Op { NONE = 0, EQ = 1, NE, LT, LE, GT, GE, SW, NSW };

class SysEventDao;
class SysEventDatabase;

class EventCol {
public:
    static std::string DOMAIN;
    static std::string NAME;
    static std::string TYPE;
    static std::string TS;
    static std::string TZ;
    static std::string PID;
    static std::string TID;
    static std::string UID;
    static std::string INFO;
    static std::string LEVEL;
    static std::string SEQ;
    static std::string TAG;
};

class FieldValue {
public:
    enum ValueType { INTEGER = 0, DOUBLE = 1, STRING = 2 };

    FieldValue(): value_(0) {}
    FieldValue(int32_t value): value_(static_cast<int64_t>(value)) {}
    FieldValue(int64_t value): value_(value) {}
    FieldValue(double value): value_(value) {}
    FieldValue(const std::string &value): value_(value) {}
    ~FieldValue() {}

    bool operator==(const FieldValue& fieldValue) const;
    bool operator!=(const FieldValue& fieldValue) const;
    bool operator<(const FieldValue& fieldValue) const;
    bool operator<=(const FieldValue& fieldValue) const;
    bool operator>(const FieldValue& fieldValue) const;
    bool operator>=(const FieldValue& fieldValue) const;
    bool IsStartWith(const FieldValue& fieldValue) const;
    bool IsNotStartWith(const FieldValue& fieldValue) const;

    bool IsInteger() const;
    bool IsDouble() const;
    bool IsString() const;
    int64_t GetInteger() const;
    double GetDouble() const;
    std::string GetString() const;

    std::variant<int64_t, double, std::string> value_;
};

class DllExport Cond {
public:
    Cond(): op_(NONE), fieldValue_(0) {}
    ~Cond() {}

    template <typename T>
    Cond(const std::string &col, Op op, const T &value): col_(col), op_(op), fieldValue_(value) {}

    template <typename T>
    Cond &And(const std::string &col, Op op, const T &value)
    {
        andConds_.emplace_back(Cond(col, op, value));
        return *this;
    }
    Cond &And(const Cond &cond);

    std::string ToString() const;

private:
    friend class DocQuery;
    friend class SysEventQuery;
    static bool IsSimpleCond(const Cond &cond);
    static void Traval(DocQuery &docQuery, const Cond &cond);

private:
    std::string col_;
    Op op_;
    FieldValue fieldValue_;
    std::vector<Cond> andConds_;
};  // Cond

class DllExport ResultSet {
public:
    ResultSet();
    ResultSet(ResultSet &&result);
    ResultSet& operator = (ResultSet &&result);
    ~ResultSet();

public:
    using RecordIter = std::vector<SysEvent>::iterator;
    int GetErrCode() const;
    bool HasNext() const;
    RecordIter Next();

private:
    friend class SysEventQuery;
    friend class SysEventQueryWrapper;
    void Set(int code, bool has);
    std::vector<SysEvent> eventRecords_;
    RecordIter iter_;
    int code_;
    bool has_;
};  // ResultSet

/* Query parameters for filtering file names */
struct SysEventQueryArg {
    std::string domain;
    std::vector<std::string> names;
    uint32_t type;
    int64_t toSeq;

    SysEventQueryArg() : SysEventQueryArg("", {}, 0, INVALID_VALUE_INT) {}
    SysEventQueryArg(const std::string& domain, const std::vector<std::string>& names,
        uint32_t type, int64_t toSeq)
    {
        this->domain = domain;
        this->names.assign(names.begin(), names.end());
        this->type = type;
        this->toSeq = toSeq;
    }
    ~SysEventQueryArg() {}
};

class DllExport SysEventQuery {
public:
    SysEventQuery(const std::string& domain, const std::vector<std::string>& names);
    virtual ~SysEventQuery() {}

    SysEventQuery &Select(const std::vector<std::string> &eventCols);

    template <typename T>
    SysEventQuery &Where(const std::string &col, Op op, const T &value)
    {
        cond_.And(col, op, value);
        return *this;
    }
    SysEventQuery &Where(const Cond &cond);

    template <typename T>
    SysEventQuery &And(const std::string &col, Op op, const T &value)
    {
        cond_.And(col, op, value);
        return *this;
    }
    SysEventQuery &And(const Cond &cond);

    SysEventQuery &Order(const std::string &col, bool isAsc = true);

    virtual ResultSet Execute(int limit = 100, DbQueryTag tag = { true, true },
        QueryProcessInfo callerInfo = std::make_pair(INNER_PROCESS_ID, ""),
        DbQueryCallback queryCallback = nullptr);

    std::string ToString() const;

    friend class SysEventDao;
    friend class SysEventDatabase;

protected:
    SysEventQuery();
    SysEventQuery(const std::string& domain, const std::vector<std::string>& names, uint32_t type, int64_t toSeq);

private:
    void BuildDocQuery(DocQuery &docQuery) const;
    CompareFunc CreateCompareFunc() const;

    int limit_;
    std::pair<std::string, bool> orderCol_;
    Cond cond_;
    SysEventQueryArg queryArg_;
}; // SysEventQuery
} // EventStore
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_BASE_EVENT_STORE_INCLUDE_SYS_EVENT_QUERY_H
