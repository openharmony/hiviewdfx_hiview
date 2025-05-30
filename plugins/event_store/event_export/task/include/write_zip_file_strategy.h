/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef HIVIEW_BASE_EVENT_WRITE_ZIP_FILE_STRAGEGY_H
#define HIVIEW_BASE_EVENT_WRITE_ZIP_FILE_STRAGEGY_H

#include <memory>

#include "event_write_strategy_factory.h"

namespace OHOS {
namespace HiviewDFX {
class WriteZipFileStrategy : public EventWriteBaseStrategy {
public:
    std::string GetPackagerKey(std::shared_ptr<CachedEvent> cachedEvent) override;
    bool Write(std::string& exportContent, WroteCallback callback) override;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIVIEW_BASE_EVENT_WRITE_ZIP_FILE_STRAGEGY_H