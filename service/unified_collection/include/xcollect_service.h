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
#ifndef UNIFIED_COLLECTION_XCOLLECT_SERVICE_H
#define UNIFIED_COLLECTION_XCOLLECT_SERVICE_H
#include <string>
#include "collect_callback.h"
#include "collect_parameter.h"
#include "collect_item_result.h"

constexpr int FILE_RIGHT_READ = 00644;

namespace OHOS {
namespace HiviewDFX {
class XcollectService {
public:
    XcollectService(std::shared_ptr<CollectParameter> collectParameter);
    XcollectService(std::shared_ptr<CollectParameter> collectParameter, std::shared_ptr<CollectCallback> callback);
    ~XcollectService();

public:
    void StartCollect();

private:
    void CollectHiTrace(const std::string &item);
    static std::string DumpTraceToDir();

private:
    std::shared_ptr<CollectParameter> collectParameter_;
    std::shared_ptr<CollectCallback> callback_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // UNIFIED_COLLECTION_XCOLLECT_SERVICE_H
