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

#ifndef OHOS_HIVIEWDFX_SYS_EVENT_SERVICE_IPC_INTERFACE_CODE_H
#define OHOS_HIVIEWDFX_SYS_EVENT_SERVICE_IPC_INTERFACE_CODE_H

/* SAID: 1203 */
namespace OHOS {
namespace HiviewDFX {
enum class SysEventServiceInterfaceCode {
    ADD_SYS_EVENT_LISTENER = 0,
    REMOVE_SYS_EVENT_LISTENER,
    QUERY_SYS_EVENT,
    SET_DEBUG_MODE,
    ADD_SYS_EVENT_SUBSCRIBER,
    REMOVE_SYS_EVENT_SUBSCRIBER,
    EXPORT_SYS_EVENT
};

enum class SysEventCallbackInterfaceCode {
    HANDLE = 0,
};

enum class QuerySysEventCallbackInterfaceCode {
    ON_QUERY = 0,
    ON_COMPLETE,
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_SYS_EVENT_SERVICE_IPC_INTERFACE_CODE_H
