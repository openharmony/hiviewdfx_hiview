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
#ifndef NATIVE_LEAK_CONFIG_H
#define NATIVE_LEAK_CONFIG_H

#include <string>
#include <unordered_map>

namespace OHOS {
namespace HiviewDFX {

class NativeLeakConfig {
public:
    NativeLeakConfig();
    ~NativeLeakConfig();
    bool GetThresholdList(std::unordered_map<std::string, uint64_t> &list);
};
} // HiviewDFX
} // OHOS
#endif // NATIVE_LEAK_CONFIG_H
