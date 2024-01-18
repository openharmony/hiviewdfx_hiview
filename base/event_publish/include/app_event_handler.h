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
#ifndef HIVIEW_BASE_APP_EVENT_HANDLER_H
#define HIVIEW_BASE_APP_EVENT_HANDLER_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
class AppEventHandler {
public:
    struct BundleInfo {
        std::string bundleName;
        std::string bundleVersion;
    };

    struct ProcessInfo {
        std::string processName;
    };

    struct AppLaunchInfo : public BundleInfo, public ProcessInfo {
        int32_t startType = 0;
        uint64_t iconInputTime = 0;
        uint64_t animationFinishTime = 0;
        uint64_t extentTime = 0;
    };

    int PostEvent(const AppLaunchInfo& event);
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIVIEW_BASE_APP_EVENT_HANDLER_H