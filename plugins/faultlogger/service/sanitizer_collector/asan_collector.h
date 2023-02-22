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

#ifndef ASAN_COLLECTOR_H
#define ASAN_COLLECTOR_H
#include <cstdio>
#include <string>
#include <unordered_map>

#include "sanitizerd_collector.h"
#include "sanitizerd_log.h"

namespace OHOS {
namespace HiviewDFX {
// asan collector.
class AsanCollector : public SanitizerdCollector {
public:
    explicit AsanCollector(std::unordered_map<std::string, std::string> &stkmap);

    ~AsanCollector() override;
    void Collect(const std::string& filepath);
    bool ComputeStackSignature(const std::string& asanDump,
                               std::string& asanSignature, bool printDiagnostics);

protected:
    virtual bool ReadRecordToString(std::string& fullFile, const std::string& fileName);
    virtual void ProcessStackTrace(const std::string& asanDump, bool printDiagnostics, unsigned *hash);
    int UpdateCollectedData(const std::string& hash, const std::string& rfile);
    bool IsDuplicate(const std::string& hash);
    std::string GetTopStackWithoutCommonLib(const std::string& description);
    void CalibrateErrTypeProcName();
    void SetHappenTime();
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // ASAN_COLLECTOR_H

