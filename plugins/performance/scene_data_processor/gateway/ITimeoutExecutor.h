/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef I_TIMEOUT_EXECUTOR_H
#define I_TIMEOUT_EXECUTOR_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
class ITimeoutExecutor {
public:
    class ITimeoutHandler {
    public:
        virtual ~ITimeoutHandler() = default;

        virtual void HandleTimeoutInMainThr(std::string name) = 0;
    };

    virtual void ExecuteTimeoutInMainThr(ITimeoutHandler* task, std::string name) = 0;
    virtual ~ITimeoutExecutor() = default;
};
} // HiviewDFX
} // OHOS
#endif
