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
#ifndef APP_TIMER_ADAPTER_H
#define APP_TIMER_ADAPTER_H

#include <vector>
#include <mutex>
#include <string>
#include <map>
#include "IAppTimer.h"
#include "ISceneTimerInfrastructure.h"

class AppTimerAdapter : public IAppTimer, public ISceneTimerInfrastructure::ICb {
public:
    const static unsigned int timeoutPeriod = 5 * 1000; // 5s

    AppTimerAdapter(int userId, ISceneTimerInfrastructure* infra);
    ~AppTimerAdapter() override;
    void Start(std::string key) override;
    void Stop(std::string key) override;
    void Expired(int id) override;
    void SetCb(IAppTimer::ICb* cb);
private:
    const int userId;

    IAppTimer::ICb* cb{nullptr};
    ISceneTimerInfrastructure* impl;
    std::vector<int> sessions;
    std::mutex mut;

    std::vector<int> ids = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                            11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                            21, 22, 23, 24, 25, 26, 27, 28, 29, 30};
    std::map<int, std::string> idToBundle;
    std::map<std::string, int> bundleToId;

    void ValidateDuplication(const int id);
    void ValidateExistence(const int id);

    std::string IdToKey(const int id);
    int KeyToId(const std::string& key);
    int AssignId(const std::string& key);
    void RecycleId(const int id);
    void ValidateExistKey(const std::string& key);
};
#endif
