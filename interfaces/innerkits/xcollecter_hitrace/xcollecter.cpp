/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <iostream>
#include "xcollecter.h"
#include "xcollect_service.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
class CollectResultCallback : public CollectCallback {
public:
    explicit CollectResultCallback(std::shared_ptr<CollectTaskResult> result) : result_(result)
    {}

    virtual ~CollectResultCallback()
    {
        result_ = nullptr;
    }

    virtual void Handle(std::shared_ptr<CollectItemResult> data, bool isFinish)
    {
        result_->Add(data);
    }
private:
    std::shared_ptr<CollectTaskResult> result_;
};

std::shared_ptr<CollectTaskResult> Xcollecter::SubmitTask(std::shared_ptr<CollectParameter> collectParameter)
{
    std::shared_ptr<CollectTaskResult> result = std::make_shared<CollectTaskResult>();
    std::shared_ptr<CollectCallback> callback = std::make_shared<CollectResultCallback>(result);
    XcollectService service(collectParameter, callback);
    service.StartCollect();
    return result;
}
} // namespace HiviewDFX
} // namespace OHOS
