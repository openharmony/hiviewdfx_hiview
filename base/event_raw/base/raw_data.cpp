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

#include "base/raw_data.h"

#include <new>

#include "hilog/log.h"
#include "securec.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventRaw {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002D10, "HiView-RawData" };
constexpr size_t EXPAND_BUF_SIZE = 512;
}

RawData::RawData()
{
    data_ = new uint8_t[EXPAND_BUF_SIZE];
    capacity_ = EXPAND_BUF_SIZE;
    len_ = 0;
}

RawData::RawData(uint8_t* data, size_t dataLen)
{
    if (data == nullptr || dataLen == 0) {
        data_ = new uint8_t[EXPAND_BUF_SIZE];
        capacity_ = EXPAND_BUF_SIZE;
        len_ = 0;
        return;
    }
    data_ = new uint8_t[dataLen];
    auto ret = memcpy_s(data_, dataLen, data, dataLen);
    if (ret != EOK) {
        HiLog::Error(LABEL, "Failed to copy RawData in constructor, ret is %{public}d.", ret);
        return;
    }
    capacity_ = dataLen;
    len_ = dataLen;
}

RawData::RawData(const RawData& data)
{
    auto dataLen = data.GetDataLength();
    if (dataLen == 0) {
        return;
    }
    data_ = new uint8_t[dataLen];
    auto ret = memcpy_s(data_, dataLen, data.GetData(), dataLen);
    if (ret != EOK) {
        HiLog::Error(LABEL, "Failed to copy RawData in constructor, ret is %{public}d.", ret);
        return;
    }
    capacity_ = dataLen;
    len_ = dataLen;
}

RawData& RawData::operator=(const RawData& data)
{
    if (this == &data) {
        return *this;
    }
    auto dataLen = data.GetDataLength();
    if (dataLen == 0) {
        return *this;
    }
    uint8_t* tmpData = new uint8_t[dataLen];
    auto ret = memcpy_s(tmpData, dataLen, data.GetData(), dataLen);
    if (ret != EOK) {
        HiLog::Error(LABEL, "Failed to copy RawData in constructor, ret is %{public}d.", ret);
        return *this;
    }
    if (data_ != nullptr) {
        delete []data_;
    }
    data_ = tmpData;
    capacity_ = dataLen;
    len_ = dataLen;
    return *this;
}

RawData::~RawData()
{
    if (data_ != nullptr) {
        delete []data_;
        data_ = nullptr;
    }
}

void RawData::Reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (data_ != nullptr) {
        delete []data_;
        data_ = nullptr;
    }
    data_ = new uint8_t[EXPAND_BUF_SIZE];
    capacity_ = EXPAND_BUF_SIZE;
    len_ = 0;
}

bool RawData::Append(uint8_t* data, size_t len)
{
    if (len == 0) {
        return true;
    }
    return Update(data, len, len_);
}

bool RawData::IsEmpty()
{
    return len_ == 0 || data_ == nullptr;
}

bool RawData::Update(uint8_t* data, size_t len, size_t pos)
{
    if (data == nullptr || pos > len_) {
        HiLog::Error(LABEL, "Invalid memory operation: length is %{public}zu, pos is %{public}zu, "
            "len_ is %{public}zu", len, pos, len_);
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto ret = EOK;
    if ((pos + len) > capacity_) {
        size_t expandedSize = (len > EXPAND_BUF_SIZE) ? len : EXPAND_BUF_SIZE;
        uint8_t* resizeData = new uint8_t[capacity_ + expandedSize];
        ret = memcpy_s(resizeData, len_, data_, len_);
        if (ret != EOK) {
            HiLog::Error(LABEL, "Failed to expand capacity of raw data, ret is %{public}d.", ret);
            delete []resizeData;
            return false;
        }
        capacity_ += expandedSize;
        if (data_ != nullptr) {
            delete []data_;
        }
        data_ = resizeData;
    }
    // append new data
    ret = memcpy_s(data_ + pos, len, data, len);
    if (ret != EOK) {
        HiLog::Error(LABEL, "Failed to append new data, ret is %{public}d.", ret);
        return false;
    }
    if ((pos + len) > len_) {
        len_ = pos + len;
        HiLog::Debug(LABEL, "Length of data is expanded to %{public}zu.", len_);
    }
    return true;
}

uint8_t* RawData::GetData() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return data_;
}

size_t RawData::GetDataLength() const
{
    return len_;
}
} // namespace EventRaw
} // namespace HiviewDFX
} // namespace OHOS