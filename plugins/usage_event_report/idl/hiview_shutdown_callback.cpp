/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "hiview_shutdown_callback.h"

#include "usage_event_report.h"
#include "logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-ShutdownCallback");

void HiViewShutdownCallback::ShutdownCallback()
{
    HIVIEW_LOGI("hiview shutdown callback start");
    UsageEventReport::SaveEventToDb();
    HIVIEW_LOGI("hiview shutdown callback end");
}
} // namespace HiviewDFX
} // namespace OHOS
