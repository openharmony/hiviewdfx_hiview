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

#include "JankAnimatorReporterAdapter.h"

#include "hiview_logger.h"
DEFINE_LOG_TAG("XPerformance");

JankAnimatorReporterAdapter::JankAnimatorReporterAdapter(
    IJankAnimatorReportInfrastructure* impl, IScrollJankEventPoster* eventPoster)
{
    this->reporter = impl;
    this->eventPoster = eventPoster;
}

void JankAnimatorReporterAdapter::ReportNormal(const JankAnimatorReportEvent& event)
{
    HIVIEW_LOGD("[JankAnimatorReporterAdapter::ReportNormal]");
    try {
        if (reporter != nullptr) {
            JankAnimatorReportData data = ConvertReporterEventToData(event);
            reporter->ReportNormal(data);
        } else {
            HIVIEW_LOGE("[JankAnimatorReporterAdapter ReportNormal] report is null");
        }

        if ((eventPoster != nullptr) && IsScrollJankEvent(event)) {
            ScrollJankEventInfo evt = ConvertReportEventToEventInfo(event);
            HIVIEW_LOGD("[JankAnimatorReporterAdapter::ReportNormal] PostScrollJankEvent begin");
            eventPoster->PostScrollJankEvent(evt);
            HIVIEW_LOGD("[JankAnimatorReporterAdapter::ReportNormal] PostScrollJankEvent end");
        }
    }
    catch (std::logic_error& ex) {
        HIVIEW_LOGE("[JankAnimatorReporterAdapter ReportNormal] exception: %{public}s", ex.what());
    }
}

void JankAnimatorReporterAdapter::ReportCritical(const JankAnimatorReportEvent& event)
{
    HIVIEW_LOGD("[JankAnimatorReporterAdapter::ReportCritical]");
    try {
        JankAnimatorReportData data = ConvertReporterEventToData(event);
        if (reporter != nullptr) {
            reporter->ReportCritical(data);
        } else {
            HIVIEW_LOGE("[JankAnimatorReporterAdapter ReportCritical] report is null");
        }
    }
    catch (std::logic_error& ex) {
        HIVIEW_LOGE("[JankAnimatorReporterAdapter::ReportCritical] exception: %s", ex.what());
    }
}

JankAnimatorReportData JankAnimatorReporterAdapter::ConvertReporterEventToData(const JankAnimatorReportEvent& event)
{
    JankAnimatorReportData data;
    data.appPid = event.appPid;
    data.versionCode = event.versionCode;
    data.versionName = event.versionName;
    data.bundleName = event.bundleName;
    data.processName = event.processName;
    data.abilityName = event.abilityName;
    data.pageUrl = event.pageUrl;
    data.sceneId = event.sceneId;
    data.bundleNameEx = event.bundleNameEx;
    data.isFocus = event.isFocus;
    data.startTime = event.startTime;
    data.durition = event.durition;
    data.totalAppFrames = event.totalAppFrames;
    data.totalAppMissedFrames = event.totalAppMissedFrames;
    data.maxAppFrameTime = event.maxAppFrameTime;
    data.maxAppSeqMissedFrames = event.maxAppSeqMissedFrames;
    data.isDisplayAnimator = event.isDisplayAnimator;
    data.totalRenderFrames = event.totalRenderFrames;
    data.totalRenderMissedFrames = event.totalRenderMissedFrames;
    data.maxRenderFrameTime = event.maxRenderFrameTime;
    data.averageRenderFrameTime = event.averageRenderFrameTime;
    data.maxRenderSeqMissedFrames = event.maxRenderSeqMissedFrames;
    data.isFoldDisp = event.isFoldDisp;
    /* only for critical */
    data.traceFileName = event.traceFileName;
    data.infoFileName = event.infoFileName;
    data.happenTime = event.happenTime;
    return data;
}

ScrollJankEventInfo JankAnimatorReporterAdapter::ConvertReportEventToEventInfo(const JankAnimatorReportEvent& event)
{
    ScrollJankEventInfo info;
    info.appPid = event.appPid;
    info.versionCode = event.versionCode;
    info.versionName = event.versionName;
    info.bundleName = event.bundleName;
    info.processName = event.processName;
    info.abilityName = event.abilityName;
    info.pageUrl = event.pageUrl;
    info.sceneId = event.sceneId;
    info.bundleNameEx = event.bundleNameEx;
    info.isFocus = event.isFocus;
    info.startTime = event.startTime;
    info.duration = event.durition;
    info.totalAppFrames = event.totalAppFrames;
    info.totalAppMissedFrames = event.totalAppMissedFrames;
    info.maxAppFrameTime = event.maxAppFrameTime;
    info.maxAppSeqMissedFrames = event.maxAppSeqMissedFrames;
    info.isDisplayAnimator = event.isDisplayAnimator;
    info.totalRenderFrames = event.totalRenderFrames;
    info.totalRenderMissedFrames = event.totalRenderMissedFrames;
    info.maxRenderFrameTime = event.maxRenderFrameTime;
    info.averageRenderFrameTime = event.averageRenderFrameTime;
    info.maxRenderSeqMissedFrames = event.maxRenderSeqMissedFrames;
    info.isFoldDisp = event.isFoldDisp;
    /* only for critical */
    info.traceFileName = event.traceFileName;
    info.infoFileName = event.infoFileName;
    info.happenTime = event.happenTime;
    return info;
}

bool JankAnimatorReporterAdapter::IsScrollJankEvent(const JankAnimatorReportEvent& event)
{
    const std::string sceneId = event.sceneId;
    return ((sceneId == "APP_LIST_FLING") || (sceneId == "APP_SWIPER_SCROLL") || (sceneId == "APP_SWIPER_FLING"));
}