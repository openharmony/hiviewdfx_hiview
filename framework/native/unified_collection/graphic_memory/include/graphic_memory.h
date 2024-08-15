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

#ifndef HIVIEWDFX_HIVIEW_GRAPHIC_MEMORY_H
#define HIVIEWDFX_HIVIEW_GRAPHIC_MEMORY_H

#include <stdint.h>

namespace OHOS {
namespace HiviewDFX {
namespace Graphic{
enum class Type {
    GL,
    GRAPH,
};

enum class ResultCode {
    SUCCESS = 0,
    UNSUPPORT = 1,
    READ_FAILED = 2,
    WRITE_FAILED = 3,
    PERMISSION_CHECK_FAILED = 4,
    SYSTEM_ERROR = 5,
};

struct CollectResult {
public:
    ResultCode retCode = ResultCode::UNSUPPORT;
    int32_t data = 0;
};

CollectResult GetGraphicUsage(int32_t pid);
CollectResult GetGraphicUsage(Type type, int32_t pid);
}
}
}

#endif //HIVIEWDFX_HIVIEW_GRAPHIC_MEMORY_H
