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

#include "process_collector_impl.h"

#include <dlfcn.h>

#include "hiview_logger.h"
#include "process_decorator.h"

using namespace OHOS::HiviewDFX::UCollect;

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
namespace {
DEFINE_LOG_TAG("UCollectUtil-ProcessCollector");
const std::string LIB_NAME = "libucollection_utility_ex.z.so";
const std::string GET_MEM_CG_PROCESS_FUNC_NAME = "GetMemCgProcess";
const std::string IS_MEM_CG_PROCESS_FUNC_NAME = "IsMemCgProcess";
}

ProcessCollectorImpl::ProcessCollectorImpl()
{
    handle_ = dlopen(LIB_NAME.c_str(), RTLD_LAZY);
    if (handle_ == nullptr) {
        HIVIEW_LOGW("dlopen failed, error: %{public}s", dlerror());
    }
}

ProcessCollectorImpl::~ProcessCollectorImpl()
{
    if (handle_ != nullptr) {
        dlclose(handle_);
    }
}

std::shared_ptr<ProcessCollector> ProcessCollector::Create()
{
    static std::shared_ptr<ProcessCollector> instance_ =
        std::make_shared<ProcessDecorator>(std::make_shared<ProcessCollectorImpl>());
    return instance_;
}

CollectResult<std::unordered_set<int32_t>> ProcessCollectorImpl::GetMemCgProcess()
{
    CollectResult<std::unordered_set<int32_t>> result;
    if (handle_ == nullptr) {
        return result;
    }

    using GetMemCgProcess = bool (*)(std::unordered_set<int32_t>&);
    GetMemCgProcess getMemCgProcess =
        reinterpret_cast<GetMemCgProcess>(dlsym(handle_, GET_MEM_CG_PROCESS_FUNC_NAME.c_str()));
    if (!getMemCgProcess) {
        HIVIEW_LOGE("dlsym failed, %{public}s.", dlerror());
        return result;
    }

    std::unordered_set<int32_t> memCgProcs;
    if (!getMemCgProcess(memCgProcs)) {
        result.retCode = UcError::READ_FAILED;
        return result;
    }
    result.data = memCgProcs;
    result.retCode = UcError::SUCCESS;
    return result;
}

CollectResult<bool> ProcessCollectorImpl::IsMemCgProcess(int32_t pid)
{
    CollectResult<bool> result;
    if (handle_ == nullptr) {
        return result;
    }

    using IsMemCgProcess = bool (*)(int32_t);
    IsMemCgProcess isMemCgProcess =
        reinterpret_cast<IsMemCgProcess>(dlsym(handle_, IS_MEM_CG_PROCESS_FUNC_NAME.c_str()));
    if (!isMemCgProcess) {
        HIVIEW_LOGE("dlsym failed, %{public}s.", dlerror());
        return result;
    }

    result.data = isMemCgProcess(pid);
    result.retCode = UcError::SUCCESS;
    return result;
}
} // UCollectUtil
} // HiViewDFX
} // OHOS
