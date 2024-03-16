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
 
#ifndef I_CONTENT_READER_H
#define I_CONTENT_READER_H

#include <string>

#include "base_def.h"

namespace OHOS {
namespace HiviewDFX {
class IContentReader {
public:
    virtual int GetContentHead(uint8_t* content, EventStore::ContentHeader& head) = 0;
    virtual size_t GetHeaderSize() = 0;
    virtual ~IContentReader() {};
};
} // HiviewDFX
} // OHOS
#endif // I_CONTENT_READER_H