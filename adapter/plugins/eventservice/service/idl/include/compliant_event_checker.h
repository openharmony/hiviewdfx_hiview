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

#ifndef OHOS_HIVIEWDFX_COMPLIANT_EVENT_CHECKER_H
#define OHOS_HIVIEWDFX_COMPLIANT_EVENT_CHECKER_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
class CompliantEventChecker {
public:
    CompliantEventChecker();
    ~CompliantEventChecker() = default;

public:
    bool IsCompliantEvent(const std::string& domain, const std::string& eventName);

private:
    bool isSecureEnabeled_ = false;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // OHOS_HIVIEWDFX_COMPLIANT_EVENT_CHECKER_H