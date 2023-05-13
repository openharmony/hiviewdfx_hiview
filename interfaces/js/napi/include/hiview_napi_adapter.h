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

#ifndef HIVIEW_NAPI_ADAPTER_H
#define HIVIEW_NAPI_ADAPTER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace HiviewDFX {
struct HiviewFileParams {
    HiviewFileParams(const std::string& logType, const std::string& logName, const std::string& destDir)
        : logType(logType), logName(logName), destDir(destDir) {};
    napi_async_work asyncWork {nullptr};
    napi_deferred deferred {nullptr};
    napi_ref callback {nullptr};
    int32_t result {0};
    std::string logType;
    std::string logName;
    std::string destDir;
};

class HiviewNapiAdapter {
public:
    static void Copy(napi_env env, HiviewFileParams* params);
    static void Move(napi_env env, HiviewFileParams* params);

private:
    HiviewNapiAdapter() = default;
    ~HiviewNapiAdapter() = default;
    
    static void CopyFileExecution(napi_env env, void* data);
    static void MoveFileExecution(napi_env env, void* data);
    static void FileOperationCompleteCallback(napi_env env, napi_status status, void* data);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif