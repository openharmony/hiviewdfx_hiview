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

#ifndef JANK_ANIMATOR_REPORTER_ADAPTER_H
#define JANK_ANIMATOR_REPORTER_ADAPTER_H

#include "IScrollJankEventPoster.h"
#include "IJankAnimatorReporter.h"
#include "IJankAnimatorReportInfrastructure.h"

namespace OHOS {
namespace HiviewDFX {
using ScrollJankEventInfo = IScrollJankEventPoster::ScrollJankEventInfo;

class JankAnimatorReporterAdapter : public IJankAnimatorReporter {
public:
    explicit JankAnimatorReporterAdapter(IJankAnimatorReportInfrastructure* impl, IScrollJankEventPoster* eventPoster);
    ~JankAnimatorReporterAdapter() override = default;

    void ReportNormal(const JankAnimatorReportEvent& report) override;
    void ReportCritical(const JankAnimatorReportEvent& report) override;

private:
    IJankAnimatorReportInfrastructure* reporter{nullptr};
    IScrollJankEventPoster* eventPoster{nullptr};

    JankAnimatorReportData ConvertReporterEventToData(const JankAnimatorReportEvent& event);
    ScrollJankEventInfo ConvertReportEventToEventInfo(const JankAnimatorReportEvent& event);
    bool IsScrollJankEvent(const JankAnimatorReportEvent& event);
};
} // HiviewDFX
} // OHOS
#endif // JANK_ANIMATOR_REPORTER_ADAPTER_H