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
#ifndef I_APP_TIMER_H
#define I_APP_TIMER_H

#include <string>
#include "IStrKeyTimer.h"

namespace OHOS {
namespace HiviewDFX {
class IAppTimer : public IStrKeyTimer {
public:
    virtual ~IAppTimer() = default;
};
} // HiviewDFX
} // OHOS
#endif
