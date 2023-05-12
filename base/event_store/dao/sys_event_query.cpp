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
#include "sys_event_query.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "doc_query.h"
#include "sys_event_database.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
std::string EventCol::DOMAIN = "domain_";
std::string EventCol::NAME = "name_";
std::string EventCol::TYPE = "type_";
std::string EventCol::TS = "time_";
std::string EventCol::TZ = "tz_";
std::string EventCol::PID = "pid_";
std::string EventCol::TID = "tid_";
std::string EventCol::UID = "uid_";
std::string EventCol::INFO = "info_";
std::string EventCol::LEVEL = "level_";
std::string EventCol::SEQ = "seq_";

bool FieldValue::IsInteger() const
{
    return (value_.index() == INTEGER);
}

bool FieldValue::IsDouble() const
{
    return (value_.index() == DOUBLE);
}

bool FieldValue::IsString() const
{
    return (value_.index() == STRING);
}

int64_t FieldValue::GetInteger() const
{
    return std::get<INTEGER>(value_);
}

double FieldValue::GetDouble() const
{
    return std::get<DOUBLE>(value_);
}

std::string FieldValue::GetString() const
{
    return std::get<STRING>(value_);
}

bool FieldValue::operator==(const FieldValue& fieldValue) const
{
    if (this->value_.index() != fieldValue.value_.index()) {
        return false;
    }
    if (this->IsInteger()) {
        return this->GetInteger() == fieldValue.GetInteger();
    }
    if (this->IsDouble()) {
        return this->GetDouble() == fieldValue.GetDouble();
    }
    if (this->IsString()) {
        return this->GetString() == fieldValue.GetString();
    }
    return false;
}

bool FieldValue::operator!=(const FieldValue& fieldValue) const
{
    if (this->value_.index() != fieldValue.value_.index()) {
        return false;
    }
    if (this->IsInteger()) {
        return this->GetInteger() != fieldValue.GetInteger();
    }
    if (this->IsDouble()) {
        return this->GetDouble() != fieldValue.GetDouble();
    }
    if (this->IsString()) {
        return this->GetString() != fieldValue.GetString();
    }
    return false;
}

bool FieldValue::operator<(const FieldValue& fieldValue) const
{
    if (this->value_.index() != fieldValue.value_.index()) {
        return false;
    }
    if (this->IsInteger()) {
        return this->GetInteger() < fieldValue.GetInteger();
    }
    if (this->IsDouble()) {
        return this->GetDouble() < fieldValue.GetDouble();
    }
    if (this->IsString()) {
        return this->GetString() < fieldValue.GetString();
    }
    return false;
}

bool FieldValue::operator<=(const FieldValue& fieldValue) const
{
    if (this->value_.index() != fieldValue.value_.index()) {
        return false;
    }
    if (this->IsInteger()) {
        return this->GetInteger() <= fieldValue.GetInteger();
    }
    if (this->IsDouble()) {
        return this->GetDouble() <= fieldValue.GetDouble();
    }
    if (this->IsString()) {
        return this->GetString() <= fieldValue.GetString();
    }
    return false;
}

bool FieldValue::operator>(const FieldValue& fieldValue) const
{
    if (this->value_.index() != fieldValue.value_.index()) {
        return false;
    }
    if (this->IsInteger()) {
        return this->GetInteger() > fieldValue.GetInteger();
    }
    if (this->IsDouble()) {
        return this->GetDouble() > fieldValue.GetDouble();
    }
    if (this->IsString()) {
        return this->GetString() > fieldValue.GetString();
    }
    return false;
}

bool FieldValue::operator>=(const FieldValue& fieldValue) const
{
    if (this->value_.index() != fieldValue.value_.index()) {
        return false;
    }
    if (this->IsInteger()) {
        return this->GetInteger() >= fieldValue.GetInteger();
    }
    if (this->IsDouble()) {
        return this->GetDouble() >= fieldValue.GetDouble();
    }
    if (this->IsString()) {
        return this->GetString() >= fieldValue.GetString();
    }
    return false;
}

bool FieldValue::IsStartWith(const FieldValue& fieldValue) const
{
    if (this->value_.index() != fieldValue.value_.index()) {
        return false;
    }
    if (this->IsString()) {
        return this->GetString().find(fieldValue.GetString()) == 0;
    }
    return false;
}

bool FieldValue::IsNotStartWith(const FieldValue& fieldValue) const
{
    if (this->value_.index() != fieldValue.value_.index()) {
        return false;
    }
    if (this->IsString()) {
        return !(this->GetString().find(fieldValue.GetString()) == 0);
    }
    return false;
}

