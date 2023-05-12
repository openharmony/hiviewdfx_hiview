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

#ifndef HIVIEW_BASE_EVENT_STORE_SYS_EVENT_DOC_LRU_CACHE_H
#define HIVIEW_BASE_EVENT_STORE_SYS_EVENT_DOC_LRU_CACHE_H

#include <list>
#include <memory>

#include <string>
#include <unordered_map>

#include "sys_event_doc.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
class SysEventDocLruCache {
public:
    typedef std::pair<std::string, std::string> LruCacheKey;
    typedef std::shared_ptr<SysEventDoc> LruCacheValue;

    explicit SysEventDocLruCache(size_t capacity) : capacity_(capacity) {}
    ~SysEventDocLruCache() {}

    bool Contain(const LruCacheKey& key) const;
    LruCacheValue Get(const LruCacheKey& key);
    bool Add(const LruCacheKey& key, const LruCacheValue& value);
    bool Remove(const LruCacheKey& key);

private:
    typedef std::pair<std::list<LruCacheKey>::iterator, LruCacheValue> LruCacheValuePair;
    struct LruCacheKeyHashfunc{
        size_t operator() (const std::pair<std::string, std::string>& keyPair) const
        {
            return std::hash<std::string>()(keyPair.first) ^ std::hash<std::string>()(keyPair.second);
        }
    };

    size_t capacity_;
    std::list<LruCacheKey> lruList_;
    std::unordered_map<LruCacheKey, LruCacheValuePair, LruCacheKeyHashfunc> lruCache_;
}; // SysEventDocLruCache
} // EventStore
} // HiviewDFX
} // OHOS
#endif // HIVIEW_BASE_EVENT_STORE_SYS_EVENT_DOC_LRU_CACHE_H