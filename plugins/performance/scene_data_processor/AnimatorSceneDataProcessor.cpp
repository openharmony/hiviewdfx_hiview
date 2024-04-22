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

#include "AnimatorSceneDataProcessor.h"
#include "JlogId.h"
#include "hiview_logger.h"
DEFINE_LOG_TAG("XPerformance");

namespace {
const int32_t S_TO_MS = 1000;
const int64_t S_TO_US = 10000000;
// process animator point when timeout
const int32_t TIME_OUT = 5000;

// define animator process point
enum ANIMATOR_POINT {
    SUP_START_POINT = -1,
    START_POINT = 0,
    END_POINT = 1
};
}
static int64_t GetCurrentRealtimeMs()
{
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_REALTIME, &t);
    return (t.tv_sec * S_TO_MS + t.tv_nsec / S_TO_US);
}

void AnimatorSceneDataProcessor::SetCb(IAnimatorSceneDataProcessor::MetricReporter* cb)
{
    this->cb = cb;
}

void AnimatorSceneDataProcessor::CacheFocusWindowPid(std::shared_ptr<XperfEvt> evt)
{
    if (evt == nullptr) {
        return;
    }
    focusWindowPid = evt->pid;
    focusBundleName = evt->bundleName;
}

void AnimatorSceneDataProcessor::ProcessSceneData(std::shared_ptr<XperfEvt> evt)
{
    HIVIEW_LOGD("[AnimatorSceneDataProcessor::ProcessSceneData]");
    if (evt == nullptr) {
        HIVIEW_LOGD("[AnimatorSceneDataProcessor::ProcessSceneData] evt == nullptr");
        return;
    }
    switch (evt->logId) {
        case JLID_ACE_INTERACTION_APP_JANK:
            HIVIEW_LOGD("[AnimatorSceneDataProcessor::ProcessSceneData] JLID_ACE_INTERACTION_APP_JANK");
            CreateRecord(evt, START_POINT);
            break;
        case JLID_GRAPHIC_INTERACTION_RENDER_JANK:
            HIVIEW_LOGD("[AnimatorSceneDataProcessor::ProcessSceneData] JLID_GRAPHIC_INTERACTION_RENDER_JANK");
            if (!HasStartPoint(evt)) {
                CreateRecord(evt, SUP_START_POINT);
            }
            SaveAnimatorPoint(evt, END_POINT);
            break;
        case JLID_WINDOWMANAGER_FOCUS_WINDOW:
            HIVIEW_LOGD("[AnimatorSceneDataProcessor::ProcessSceneData] JLID_WINDOWMANAGER_FOCUS_WINDOW");
            CacheFocusWindowPid(evt);
            break;
        default:
            return;
    }
    if (AllPointsReceived(evt)) {
        HIVIEW_LOGD("[AnimatorSceneDataProcessor::ProcessSceneData] AllPointsReceived");
        AnimatorRecord* record = GetRecord(evt->animatorInfo.basicInfo.uniqueId);
        if (record == nullptr) {
            HIVIEW_LOGD("[AnimatorSceneDataProcessor::ProcessSceneData] record == nullptr");
            return;
        }
        AnimatorMetrics metrics = CalcMetrics(*record);
        Report(metrics);
        DeleteRecord(evt->animatorInfo.basicInfo.uniqueId);
    }
    HandleTimeOutPoints();
}

void AnimatorSceneDataProcessor::CreateRecord(std::shared_ptr<XperfEvt> evt, int32_t indexPoint)
{
    HIVIEW_LOGD("[AnimatorSceneDataProcessor::CreateRecord]");
    if (evt == nullptr) {
        return;
    }
    AnimatorRecord* animatorRecord = new AnimatorRecord();
    db.insert(std::map<int32_t, AnimatorRecord*>::value_type(evt->animatorInfo.basicInfo.uniqueId, animatorRecord));
    SaveAnimatorPoint(evt, indexPoint);
}

bool AnimatorSceneDataProcessor::HasStartPoint(std::shared_ptr<XperfEvt> evt)
{
    if (evt == nullptr) {
        return false;
    }
    AnimatorRecord* record = GetRecord(evt->animatorInfo.basicInfo.uniqueId);
    if (record != nullptr && record->hasStartPoint) {
        return true;
    }
    return false;
}

