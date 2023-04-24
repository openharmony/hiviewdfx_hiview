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

#ifndef BASE_EVENT_RAW_DECODE_INCLUDE_DECODED_PARAM_H
#define BASE_EVENT_RAW_DECODE_INCLUDE_DECODED_PARAM_H

#include <string>
#include <vector>

#include "base/raw_data_base_def.h"
#include "base/value_param.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventRaw {
class DecodedParam : public ValueParam {

public:
    DecodedParam(uint8_t* rawData, const size_t maxLen, size_t& pos, const std::string& key)
        : rawData_(rawData), maxLen_(maxLen), pos_(pos), key_(key)
    {}

    virtual ~DecodedParam()
    {}

public:
    virtual std::string GetKey();
    virtual size_t GetPosition();

public:
    virtual bool AsUint64(uint64_t& dest) override;
    virtual bool AsInt64(int64_t& dest) override;
    virtual bool AsDouble(double& dest) override;
    virtual bool AsString(std::string& dest) override;
    virtual bool AsUint64Vec(std::vector<uint64_t>& dest) override;
    virtual bool AsInt64Vec(std::vector<int64_t>& dest) override;
    virtual bool AsDoubleVec(std::vector<double>& dest) override;
    virtual bool AsStringVec(std::vector<std::string>& dest) override;

public:
    virtual bool DecodeValue() = 0;

protected:
    uint8_t* rawData_;
    size_t maxLen_;
    size_t pos_;
    std::string key_;
};

class UnsignedVarintDecodedParam : public DecodedParam {
public:
    UnsignedVarintDecodedParam(uint8_t* rawData, const size_t maxLen, size_t& pos, const std::string& key)
        : DecodedParam(rawData, maxLen, pos, key) {}

    virtual bool DecodeValue() override;
    virtual DataCodedType GetDataCodedType() override;
    virtual bool AsUint64(uint64_t& dest) override;

private:
    uint64_t val_;
};

class UnsignedVarintDecodedArrayParam : public DecodedParam {
public:
    UnsignedVarintDecodedArrayParam(uint8_t* rawData, const size_t maxLen, size_t& pos, const std::string& key)
        : DecodedParam(rawData, maxLen, pos, key) {}

    virtual bool DecodeValue() override;
    virtual DataCodedType GetDataCodedType() override;
    virtual bool AsUint64Vec(std::vector<uint64_t>& dest) override;

private:
    std::vector<uint64_t> vals_;
};

class SignedVarintDecodedParam : public DecodedParam {
public:
    SignedVarintDecodedParam(uint8_t* rawData, const size_t maxLen, size_t& pos, const std::string& key)
        : DecodedParam(rawData, maxLen, pos, key) {}

    virtual bool DecodeValue() override;
    virtual DataCodedType GetDataCodedType() override;
    virtual bool AsInt64(int64_t& dest) override;

private:
    int64_t val_;
};

class SignedVarintDecodedArrayParam : public DecodedParam {
public:
    SignedVarintDecodedArrayParam(uint8_t* rawData, const size_t maxLen, size_t& pos, const std::string& key)
        : DecodedParam(rawData, maxLen, pos, key) {}

    virtual bool DecodeValue() override;
    virtual DataCodedType GetDataCodedType() override;
    virtual bool AsInt64Vec(std::vector<int64_t>& dest) override;

private:
    std::vector<int64_t> vals_;
};

class FloatingNumberDecodedParam : public DecodedParam {
public:
    FloatingNumberDecodedParam(uint8_t* rawData, const size_t maxLen, size_t& pos, const std::string& key)
        : DecodedParam(rawData, maxLen, pos, key) {}

    virtual bool DecodeValue() override;
    virtual DataCodedType GetDataCodedType() override;
    virtual bool AsDouble(double& dest) override;

private:
    double val_;
};

class FloatingNumberDecodedArrayParam : public DecodedParam {
public:
    FloatingNumberDecodedArrayParam(uint8_t* rawData, const size_t maxLen, size_t& pos, const std::string& key)
        : DecodedParam(rawData, maxLen, pos, key) {}

    virtual bool DecodeValue() override;
    virtual DataCodedType GetDataCodedType() override;
    virtual bool AsDoubleVec(std::vector<double>& dest) override;

private:
    std::vector<double> vals_;
};

class StringDecodedParam : public DecodedParam {
public:
    StringDecodedParam(uint8_t* rawData, const size_t maxLen, size_t& pos, const std::string& key)
        : DecodedParam(rawData, maxLen, pos, key) {}

    virtual bool DecodeValue() override;
    virtual DataCodedType GetDataCodedType() override;
    virtual bool AsString(std::string& dest) override;

private:
    std::string val_;
};

class StringDecodedArrayParam : public DecodedParam {
public:
    StringDecodedArrayParam(uint8_t* rawData, const size_t maxLen, size_t& pos, const std::string& key)
        : DecodedParam(rawData, maxLen, pos, key) {}

    virtual bool DecodeValue() override;
    virtual DataCodedType GetDataCodedType() override;
    virtual bool AsStringVec(std::vector<std::string>& dest) override;

private:
    std::vector<std::string> vals_;
};
} // namespace EventRaw
} // namespace HiviewDFX
} // namespace OHOS

#endif // BASE_EVENT_RAW_DECODE_INCLUDE_DECODED_PARAM_H