/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef HIVIEW_BASE_PLUGIN_FACTORY_H
#define HIVIEW_BASE_PLUGIN_FACTORY_H

#include <memory>
#include <unordered_map>

#include "plugin.h"
#include "plugin_proxy.h"

namespace OHOS {
namespace HiviewDFX {
struct DllExport PluginRegistInfo {
    PluginRegistInfo(std::function<std::shared_ptr<Plugin>()> getPluginObject,
        bool needCreateProxy, bool needStartupLoading)
        : getPluginObject(getPluginObject),
          needCreateProxy(needCreateProxy),
          needStartupLoading(needStartupLoading) {};
    std::function<std::shared_ptr<Plugin>()> getPluginObject;
    bool needCreateProxy;
    bool needStartupLoading;
};

class DllExport PluginFactory {
public:
    static void RegisterPlugin(const std::string& name, std::shared_ptr<PluginRegistInfo> func);
    static void UnregisterPlugin(const std::string& name);
    static std::shared_ptr<Plugin> GetPlugin(const std::string& name);
    static std::shared_ptr<PluginRegistInfo> GetGlobalPluginInfo(const std::string& name);

private:
    static std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<PluginRegistInfo>>>
        GetGlobalPluginRegistryMap();
};

class PluginRegister {
public:
    PluginRegister(const std::string& name, std::shared_ptr<PluginRegistInfo> fp)
    {
        PluginFactory::RegisterPlugin(name, fp);
    };
    ~PluginRegister(){};
};

#define REGISTER__(ClassName, needCreateProxy, needStartupLoading)              \
class Register##ClassName {                                                     \
    public:                                                                     \
        static std::shared_ptr<Plugin> GetObject()                              \
        {                                                                       \
            return std::make_shared<ClassName>();                               \
        }                                                                       \
    private:                                                                    \
        static const PluginRegister g_staticPluginRegister;                     \
};                                                                              \
const PluginRegister Register##ClassName::g_staticPluginRegister(#ClassName,    \
    std::make_shared<PluginRegistInfo>(Register##ClassName::GetObject,          \
    needCreateProxy, needStartupLoading));

#define REGISTER(ClassName) REGISTER__(ClassName, false, true);
#define REGISTER_PROXY(ClassName) REGISTER__(ClassName, true, false);
#define REGISTER_PROXY_WITH_LOADED(ClassName) REGISTER__(ClassName, true, true);
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_BASE_PLUGIN_FACTORY_H
