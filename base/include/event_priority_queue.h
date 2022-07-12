/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef HIVIEW_BASE_EVENT_PRIORITY_QUEUE_H
#define HIVIEW_BASE_EVENT_PRIORITY_QUEUE_H
#include <queue>
namespace OHOS {
namespace HiviewDFX {
using Task = std::function<void()>;
template<typename T>
class EventPriorityQueue : public std::priority_queue<T, std::vector<T>> {
public:
    bool remove(uint64_t seq)
    {
        auto it = std::find_if(this->c.begin(), this->c.end(), [seq](T event) {
            return event.seq == seq;
        });
        if (it != this->c.end()) {
            this->c.erase(it);
            std::make_heap(this->c.begin(), this->c.end(), this->comp);
            return true;
        } else {
            return false;
        };
    };

    void ShrinkIfNeedLocked()
    {
        if ((this->c.capacity() / this->c.size()) > 10) {   // 10 times, begin to shrink
            this->c.shrink_to_fit();
        }
    }
};
}  // namespace HiviewDFX
}  // namespace OHOS
#endif  // HIVIEW_BASE_EVENT_LOOP_H