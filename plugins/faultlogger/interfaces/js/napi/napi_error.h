/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef HIVIEWDFX_FAULTLOGGER_NAPI_ERROR_H
#define HIVIEWDFX_FAULTLOGGER_NAPI_ERROR_H

namespace OHOS {
namespace HiviewDFX {
namespace NapiError {
constexpr int ERR_INPUT_PARAM = 401; // invalid input parameter
constexpr int ERR_SYSTEM_CAPABILITY = 801; // the specified SystemCapability name was not found
constexpr int ERR_SERVICE_STATUS = 10600001; // service is not running or broken
} // namespace NapiError
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEWDFX_FAULTLOGGER_NAPI_ERROR_H
