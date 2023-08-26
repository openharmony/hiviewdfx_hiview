/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "common_util.h"
#include "logger.h"
#include "file_util.h"

DEFINE_LOG_TAG("UCollectUtil");


namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
template <typename T> bool CommonUtil::StrToNum(const std::string &sString, T &tX)
{
    std::istringstream iStream(sString);
    return (iStream >> tX) ? true : false;
}

bool CommonUtil::ParseTypeAndValue(const std::string &str, std::string &type, int32_t &value)
{
    std::string::size_type typePos = str.find(":");
    if (typePos != std::string::npos) {
        type = str.substr(0, typePos);
        std::string valueStr = str.substr(typePos + 1);
        std::string::size_type valuePos = valueStr.find("kB");
        HIVIEW_LOGD("valuePos=%{public}d", valuePos);
        if (valuePos == std::string::npos) {
            valuePos = valueStr.find("KB");
            HIVIEW_LOGD("valuePos=%{public}d", valuePos);
        }
        if (valuePos != std::string::npos) {
            valueStr.resize(valuePos);
            StrToNum(valueStr, value);
            HIVIEW_LOGD("value=%{public}d", value);
            return true;
        } else {
            StrToNum(valueStr, value);
            HIVIEW_LOGD("value=%{public}d", value);
            return true;
        }
    }
    return false;
}
} // UCollectUtil
} // HiViewDFX
} // OHOS