bool AnimatorSceneDataProcessor::AllPointsReceived(std::shared_ptr<XperfEvt> evt)
{
    if (evt == nullptr) {
        return false;
    }
    AnimatorRecord* record = GetRecord(evt->animatorInfo.basicInfo.uniqueId);
    if (record != nullptr) {
        if (record->hasStartPoint &&
            record->receviedPoint >= record->allReceviedPoint) {
            return true;
        }
    }
    return false;
}

void AnimatorSceneDataProcessor::SaveAnimatorPoint(std::shared_ptr<XperfEvt> evt, int32_t indexPoint)
{
    HIVIEW_LOGD("[AnimatorSceneDataProcessor::SaveAnimatorPoint]");
    if (evt == nullptr) {
        return;
    }
    AnimatorRecord* record = GetRecord(evt->animatorInfo.basicInfo.uniqueId);
    if (record == nullptr) {
        return;
    }
    if (indexPoint == START_POINT || indexPoint == SUP_START_POINT) {
        record->hasStartPoint = true;
        record->startTime = GetCurrentRealtimeMs();
    }
    record->receviedPoint++;
    if (indexPoint == SUP_START_POINT) {
        record->animatorPoints[START_POINT].basicInfo = evt->animatorInfo.basicInfo;
        return;
    }
    record->animatorPoints[indexPoint].basicInfo = evt->animatorInfo.basicInfo;
    record->animatorPoints[indexPoint].commonInfo = evt->animatorInfo.commonInfo;
}

AnimatorMetrics AnimatorSceneDataProcessor::CalcMetrics(const AnimatorRecord& record)
{
    AnimatorMetrics animatorMetrics;
    animatorMetrics.basicInfo = record.animatorPoints[START_POINT].basicInfo;
    animatorMetrics.appInfo = record.animatorPoints[START_POINT].commonInfo;
    animatorMetrics.rsInfo = record.animatorPoints[END_POINT].commonInfo;
    if (animatorMetrics.basicInfo.bundleNameEx.empty()) {
        animatorMetrics.basicInfo.bundleNameEx = animatorMetrics.basicInfo.moduleName;
    }
    if (animatorMetrics.basicInfo.bundleNameEx == focusBundleName ||
        (animatorMetrics.basicInfo.bundleNameEx.empty() && animatorMetrics.basicInfo.moduleName == focusBundleName)) {
        animatorMetrics.focus = true;
    } else {
        animatorMetrics.focus = false;
    }
    return animatorMetrics;
}

void AnimatorSceneDataProcessor::Report(const AnimatorMetrics& metrics)
{
    HIVIEW_LOGD("[AnimatorSceneDataProcessor::Report]");
    cb->ReportMetrics(metrics);
}

AnimatorRecord* AnimatorSceneDataProcessor::GetRecord(const int32_t uniqueId)
{
    AnimatorRecord* record = nullptr;
    if (db.find(uniqueId) != db.end()) {
        record = db[uniqueId];
    }
    return record;
}

void AnimatorSceneDataProcessor::DeleteRecord(const int32_t uniqueId)
{
    auto iter = db.find(uniqueId);
    if (iter != db.end()) {
        delete iter->second;
        db.erase(iter);
    }
}

void AnimatorSceneDataProcessor::HandleTimeOutPoints()
{
    HIVIEW_LOGD("[AnimatorSceneDataProcessor::HandleTimeOutPoints] begin");
    for (auto it = db.begin(); it != db.end();) {
        if (it->second != nullptr && (GetCurrentRealtimeMs() - (it->second)->startTime) >= TIME_OUT) {
            HIVIEW_LOGD("[AnimatorSceneDataProcessor::HandleTimeOutPoints] match if");
            Report(CalcMetrics(*(it->second)));
            delete it->second;
            it = db.erase(it);
            continue;
        }
        it++;
    }
    HIVIEW_LOGD("[AnimatorSceneDataProcessor::HandleTimeOutPoints] end");
}
