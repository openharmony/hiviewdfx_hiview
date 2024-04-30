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
#ifndef SCENE_TIMER_OH_IMPL_H
#define SCENE_TIMER_OH_IMPL_H

#include "ISceneTimerInfrastructure.h"
#include <map>
#include <mutex>
#include <condition_variable>

/* this design is difficult to UT, that's a problem */
class SceneTimerOhImpl : public ISceneTimerInfrastructure {
public:
    const static unsigned int checkInterval = 500; // this means accuracy

    SceneTimerOhImpl();
    void RegUser(int userId, ICb* cb) override;
    void UnRegUser(int userId) override;
    void Start(int user, int id, long interval) override;
    void Stop(int user, int id) override;

private:
    const static int maxUserId = 16;
    const static int maxRecordPerUser = 128;

    std::map<int, long long> records;
    std::mutex mut;
    std::condition_variable cv;
    std::map<int, ICb*> callbacks; // userId as key, potentially multi-thread problem, but for now, lock is not needed

    void Loop();
    void FillRecordAndNotify(int key, long long expireTs);
    void ValidateDuplication(int key);
    void ValidateExistence(int key);
    long long GetCurTimeStamp();
    void CheckRecordsAndTrigger();
    static int BuildRecordKey(int user, int id);
    long long CalcExpireTimeStamp(long delay);
    void TriggerCallbak(int recordKey);
    static int ExtractUserFromRecordKey(int key);
    static int ExtractIdFromRecordKey(int key);
    void RemoveRecordAndNotify(int key);
};
#endif
