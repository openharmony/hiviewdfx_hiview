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
#include "collect_device_client.h"

#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include "logger.h"
#include "securec.h"

#define UNIFIED_COLLECTION_DEVICE  "/dev/ucollection"
namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("UDeviceCli");

CollectDeviceClient::CollectDeviceClient(): fd_(-1)
{}

CollectDeviceClient::~CollectDeviceClient()
{
    if (fd_ > 0) {
        close(fd_);
    }
}

int CollectDeviceClient::GetDeviceFd(bool readOnly)
{
    int f = 0;
    if (readOnly) {
        f = open(UNIFIED_COLLECTION_DEVICE, O_RDONLY);
    } else {
        f = open(UNIFIED_COLLECTION_DEVICE, O_WRONLY);
    }

    if (f < 0) {
        HIVIEW_LOGI("Cannot open file=%{public}s, readOnly=%{public}d", UNIFIED_COLLECTION_DEVICE, readOnly);
        return -1;
    }
    HIVIEW_LOGI("open ucollection device readOnly=%{public}d successful", readOnly);
    return f;
}

int CollectDeviceClient::Open()
{
    fd_ = GetDeviceFd(true);
    return (fd_ > 0) ? 0 : -1;
}

std::shared_ptr<ProcessCpuData> CollectDeviceClient::FetchProcessCpuData()
{
    HIVIEW_LOGI("send IOCTRL_COLLECT_ALL_PROC_CPU, cmd=%{public}u", IOCTRL_COLLECT_ALL_PROC_CPU);
    std::shared_ptr<ProcessCpuData> data = std::make_shared<ProcessCpuData>();
    int ret = ioctl(fd_, IOCTRL_COLLECT_ALL_PROC_CPU, data->entry_);
    if (ret < 0) {
        HIVIEW_LOGI("ioctl IOCTRL_COLLECT_ALL_PROC_CPU cmd=%{public}d, ret=%{public}d",
            IOCTRL_COLLECT_ALL_PROC_CPU, ret);
        return data;
    }
    return data;
}

std::shared_ptr<ProcessCpuData> CollectDeviceClient::FetchProcessCpuData(int pid)
{
    HIVIEW_LOGI("send IOCTRL_COLLECT_THE_PROC_CPU, cmd=%{public}u", IOCTRL_COLLECT_THE_PROC_CPU);
    std::shared_ptr<ProcessCpuData> data = std::make_shared<ProcessCpuData>(pid);
    int ret = ioctl(fd_, IOCTRL_COLLECT_THE_PROC_CPU, data->entry_);
    if (ret < 0) {
        HIVIEW_LOGI("ioctl IOCTRL_COLLECT_THE_PROC_CPU cmd=%{public}d, ret=%{public}d",
            IOCTRL_COLLECT_THE_PROC_CPU, ret);
        return data;
    }
    return data;
}

int CollectDeviceClient::SetDmips(const std::vector<char> &dmipVals)
{
    if (dmipVals.empty()) {
        return 0;
    }
    size_t totalSize = sizeof(struct ucollection_cpu_dmips) + sizeof(char) * dmipVals.size();
    struct ucollection_cpu_dmips *dmips = (struct ucollection_cpu_dmips *)malloc(totalSize);
    if (dmips == nullptr) {
        HIVIEW_LOGW("dmips is nullptr");
        return -1;
    }
    memset_s(dmips, totalSize, 0, totalSize);

    dmips->magic = IOCTRL_SET_CPU_DMIPS_MAGIC;
    dmips->total_count = static_cast<int>(dmipVals.size());
    for (int ii = 0; ii < dmips->total_count; ii++) {
        dmips->dmips[ii] = dmipVals[ii];
    }

    HIVIEW_LOGI("send IOCTRL_SET_CPU_DMIPS, cmd=%{public}u", IOCTRL_SET_CPU_DMIPS);
    int f = GetDeviceFd(false);
    int ret = ioctl(f, IOCTRL_SET_CPU_DMIPS, dmips);
    if (ret < 0) {
        HIVIEW_LOGI("ioctl IOCTRL_SET_CPU_DMIPS cmd=%{public}u, ret=%{public}d", IOCTRL_SET_CPU_DMIPS, ret);
        free(dmips);
        return -1;
    } else {
        close(f);
        HIVIEW_LOGI("set cpu dmips successful");
    }
    free(dmips);
    return 0;
}
} // HiviewDFX
} // OHOS
