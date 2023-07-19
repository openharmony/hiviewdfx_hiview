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
#ifndef UNIFIED_COLLECTION_COLLECT_ITEM_RESULT_H
#define UNIFIED_COLLECTION_COLLECT_ITEM_RESULT_H
#include <cinttypes>
#include <map>
#include <string>

namespace OHOS {
namespace HiviewDFX {
struct CollectItemValue {
    int32_t intValue_;
    std::string strValue_;
    std::string unit_;
};

class CollectItemResult {
public:
    CollectItemResult();
    ~CollectItemResult();

    void SetCollectItemValue(const std::string& item, std::string &value);
    void GetCollectItemValue(const std::string& item, std::string &value);
    void GetCollectItemValue(const std::string& item, std::string &value, std::string &unit);
    void GetCollectItemValue(const std::string& item, int32_t &value);
    void GetCollectItemValue(const std::string& item, int32_t &value, std::string &unit);

private:
    std::map<std::string, CollectItemValue> itemValues_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // UNIFIED_COLLECTION_COLLECT_ITEM_RESULT_H
