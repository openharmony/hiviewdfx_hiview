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
#include "doc_query.h"

#include <unordered_set>

// #include "json/json.h"
#include "logger.h"
#include "sys_event_query.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
DEFINE_LOG_TAG("HiView-DocQuery");
namespace {
// bool ParseJsonString(const std::string& eventJson, Json::Value& root)
// {
//     Json::CharReaderBuilder jsonRBuilder;
//     Json::CharReaderBuilder::strictMode(&jsonRBuilder.settings_);
//     std::unique_ptr<Json::CharReader> const reader(jsonRBuilder.newCharReader());
//     JSONCPP_STRING errs;
//     if (!reader->parse(eventJson.data(), eventJson.data() + eventJson.size(), &root, &errs)) {
//         HIVIEW_LOGE("failed to parse eventJson: %{public}s.", eventJson.c_str());
//         return false;
//     }
//     return true;
// }
}

std::pair<std::string, bool> DocQuery::GetOrderField()
{
    return orderField_;
}

void DocQuery::SetOrderField(const std::string& col, bool isAsc)
{
    orderField_ = std::make_pair<>(col, isAsc);
}

void DocQuery::And(const Cond& cond)
{
    if (IsInnerCond(cond)) {
        innerConds_.push_back(cond);
        return;
    }
    extraConds_.push_back(cond);
}

bool DocQuery::IsInnerCond(const Cond& cond) const
{
    const std::unordered_set<std::string> innerFields = {
        EventCol::SEQ, EventCol::TS, EventCol::TZ,
        EventCol::PID, EventCol::TID, EventCol::UID,
    };
    return innerFields.find(cond.col_) != innerFields.end();
}

bool DocQuery::IsContainCond(const Cond& cond, const FieldValue& value) const
{
    switch (cond.op_) {
        case EQ:
            return value == cond.fieldValue_;
        case NE:
            return value != cond.fieldValue_;
        case GT:
            return value > cond.fieldValue_;
        case GE:
            return value >= cond.fieldValue_;
        case LT:
            return value < cond.fieldValue_;
        case LE:
            return value <= cond.fieldValue_;
        case SW:
            return value.IsStartWith(cond.fieldValue_);
        case NSW:
            return value.IsNotStartWith(cond.fieldValue_);
        default:
            return false;
    }
}

bool DocQuery::IsContainInnerCond(const InnerFieldStruct& innerField, const Cond& cond) const
{
    FieldValue value;
    if (cond.col_ == EventCol::SEQ) {
        value = innerField.seq;
    } else if (cond.col_ == EventCol::TS) {
        value = static_cast<int64_t>(innerField.ts);
    } else if (cond.col_ == EventCol::TZ) {
        value = static_cast<int64_t>(innerField.tz);
    } else if (cond.col_ == EventCol::UID) {
        value = static_cast<int64_t>(innerField.uid);
    } else if (cond.col_ == EventCol::PID) {
        value = static_cast<int64_t>(innerField.pid);
    } else if (cond.col_ == EventCol::TID) {
        value = static_cast<int64_t>(innerField.tid);
    } else {
        return false;
    }
    return IsContainCond(cond, value);
}

bool DocQuery::IsContainInnerConds(uint8_t* content) const
{
    HIVIEW_LOGI("liangyujian start 1");
    if (innerConds_.empty()) {
        return true;
    }
    InnerFieldStruct innerField = *(reinterpret_cast<InnerFieldStruct*>(content + BLOCK_SIZE));
    // HIVIEW_LOGI("liangyujian innerField.seq=%{public}lld", innerField.seq);
    // HIVIEW_LOGI("liangyujian innerField.ts=%{public}llu", innerField.ts);
    // HIVIEW_LOGI("liangyujian innerField.tz=%{public}u", innerField.tz);
    // HIVIEW_LOGI("liangyujian innerField.uid=%{public}u", innerField.uid);
    // HIVIEW_LOGI("liangyujian innerField.pid=%{public}u", innerField.pid);
    // HIVIEW_LOGI("liangyujian innerField.tid=%{public}u", innerField.tid);
    HIVIEW_LOGI("liangyujian innerConds_.size=%{public}zu", innerConds_.size());
    return std::all_of(innerConds_.begin(), innerConds_.end(), [this, &innerField] (auto& cond) {
        return IsContainInnerCond(innerField, cond);
    });
}

bool DocQuery::IsContainExtraConds(uint8_t* content, uint32_t contentSize) const
{
    // liangyujian
    return true;
}

bool DocQuery::IsContain(uint8_t* content, uint32_t contentSize) const
{
    return IsContainInnerConds(content) && IsContainExtraConds(content, contentSize);
}

std::string DocQuery::ToString() const
{
    return "liangyujian";
}
}; // DocQuery
} // HiviewDFX
} // OHOS
