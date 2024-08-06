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

#include "AppTimerAdapter.h"

namespace OHOS {
namespace HiviewDFX {
AppTimerAdapter::AppTimerAdapter(int userId, ISceneTimerInfrastructure* infra) : userId(userId)
{
    this->impl = infra;
    impl->RegUser(userId, this);
}

AppTimerAdapter::~AppTimerAdapter()
{
    impl->UnRegUser(userId);
}

void AppTimerAdapter::Start(std::string key)
{
    std::lock_guard<std::mutex> uniqueLock(mut);
    int id = AssignId(key);
    ValidateDuplication(id);
    impl->Start(userId, id, timeoutPeriod);
    sessions.push_back(id);
}

void AppTimerAdapter::Stop(std::string key)
{
    std::lock_guard<std::mutex> uniqueLock(mut);
    int id = KeyToId(key);
    ValidateExistence(id);
    impl->Stop(userId, id);
    sessions.erase(std::find(sessions.begin(), sessions.end(), id));
    RecycleId(id);
}

void AppTimerAdapter::Expired(int id)
{
    std::lock_guard<std::mutex> uniqueLock(mut);
    std::string key = IdToKey(id);
    if (!key.empty()) {
        cb->Expired(key);
    }
    sessions.erase(std::find(sessions.begin(), sessions.end(), id));
    RecycleId(id);
}

void AppTimerAdapter::SetCb(IAppTimer::ICb* cb)
{
    this->cb = cb;
}

void AppTimerAdapter::ValidateDuplication(const int id)
{
    if (std::find(sessions.begin(), sessions.end(), id) != sessions.end()) {
        throw std::invalid_argument("duplicated id");
    }
}

void AppTimerAdapter::ValidateExistence(const int id)
{
    if (std::find(sessions.begin(), sessions.end(), id) == sessions.end()) {
        throw std::invalid_argument("non-existing id");
    }
}


int AppTimerAdapter::AssignId(const std::string& key)
{
    if (ids.empty()) {
        throw std::invalid_argument("container ids is empty");
    }
    ValidateExistKey(key);
    int id = ids.back();
    ids.pop_back();
    idToBundle[id] = key;
    bundleToId[key] = id;
    return id;
}

void AppTimerAdapter::RecycleId(const int id)
{
    std::string bundle = idToBundle[id];
    bundleToId.erase(bundle);
    idToBundle.erase(id);
    ids.push_back(id);
}

std::string AppTimerAdapter::IdToKey(const int id)
{
    if (idToBundle.find(id) != idToBundle.end()) {
        return idToBundle[id];
    }
    return "";
}

int AppTimerAdapter::KeyToId(const std::string& key)
{
    if (bundleToId.find(key) != bundleToId.end()) {
        return bundleToId[key];
    }
    return -1;
}

void AppTimerAdapter::ValidateExistKey(const std::string& key)
{
    if (bundleToId.find(key) != bundleToId.end()) {
        throw std::invalid_argument("bundleToId key exist duplicated this record");
    }
}
} // HiviewDFX
} // OHOS