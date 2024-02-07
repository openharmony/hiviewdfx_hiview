/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef PARAM_CONST_COMMON_H
#define PARAM_CONST_COMMON_H

#include <string>
#include <stdio.h>

namespace OHOS {
namespace HiviewDFX {
    const std::string CONFIG_TYPE = "HIVIEWPARA";
    const std::string SUB_TYPE = "CHINA";
    const std::string CONFIG_UPDATED_ACTION = "usual.event.DUE_SA_CFG_UPDATED";
    const std::string CFG_PATH = "/data/service/el1/public/update/param_service/install/system/etc/HIVIEWPARA/CHINA";
    const std::string PUBKEY_PATH = "/system/etc/hiview/hwkey_param_upgrade_hiviewdfx_v1.pem";
    const std::string LOCAL_CFG_PATH = "/system/etc/hiview";
    const std::string CLOUD_CFG_PATH = "/data/system/hiview";
}
}
#endif
