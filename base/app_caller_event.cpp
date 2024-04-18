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

#include "app_caller_event.h"

namespace OHOS {
namespace HiviewDFX {
bool AppCallerEvent::isDynamicTraceOpen_ = false;
bool AppCallerEvent::enableDynamicTrace_ = false;

AppCallerEvent::AppCallerEvent(const std::string &sender): Event(sender), 
    bundleName_(""), bundleVersion_(""), uid_(0), pid_(0), beginTime_(0), endTime_(0), 
    resultCode_(0), taskBeginTime_(0), taskEndTime_(0), externalLog_("")
{
}

bool AppCallerEvent::IsEnableAppCaptureTrace()
{
    return AppCallerEvent::enableDynamicTrace_;
}
} // HiviewDFX
} // OHOS