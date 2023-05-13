/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef IHIVIEW_SERVICE_ABILITY_H
#define IHIVIEW_SERVICE_ABILITY_H

#include <string>

#include "hiview_file_info.h"
#include "iremote_broker.h"

namespace OHOS {
namespace HiviewDFX {
class IHiviewServiceAbility : public IRemoteBroker {
public:
    IHiviewServiceAbility() = default;
    ~IHiviewServiceAbility() = default;

    virtual int32_t List(const std::string& logType, std::vector<HiviewFileInfo>& fileInfos) = 0;
    virtual int32_t Copy(const std::string& logType, const std::string& logName, const std::string& dest) = 0;
    virtual int32_t Move(const std::string& logType, const std::string& logName, const std::string& dest) = 0;
    virtual int32_t Remove(const std::string& logType, const std::string& logName) = 0;

    enum {
        HIVIEW_SERVICE_ID_LIST = 1001,
        HIVIEW_SERVICE_ID_COPY = 1002,
        HIVIEW_SERVICE_ID_MOVE = 1003,
        HIVIEW_SERVICE_ID_REMOVE = 1004
    };

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.HiviewDFX.IHiviewServiceAbility");
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // IHIVIEW_SERVICE_H