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
 
#ifndef CONTENT_READER_FACTORY_H
#define CONTENT_READER_FACTORY_H

#include <unordered_map>

#include "content_reader.h"
#include "singleton.h"

namespace OHOS {
namespace HiviewDFX {
class ContentReaderFactory : public OHOS::DelayedRefSingleton<ContentReaderFactory> {
public:
    void Register(int8_t version, std::shared_ptr<ContentReader> reader);
    std::shared_ptr<ContentReader> Get(int8_t version);

private:
    std::unordered_map<int8_t, std::shared_ptr<ContentReader>> readerMap_;
};

class ReaderRegister {
public:
    ReaderRegister(int8_t version, std::shared_ptr<ContentReader> reader)
    {
        ContentReaderFactory::GetInstance().Register(version, reader);
    }
    ~ReaderRegister(){};
};

#define REGISTER_CONTENT_READER(version, className) \
class Register##className { \
private: \
    static const ReaderRegister g_readerGegister; \
}; \
const ReaderRegister Register##className::g_readerGegister(version, std::make_shared<className>())

} // HiviewDFX
} // OHOS

#endif // CONTENT_READER_FACTORY_H