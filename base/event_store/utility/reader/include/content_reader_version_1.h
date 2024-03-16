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
 
#ifndef CONTENT_READER_VERSION_1_H
#define CONTENT_READER_VERSION_1_H

#include <string>

#include "i_content_reader.h"

namespace OHOS {
namespace HiviewDFX {
class ContentReaderVersion1 : public IContentReader {
public:
    int GetContentHead(uint8_t* content, EventStore::ContentHeader& head) override;
    size_t GetHeaderSize() override;
};
} // HiviewDFX
} // OHOS
#endif // CONTENT_READER_VERSION_1_H