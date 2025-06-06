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
#ifndef FAULTLOG_PROCESSOR_FACTORY_H
#define FAULTLOG_PROCESSOR_FACTORY_H

#include "functional"

#include "faultlog_processor_interface.h"

namespace OHOS {
namespace HiviewDFX {
class FaultLogProcessorFactory {
public:
    FaultLogProcessorFactory();
    std::unique_ptr<FaultLogProcessorInterface> CreateFaultLogProcessor(FaultLogType type);
private:
    using FaultLogProcessor = std::function<std::unique_ptr<FaultLogProcessorInterface>()>;
    std::unordered_map<FaultLogType, FaultLogProcessor> faultLogProcessors;
    void InitializeProcessors();
};
} // namespace HiviewDFX
} // namespace OHOS
#endif
