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
#ifndef FAULT_LEVEL_THREAD_HOLD_H
#define FAULT_LEVEL_THREAD_HOLD_H

#include <string>
#include <climits>

namespace OHOS {
namespace HiviewDFX {
const int DEFAULT_LEVEL_A = UINT_MAX;
const int DEFAULT_LEVEL_B = UINT_MAX;
const int DEFAULT_LEVEL_C = UINT_MAX;
const std::string DEFAULT_COMMENT = "";
const int DEFAULT_DUMP_BITMAP = 0;

struct FaultLevelThreshold {
    unsigned int levelA{UINT_MAX}; /* A standard --total time span,0-invalid,unit:ms */
    unsigned int levelB{UINT_MAX}; /* B standard --total time span,0-invalid,unit:ms */
    unsigned int levelC{UINT_MAX}; /* C standard --total time span,0-invalid,unit:ms */
    unsigned int dumpBitmap{DEFAULT_DUMP_BITMAP};
    std::string comment{DEFAULT_COMMENT};
    FaultLevelThreshold()
    {
        this->levelA = DEFAULT_LEVEL_A;
        this->levelB = DEFAULT_LEVEL_B;
        this->levelC = DEFAULT_LEVEL_C;
        this->comment = DEFAULT_COMMENT;
        this->dumpBitmap = DEFAULT_DUMP_BITMAP;
    }
    FaultLevelThreshold(const FaultLevelThreshold& faultLevelThreshold)
    {
        this->levelA = faultLevelThreshold.levelA;
        this->levelB = faultLevelThreshold.levelB;
        this->levelC = faultLevelThreshold.levelC;
        this->comment = faultLevelThreshold.comment;
        this->dumpBitmap = faultLevelThreshold.dumpBitmap;
    }
    FaultLevelThreshold& operator=(const FaultLevelThreshold& faultLevelThreshold)
    {
        this->levelA = faultLevelThreshold.levelA;
        this->levelB = faultLevelThreshold.levelB;
        this->levelC = faultLevelThreshold.levelC;
        this->comment = faultLevelThreshold.comment;
        this->dumpBitmap = faultLevelThreshold.dumpBitmap;
        return *this;
    }
};
} // HiviewDFX
} // OHOS
#endif