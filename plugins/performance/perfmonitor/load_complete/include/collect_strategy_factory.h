/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef COLLECT_STRATEGY_FACTORY_H
#define COLLECT_STRATEGY_FACTORY_H

#include "collect_strategy.h"
#include <string>
#include <memory>

namespace OHOS {
namespace HiviewDFX {

/**
 * @brief 组件收集策略工厂
 * 根据应用名称和场景类型创建相应的收集策略
 */
class CollectStrategyFactory {
public:
    /**
     * @brief 创建组件收集策略
     * @param appName 应用包名
     * @param isLaunch 是否为启动场景
     * @return 组件收集策略指针
     */
    static std::unique_ptr<ComponentCollectStrategy> CreateStrategy(
        const std::string& appName, bool isLaunch);
};

} // namespace HiviewDFX
} // namespace OHOS

#endif // COLLECT_STRATEGY_FACTORY_H