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
#ifndef HIVIEW_SERVICE_ABILITY_PROXY_H
#define HIVIEW_SERVICE_ABILITY_PROXY_H

#include <string>

#include "hiview_file_info.h"
#include "ihiview_service_ability.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace HiviewDFX {
class HiviewServiceAbilityProxy : public IRemoteProxy<IHiviewServiceAbility> {
public:
    explicit HiviewServiceAbilityProxy(const sptr<IRemoteObject> &remote) : IRemoteProxy<IHiviewServiceAbility>(remote)
    {}
    virtual ~HiviewServiceAbilityProxy() = default;

    int32_t List(const std::string& logType, std::vector<HiviewFileInfo>& fileInfos) override;
    int32_t Copy(const std::string& logType, const std::string& logName, const std::string& dest) override;
    int32_t Move(const std::string& logType, const std::string& logName, const std::string& dest) override;
    int32_t Remove(const std::string& logType, const std::string& logName) override;

private:
    int32_t CopyOrMoveFile(
        const std::string& logType, const std::string& logName, const std::string& dest, bool isMove);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_SERVICE_ABILITY_PROXY_H
