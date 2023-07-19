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
#ifndef UNIFIED_COLLECTION_COLLECT_PARAMETER_H
#define UNIFIED_COLLECTION_COLLECT_PARAMETER_H
#include <vector>
#include <string>
#include <map>

namespace OHOS {
namespace HiviewDFX {
class CollectParameter {
    friend class XcollectService;
public:
    explicit CollectParameter(const std::string &caller);
    ~CollectParameter();

public:
    void SetRuntime(const std::string &second = "*", const std::string &minute = "*", const std::string &hour = "*");
    void SetExecTimes(uint32_t times = 1);
    void SetCollectItems(const std::vector<std::string> &items);
    void SetConfig(const std::string &collectModule, const std::string &config, const std::string value);
    void SetConfig(const std::string &collectModule, const std::string &config, uint32_t value);

private:
    const std::string caller_;
    uint32_t times_;
    std::string second_;
    std::string minute_;
    std::string hour_;
    std::vector<std::string> items_;
    std::map<std::string, std::map<std::string, uint32_t>> itemIntConfigs_;
    std::map<std::string, std::map<std::string, std::string>> itemStrConfigs_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // UNIFIED_COLLECTION_COLLECT_PARAMETER_H
