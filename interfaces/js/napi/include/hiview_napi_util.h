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

#ifndef HIVIEW_NAPI_UTIL_H
#define HIVIEW_NAPI_UTIL_H

#include <string>

#include "hiview_file_info.h"
#include "napi/native_api.h"

namespace OHOS {
namespace HiviewDFX {
class HiviewNapiUtil {
public:
    static void CreateUndefined(const napi_env env, napi_value& ret);
    static void CreateInt32Value(const napi_env env, int32_t value, napi_value& ret);
    static void CreateStringValue(const napi_env env, const std::string& value, napi_value& ret);
    static bool IsMatchType(napi_env env, const napi_value& value, napi_valuetype type);
    static bool ParseStringValue(
        const napi_env env, const std::string& paramName, const napi_value& value, std::string& retValue);
    static napi_value GenerateFileInfoResult(const napi_env env, const std::vector<HiviewFileInfo>& fileInfos);
    static void ThrowParamTypeError(napi_env env, const std::string& paramName, const std::string& paramType);
    static void ThrowParamContentError(napi_env env, const std::string& paramName);
    static void ThrowErrorByCode(napi_env env, int32_t errCode);
    static void ThrowSystemAppPermissionError(napi_env env);
    static bool CheckDirPath(const std::string& path);
    static void CreateErrorByRet(napi_env env, const int32_t retCode, napi_value& ret);
    static bool IsSystemAppCall();

private:
    HiviewNapiUtil() = default;
    ~HiviewNapiUtil() = default;

    static void CreateInt64Value(const napi_env env, int64_t value, napi_value& ret);
    static void CreateJsFileInfo(const napi_env env, const HiviewFileInfo& fileInfo, napi_value& val);
    static void SetNamedProperty(
        const napi_env env, napi_value& object, const std::string& propertyName, napi_value& propertyValue);
    static void ThrowError(napi_env env, const int32_t code, const std::string& msg);
    static std::pair<int32_t, std::string> GetErrorDetailByRet(napi_env env, const int32_t retCode);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif