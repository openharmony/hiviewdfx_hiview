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
#include "sys_event_doc_lru_cache.h"

#include "hiview_logger.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventStore {
DEFINE_LOG_TAG("HiView-SysEventDocLruCache");
bool SysEventDocLruCache::Contain(const LruCacheKey& key) const
{
    return lruCache_.find(key) != lruCache_.end();
}

SysEventDocLruCache::LruCacheValue SysEventDocLruCache::Get(const LruCacheKey& key)
{
    auto iterCache = lruCache_.find(key);
    if (iterCache == lruCache_.end()) {
        return nullptr;
    }
    auto value = iterCache->second.second;
    auto iterList = iterCache->second.first;
    lruList_.erase(iterList);
    lruList_.push_front(iterCache->first);
    iterCache->second.first = lruList_.begin();
    return value;
}

bool SysEventDocLruCache::Add(const LruCacheKey& key, const LruCacheValue& value)
{
    if (Contain(key)) {
        return false;
    }
    lruList_.push_front(key);
    auto valuePair = LruCacheValuePair(lruList_.begin(), value);
    lruCache_.insert(std::make_pair(key, valuePair));
    HIVIEW_LOGD("put sysEventDoc domain=%{public}s, name=%{public}s", key.first.c_str(), key.second.c_str());

    if (lruList_.size() > capacity_) {
        return Remove(lruList_.back());
    }
    return true;
}

bool SysEventDocLruCache::Remove(const LruCacheKey& key)
{
    auto iterCache = lruCache_.find(key);
    if (iterCache == lruCache_.end()) {
        return false;
    }
    lruList_.erase(iterCache->second.first);
    lruCache_.erase(iterCache);
    return true;
}

void SysEventDocLruCache::Clear()
{
    lruList_.clear();
    lruCache_.clear();
}
} // EventStore
} // HiviewDFX
} // OHOS
