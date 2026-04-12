/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "hiretrieval_base_util.h"

#include "hiretrieval_base_def.h"
#include "parameter.h"
#include "parameters.h"

namespace OHOS::HiviewDFX {
std::string HiRetrievalBaseUtil::GetDefaultDeviceType()
{
    static std::string deviceType = OHOS::system::GetDeviceType();
    return deviceType;
}

std::string HiRetrievalBaseUtil::GetDefaultDeviceModel()
{
    const char* model = GetProductModel();
    if (model == nullptr) {
        return "";
    }
    return std::string(model);
}
};
