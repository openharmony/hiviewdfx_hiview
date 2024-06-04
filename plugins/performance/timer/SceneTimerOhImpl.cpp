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

#include "SceneTimerOhImpl.h"
#include <sys/prctl.h>
#include <chrono>
#include <thread>
#include "hiview_logger.h"

DEFINE_LOG_LABEL(0xD002D66, "Hiview-XPerformance");

using namespace std::chrono;

const static std::string TIMER_THREAD_NAME = "XperfTimerThr";

SceneTimerOhImpl::SceneTimerOhImpl()
{
    std::thread timerThread(&SceneTimerOhImpl::Loop, this);
    timerThread.detach();
}

void SceneTimerOhImpl::RegUser(int userId, ICb* cb)
{
    callbacks[userId] = cb;
}

void SceneTimerOhImpl::UnRegUser(int userId)
{
    callbacks.erase(userId);
}

void SceneTimerOhImpl::Start(int user, int id, long interval)
{
    int key = BuildRecordKey(user, id);
    ValidateDuplication(key);
    long long expireTs = CalcExpireTimeStamp(interval);
    FillRecordAndNotify(key, expireTs);
}

void SceneTimerOhImpl::Stop(int user, int id)
{
    int key = BuildRecordKey(user, id);
    ValidateExistence(key);
    RemoveRecordAndNotify(key);
}

void SceneTimerOhImpl::Loop()
{
    std::unique_lock<std::mutex> uniqueLock(mut);
    prctl(PR_SET_NAME, TIMER_THREAD_NAME.c_str(), nullptr, nullptr, nullptr);
    milliseconds interval(checkInterval);
    while (true) {
        if (!records.empty()) {
            cv.wait_for(uniqueLock, interval);
            /* note the lock is held now */
            CheckRecordsAndTrigger();
            continue;
        }
        cv.wait(uniqueLock);
    }
}

void SceneTimerOhImpl::FillRecordAndNotify(int key, long long expireTs)
{
    std::unique_lock<std::mutex> uniqueLock(mut);
    // I believe this double-check is needed
    if (records.find(key) != records.end()) {
        throw std::invalid_argument("duplicated id");
    }
    records.emplace(key, expireTs);
    cv.notify_all(); // remember this
}

void SceneTimerOhImpl::ValidateDuplication(int key)
{
    std::unique_lock<std::mutex> uniqueLock(mut);
    if (records.find(key) != records.end()) {
        throw std::invalid_argument("duplicated id");
    }
}

void SceneTimerOhImpl::ValidateExistence(int key)
{
    std::unique_lock<std::mutex> uniqueLock(mut);
    if (records.find(key) == records.end()) {
        throw std::invalid_argument("non-existing id");
    }
}

long long SceneTimerOhImpl::GetCurTimeStamp()
{
    milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    return ms.count();
}

/* already have lock */
void SceneTimerOhImpl::CheckRecordsAndTrigger()
{
    long long nowMs = GetCurTimeStamp();
    for (std::map<int, long long>::iterator it = records.begin(); it != records.end();) {
        long long expire = it->second;
        int key = it->first;
        if (nowMs >= expire) {
            it = records.erase(it);
            TriggerCallbak(key);
        } else {
            it++;
        }
    }
}

int SceneTimerOhImpl::BuildRecordKey(int user, int id)
{
    if (user >= maxUserId) {
        throw std::invalid_argument("exceeds max user id");
    }
    if (id >= maxRecordPerUser) {
        throw std::invalid_argument("exceeds max id per user");
    }
    int ret = user * maxRecordPerUser + id;
    return ret;
}

long long SceneTimerOhImpl::CalcExpireTimeStamp(long delay)
{
    long long nowMs = GetCurTimeStamp();
    long long expire = nowMs + delay;
    return expire;
}

void SceneTimerOhImpl::TriggerCallbak(int recordKey)
{
    int user = ExtractUserFromRecordKey(recordKey);
    int id = ExtractIdFromRecordKey(recordKey);
    try {
        ICb* cb = callbacks.at(user);
        if (cb == nullptr) {
            HIVIEW_LOGE("SceneTimerImpl::TriggerCallbak cb is null");
            return;
        }
        cb->Expired(id);
    }
    catch (const std::out_of_range& outex) {
        HIVIEW_LOGE("SceneTimerImpl::back id=%{public}d;user=%{public}d;key=%{public}d;", id, user, recordKey);
    }
}

int SceneTimerOhImpl::ExtractUserFromRecordKey(int key)
{
    return key / maxRecordPerUser;
}

int SceneTimerOhImpl::ExtractIdFromRecordKey(int key)
{
    return key % maxRecordPerUser;
}

void SceneTimerOhImpl::RemoveRecordAndNotify(int key)
{
    std::unique_lock<std::mutex> uniqueLock(mut);
    records.erase(records.find(key));
    cv.notify_all();
}
