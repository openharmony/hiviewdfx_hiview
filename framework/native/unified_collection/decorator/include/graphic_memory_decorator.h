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

#ifndef HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_GRAPHIC_MEMORY_DECORATOR_H
#define HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_GRAPHIC_MEMORY_DECORATOR_H

#include "decorator.h"
#include "graphic_memory_collector.h"

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
class GraphicMemoryDecorator : public GraphicMemoryCollector, public UCDecorator {
public:
    GraphicMemoryDecorator(std::shared_ptr<GraphicMemoryCollector> collector) : graphicMemoryCollector_(collector) {};
    virtual ~GraphicMemoryDecorator() = default;

public:
    virtual CollectResult<int32_t> GetGraphicUsage(int32_t pid, GraphicType type, bool isLowLatencyMode) override;
    static void SaveStatCommonInfo();
    static void ResetStatInfo();

private:
    std::shared_ptr<GraphicMemoryCollector> graphicMemoryCollector_;
    static StatInfoWrapper statInfoWrapper_;
};
} // namespace UCollectUtil
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_FRAMEWORK_NATIVE_UNIFIED_COLLECTION_GRAPHIC_MEMORY_DECORATOR_H