Cond &Cond::And(const Cond &cond)
{
    andConds_.emplace_back(cond);
    return *this;
}

bool Cond::IsSimpleCond(const Cond &cond)
{
    return (!cond.col_.empty() && cond.andConds_.size() == 0)
        || (cond.col_.empty() && cond.andConds_.size() == 1); // 1 means simple condition
}

void Cond::Traval(DocQuery &docQuery, const Cond &cond)
{
    if (!cond.col_.empty()) {
        docQuery.And(cond);
    }
    if (!cond.andConds_.empty()) {
        for (auto it = cond.andConds_.begin(); it != cond.andConds_.end(); it++) {
            if (IsSimpleCond(*it)) {
                docQuery.And(*it);
            } else {
                Traval(docQuery, *it);
            }
        }
    }
}

ResultSet::ResultSet(): iter_(eventRecords_.begin()), code_(0), has_(false)
{
}

ResultSet::~ResultSet()
{
}

ResultSet::ResultSet(ResultSet &&result)
{
    eventRecords_ = move(result.eventRecords_);
    code_ = result.code_;
    has_ = result.has_;
    iter_ = result.iter_;
}

ResultSet& ResultSet::operator = (ResultSet &&result)
{
    eventRecords_ = move(result.eventRecords_);
    code_ = result.code_;
    has_ = result.has_;
    iter_ = result.iter_;
    return *this;
}

int ResultSet::GetErrCode() const
{
    return code_;
}

bool ResultSet::HasNext() const
{
    return has_;
}

ResultSet::RecordIter ResultSet::Next()
{
    if (!has_) {
        return eventRecords_.end();
    }

    auto tempIter = iter_;
    iter_++;
    if (iter_ == eventRecords_.end()) {
        has_ = false;
    }

    return tempIter;
}

void ResultSet::Set(int code, bool has)
{
    code_ = code;
    has_ = has;
    if (eventRecords_.size() > 0) {
        iter_ = eventRecords_.begin();
    }
}

SysEventQuery::SysEventQuery(const std::string& domain, const std::vector<std::string>& names)
    : SysEventQuery(domain, names, 0, INVALID_VALUE_INT)
{}

SysEventQuery::SysEventQuery(const std::string& domain, const std::vector<std::string>& names,
    uint32_t type, int64_t toSeq) : queryArg_(domain, names, type, toSeq)
{}

SysEventQuery &SysEventQuery::Select(const std::vector<std::string> &eventCols)
{
    return *this;
}

SysEventQuery &SysEventQuery::Where(const Cond &cond)
{
    cond_.And(cond);
    return *this;
}

SysEventQuery &SysEventQuery::And(const Cond &cond)
{
    cond_.And(cond);
    return *this;
}

SysEventQuery &SysEventQuery::Order(const std::string &col, bool isAsc)
{
    orderCol_ = std::make_pair<>(col, isAsc);
    return *this;
}

void SysEventQuery::BuildDocQuery(DocQuery &docQuery) const
{
    Cond::Traval(docQuery, cond_);
    if (!orderCol_.first.empty()) {
        // docQuery.Order(orderCol_.first, orderCol_.second); liangyujian
    }
}

ResultSet SysEventQuery::Execute(int limit, DbQueryTag tag, QueryProcessInfo callerInfo,
    DbQueryCallback queryCallback)
{
    return ExecuteSQL(limit);
}

ResultSet SysEventQuery::ExecuteSQL(int limit)
{
    limit_ = limit;
    std::vector<Entry> entries;
    int retCode = SysEventDatabase::GetInstance().Query(*this, entries);
    ResultSet resultSet;
    if (retCode != DOC_STORE_SUCCESS) {
        resultSet.Set(retCode, false);
        return resultSet;
    }
    if (entries.empty()) {
        resultSet.Set(DOC_STORE_SUCCESS, false);
        return resultSet;
    }
    for (auto it = entries.begin(); it != entries.end(); it++) {
        SysEvent sysEvent("", nullptr, it->value);
        sysEvent.SetSeq(it->id);
        resultSet.eventRecords_.emplace_back(sysEvent);
    }
    resultSet.Set(DOC_STORE_SUCCESS, true);
    return resultSet;
}

std::string SysEventQuery::ToString() const
{
    return ""; //liangyujian
}
} // EventStore
} // namespace HiviewDFX
} // namespace OHOS
