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

#ifndef FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_TRACE_STORAGE_H
#define FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_TRACE_STORAGE_H

#include <memory>

#include "rdb_store.h"
#include "unified_collection_data.h"

namespace OHOS {
namespace HiviewDFX {
class TraceStorage {
public:
    TraceStorage();
    ~TraceStorage() = default;
    void Store(const UcollectionTraceStorage& traceCollection);
    void Query(std::vector<uint64_t>& values);

private:
    void InitDbStore();
    int32_t CreateTable();
    void InsertTable(const UcollectionTraceStorage& traceCollection);
    void GetResultItems(std::vector<uint64_t>& values);

private:
    std::string dbStorePath_;
    std::shared_ptr<NativeRdb::RdbStore> dbStore_;
}; // TraceStorage
} // namespace HiviewDFX
} // namespace OHOS
#endif // FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_TRACE_STORAGE_H