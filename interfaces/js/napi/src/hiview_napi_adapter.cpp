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

#include "hiview_napi_adapter.h"

#include "hiview_napi_err_code.h"
#include "hiview_napi_util.h"
#include "hiview_service_agent.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr size_t ERR_INDEX = 0;
constexpr size_t VAL_INDEX = 1;
constexpr size_t RET_SIZE = 2;
}

void HiviewNapiAdapter::Copy(napi_env env, HiviewFileParams* params)
{
    napi_value resource = nullptr;
    HiviewNapiUtil::CreateStringValue(env, "CopyFileProcess", resource);
    napi_create_async_work(env, nullptr, resource, CopyFileExecution,
        FileOperationCompleteCallback, reinterpret_cast<void*>(params), &params->asyncWork);
    napi_queue_async_work_with_qos(env, params->asyncWork, napi_qos_background);
}

void HiviewNapiAdapter::CopyFileExecution(napi_env env, void* data)
{
    HiviewFileParams* params = reinterpret_cast<HiviewFileParams*>(data);
    params->result = HiviewServiceAgent::Copy(params->logType, params->logName, params->destDir);
}

void HiviewNapiAdapter::Move(napi_env env, HiviewFileParams* params)
{
    napi_value resource = nullptr;
    HiviewNapiUtil::CreateStringValue(env, "MoveFileProcess", resource);
    napi_create_async_work(env, nullptr, resource, MoveFileExecution,
        FileOperationCompleteCallback, reinterpret_cast<void*>(params), &params->asyncWork);
    napi_queue_async_work_with_qos(env, params->asyncWork, napi_qos_background);
}

void HiviewNapiAdapter::MoveFileExecution(napi_env env, void* data)
{
    HiviewFileParams* params = reinterpret_cast<HiviewFileParams*>(data);
    params->result = HiviewServiceAgent::Move(params->logType, params->logName, params->destDir);
}

void HiviewNapiAdapter::FileOperationCompleteCallback(napi_env env, napi_status status, void* data)
{
    HiviewFileParams* params = reinterpret_cast<HiviewFileParams*>(data);
    napi_value results[RET_SIZE] = {0};
    auto isSuccess = (params->result == 0);
    if (isSuccess) {
        HiviewNapiUtil::CreateUndefined(env, results[ERR_INDEX]);
        HiviewNapiUtil::CreateInt32Value(env, params->result, results[VAL_INDEX]);
    } else {
        HiviewNapiUtil::CreateUndefined(env, results[VAL_INDEX]);
        HiviewNapiUtil::CreateErrorByRet(env, params->result, results[ERR_INDEX]);
    }
    if (params->deferred != nullptr) {
        isSuccess ? napi_resolve_deferred(env, params->deferred, results[VAL_INDEX]) :
            napi_reject_deferred(env, params->deferred, results[ERR_INDEX]);
    } else {
        napi_value callback = nullptr;
        napi_get_reference_value(env, params->callback, &callback);
        napi_value retValue = nullptr;
        napi_call_function(env, nullptr, callback, RET_SIZE, results, &retValue);
        napi_delete_reference(env, params->callback);
    }
    napi_delete_async_work(env, params->asyncWork);
    delete params;
}
} // namespace HiviewDFX
} // namespace OHOS