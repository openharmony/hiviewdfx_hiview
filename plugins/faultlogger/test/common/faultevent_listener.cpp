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

#include "faultevent_listener.h"

#include <cstring>
#include <iostream>

namespace OHOS {
namespace HiviewDFX {
void FaultEventListener::OnEvent(std::shared_ptr<HiSysEventRecord> sysEvent)
{
    if (sysEvent == nullptr) {
        return;
    }
    auto str = sysEvent->AsJson();
    jsonStr.emplace_back(str);
}

bool FaultEventListener::CheckKeywords(const std::vector<std::string> keywords)
{
    for (auto str : jsonStr) {
        bool flag = true;
        for (auto keyword : keywords) {
            if (str.find(keyword) == std::string::npos) {
                flag = false;
            }
        }
        if (flag) {
            return true;
        }
    }
    return false;
}
} // namespace HiviewDFX
} // namespace OHOS