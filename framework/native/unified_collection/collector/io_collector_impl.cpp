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
#include "io_collector.h"

#include <string_ex.h>

#include "common_util.h"
#include "common_utils.h"
#include "file_util.h"
#include "logger.h"

using namespace OHOS::HiviewDFX::UCollect;

DEFINE_LOG_TAG("UCollectUtil");

namespace OHOS {
namespace HiviewDFX {
namespace UCollectUtil {
class IoCollectorImpl : public IoCollector {
public:
    IoCollectorImpl() = default;
    virtual ~IoCollectorImpl() = default;

public:
    virtual CollectResult<ProcessIo> CollectProcessIo(int32_t pid) override;
};

std::shared_ptr<IoCollector> IoCollector::Create()
{
    return std::make_shared<IoCollectorImpl>();
}

CollectResult<ProcessIo> IoCollectorImpl::CollectProcessIo(int32_t pid)
{
    CollectResult<ProcessIo> result;
    std::string filename = PROC + std::to_string(pid) + IO;
    std::string content;
    FileUtil::LoadStringFromFile(filename, content);
    std::vector<std::string> vec;
    OHOS::SplitStr(content, "\n", vec);
    ProcessIo& processIO = result.data;
    processIO.pid = pid;
    processIO.name = CommonUtils::GetProcNameByPid(pid);
    std::string type;
    int32_t value = 0;
    for (const std::string &str : vec) {
        if (CommonUtil::ParseTypeAndValue(str, type, value)) {
            if (type == "rchar") {
                processIO.rchar = value;
                HIVIEW_LOGD("rchar=%{public}d", processIO.rchar);
            } else if (type == "wchar") {
                processIO.wchar = value;
                HIVIEW_LOGD("wchar=%{public}d", processIO.wchar);
            } else if (type == "syscr") {
                processIO.syscr = value;
                HIVIEW_LOGD("syscr=%{public}d", processIO.syscr);
            } else if (type == "syscw") {
                processIO.syscw = value;
                HIVIEW_LOGD("syscw=%{public}d", processIO.syscw);
            } else if (type == "read_bytes") {
                processIO.readBytes = value;
                HIVIEW_LOGD("readBytes=%{public}d", processIO.readBytes);
            } else if (type == "cancelled_write_bytes") {
                processIO.cancelledWriteBytes = value;
                HIVIEW_LOGD("cancelledWriteBytes=%{public}d", processIO.cancelledWriteBytes);
            }
        }
    }
    result.retCode = UcError::SUCCESS;
    return result;
}
} // UCollectUtil
} // HiViewDFX
} // OHOS