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

#include "event_server.h"

#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <securec.h>

#include "base/raw_data_base_def.h"
#include "decoded/decoded_event.h"
#include "device_node.h"
#include "init_socket.h"
#include "logger.h"
#include "socket_util.h"

#define SOCKET_FILE_DIR "/dev/unix/socket/hisysevent"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-EventServer");
namespace {
constexpr int BUFFER_SIZE = 384 * 1024;
#ifndef KERNEL_DEVICE_BUFFER
constexpr int EVENT_READ_BUFFER = 2048;
#else
constexpr int EVENT_READ_BUFFER = KERNEL_DEVICE_BUFFER;
#endif
constexpr char SOCKET_CONFIG_FILE[] = "/system/etc/hiview/hisysevent_extra_socket";
std::string g_extraSocketPath;

struct Header {
    unsigned short len;
    unsigned short headerSize;
    char msg[0];
};

struct Initializer {
    Initializer()
    {
        if (access(SOCKET_CONFIG_FILE, F_OK) != 0) {
            HIVIEW_LOGE("socket config file does not exist");
            return;
        }
        std::ifstream file;
        file.open(SOCKET_CONFIG_FILE);
        if (file.fail()) {
            HIVIEW_LOGE("open socket config file failed");
            return;
        }
        g_extraSocketPath.clear();
        file >> g_extraSocketPath;
        HIVIEW_LOGI("read extra socket path: %{public}s", g_extraSocketPath.c_str());
    }
};
Initializer g_initializer;

void InitSocketBuf(int socketId, int optName)
{
    int bufferSizeOld = 0;
    socklen_t sizeOfInt = static_cast<socklen_t>(sizeof(int));
    if (getsockopt(socketId, SOL_SOCKET, optName, static_cast<void *>(&bufferSizeOld), &sizeOfInt) < 0) {
        HIVIEW_LOGE("get socket buffer error=%{public}d, msg=%{public}s", errno, strerror(errno));
    }

    int bufferSizeSet = BUFFER_SIZE;
    if (setsockopt(socketId, SOL_SOCKET, optName, static_cast<void *>(&bufferSizeSet), sizeof(int)) < 0) {
        HIVIEW_LOGE("set socket buffer error=%{public}d, msg=%{public}s", errno, strerror(errno));
    }

    int bufferSizeNew = 0;
    sizeOfInt = static_cast<socklen_t>(sizeof(int));
    if (getsockopt(socketId, SOL_SOCKET, optName, static_cast<void *>(&bufferSizeNew), &sizeOfInt) < 0) {
        HIVIEW_LOGE("get new socket buffer error=%{public}d, msg=%{public}s", errno, strerror(errno));
    }
    HIVIEW_LOGI("reset buffer size old=%{public}d, new=%{public}d", bufferSizeOld, bufferSizeNew);
}

void InitSendBuffer(int socketId)
{
    InitSocketBuf(socketId, SO_SNDBUF);
}

void InitRecvBuffer(int socketId)
{
    InitSocketBuf(socketId, SO_RCVBUF);
}

void TransferEvent(std::string& text)
{
    HIVIEW_LOGD("event need to transfer: %{public}s", text.c_str());
    if (text.back() == '}') {
        text.pop_back();
        text += ", \"transfer_\":1}";
    }
    struct sockaddr_un serverAddr;
    serverAddr.sun_family = AF_UNIX;
    if (strcpy_s(serverAddr.sun_path, sizeof(serverAddr.sun_path), g_extraSocketPath.c_str()) != EOK) {
        HIVIEW_LOGE("can not assign server path");
        return;
    }
    serverAddr.sun_path[sizeof(serverAddr.sun_path) - 1] = '\0';

    int socketId = TEMP_FAILURE_RETRY(socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));
    if (socketId < 0) {
        HIVIEW_LOGE("create hisysevent socket failed, error=%{public}d, msg=%{public}s", errno, strerror(errno));
        return;
    }
    InitSendBuffer(socketId);
    if (TEMP_FAILURE_RETRY(sendto(socketId, text.c_str(), text.size(), 0, reinterpret_cast<sockaddr*>(&serverAddr),
        sizeof(serverAddr))) < 0) {
        close(socketId);
        socketId = -1;
        HIVIEW_LOGE("send data failed, error=%{public}d, msg=%{public}s", errno, strerror(errno));
        return;
    }
    close(socketId);
    HIVIEW_LOGD("send data successful");
}
}

void SocketDevice::InitSocket(int &socketId)
{
    struct sockaddr_un serverAddr;
    serverAddr.sun_family = AF_UNIX;
    if (strcpy_s(serverAddr.sun_path, sizeof(serverAddr.sun_path), SOCKET_FILE_DIR) != EOK) {
        socketId = -1;
        HIVIEW_LOGE("copy hisysevent dev path failed, error=%{public}d, msg=%{public}s", errno, strerror(errno));
        return;
    }
    serverAddr.sun_path[sizeof(serverAddr.sun_path) - 1] = '\0';
    socketId = TEMP_FAILURE_RETRY(socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));
    if (socketId < 0) {
        HIVIEW_LOGE("create hisysevent socket failed, error=%{public}d, msg=%{public}s", errno, strerror(errno));
        return;
    }
    InitRecvBuffer(socketId_);
    unlink(serverAddr.sun_path);
    if (TEMP_FAILURE_RETRY(bind(socketId, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr))) < 0) {
        close(socketId);
        socketId = -1;
        HIVIEW_LOGE("bind hisysevent socket failed, error=%{public}d, msg=%{public}s", errno, strerror(errno));
        return;
    }
}

int SocketDevice::Open()
{
    socketId_ = GetControlSocket("hisysevent");
    if (socketId_ < 0) {
        HIVIEW_LOGI("create hisysevent socket");
        InitSocket(socketId_);
    } else {
        InitRecvBuffer(socketId_);
        HIVIEW_LOGI("use hisysevent exist socket");
    }

    if (socketId_ < 0) {
        HIVIEW_LOGE("hisysevent create socket failed");
        return -1;
    }
    return socketId_;
}

int SocketDevice::Close()
{
    if (socketId_ > 0) {
        close(socketId_);
        socketId_ = -1;
    }
    return 0;
}

uint32_t SocketDevice::GetEvents()
{
    return EPOLLIN | EPOLLET;
}

std::string SocketDevice::GetName()
{
    return "SysEventSocket";
}

int SocketDevice::ReceiveMsg(std::vector<std::shared_ptr<EventReceiver>> &receivers)
{
    char* buffer = new char[BUFFER_SIZE + 1]();
    while (true) {
        struct sockaddr_un clientAddr;
        socklen_t clientLen = static_cast<socklen_t>(sizeof(clientAddr));
        int n = recvfrom(socketId_, buffer, sizeof(char) * BUFFER_SIZE, 0,
            reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
        if (n < static_cast<int>(EventRaw::GetValidDataMinimumByteCount())) {
            break;
        }
        buffer[n] = 0;
        int32_t dataByteCnt = *(reinterpret_cast<int32_t*>(buffer));
        if (dataByteCnt != n) {
            HIVIEW_LOGE("length of data received from client is invalid.");
            break;
        }
        HIVIEW_LOGD("length of data received from client is %{public}d.", dataByteCnt);
        EventRaw::DecodedEvent event(reinterpret_cast<uint8_t*>(buffer));
        std::string eventJsonStr = event.AsJsonStr();
        HIVIEW_LOGD("receive from client %{private}s", eventJsonStr.c_str());
        if (!g_extraSocketPath.empty()) {
            TransferEvent(eventJsonStr);
        }
        for (auto receiver = receivers.begin(); receiver != receivers.end(); receiver++) {
            (*receiver)->HandlerEvent(event.GetRawData());
        }
    }
    delete[] buffer;
    return 0;
}

int BBoxDevice::Close()
{
    if (fd_ > 0) {
        close(fd_);
        fd_ = -1;
    }
    return 0;
}

int BBoxDevice::Open()
{
    fd_ = open("/dev/sysevent", O_RDONLY, O_NONBLOCK, 0);
    if (fd_ < 0) {
        fd_ = open("/dev/bbox", O_RDONLY, O_NONBLOCK, 0);
    } else {
        hasBbox_ = true;
    }

    if (fd_ < 0) {
        HIVIEW_LOGE("open bbox failed, error=%{public}d, msg=%{public}s", errno, strerror(errno));
        return -1;
    }
    return fd_;
}

uint32_t BBoxDevice::GetEvents()
{
    return EPOLLIN;
}

std::string BBoxDevice::GetName()
{
    return "BBox";
}

int BBoxDevice::ReceiveMsg(std::vector<std::shared_ptr<EventReceiver>> &receivers)
{
    char buffer[EVENT_READ_BUFFER];
    (void)memset_s(buffer, sizeof(buffer), 0, sizeof(buffer));
    int ret = read(fd_, buffer, EVENT_READ_BUFFER);
    if (ret < static_cast<int>(EventRaw::GetValidDataMinimumByteCount())) {
        return -1;
    }
    buffer[EVENT_READ_BUFFER - 1] = '\0';
    int32_t dataByteCnt = *(reinterpret_cast<int32_t*>(buffer));
    if ((hasBbox_ && dataByteCnt != ret) ||
        (!hasBbox_ && dataByteCnt != (ret - sizeof(struct Header) - 1))) { // extra bytes in kernel write
        HIVIEW_LOGE("length of data received from kernel is invalid.");
        return -1;
    }
    HIVIEW_LOGD("length of data received from kernel is %{public}d.", dataByteCnt);
    EventRaw::DecodedEvent event(reinterpret_cast<uint8_t*>(buffer));
    std::string eventJsonStr = event.AsJsonStr();
    HIVIEW_LOGD("receive data from kernel %{private}s", eventJsonStr.c_str());
    if (!g_extraSocketPath.empty()) {
        TransferEvent(eventJsonStr);
    }
    for (auto receiver = receivers.begin(); receiver != receivers.end(); receiver++) {
        (*receiver)->HandlerEvent(event.GetRawData());
    }
    return 0;
}

void EventServer::AddDev(std::shared_ptr<DeviceNode> dev)
{
    int fd = dev->Open();
    if (fd < 0) {
        HIVIEW_LOGI("open device %{public}s failed", dev->GetName().c_str());
        return;
    }
    devs_[fd] = dev;
}

int EventServer::OpenDevs()
{
    AddDev(std::make_shared<SocketDevice>());
    AddDev(std::make_shared<BBoxDevice>());
    if (devs_.empty()) {
        HIVIEW_LOGE("can not open any device");
        return -1;
    }
    HIVIEW_LOGI("has open %{public}zu devices", devs_.size());
    return 0;
}

int EventServer::AddToMonitor(int pollFd, struct epoll_event pollEvents[])
{
    int index = 0;
    auto it = devs_.begin();
    while (it != devs_.end()) {
        HIVIEW_LOGI("add to poll device %{public}s, fd=%{public}d", it->second->GetName().c_str(), it->first);
        pollEvents[index].data.fd = it->first;
        pollEvents[index].events = it->second->GetEvents();
        int ret = epoll_ctl(pollFd, EPOLL_CTL_ADD, it->first, &pollEvents[index]);
        if (ret < 0) {
            HIVIEW_LOGE("add to poll fail device %{public}s error=%{public}d, msg=%{public}s",
                it->second->GetName().c_str(), errno, strerror(errno));
            it->second->Close();
            it = devs_.erase(it);
        } else {
            it++;
        }
        index++;
    }

    if (devs_.empty()) {
        HIVIEW_LOGE("can not monitor any device");
        return -1;
    }
    HIVIEW_LOGI("monitor devices %{public}zu", devs_.size());
    return 0;
}

void EventServer::Start()
{
    HIVIEW_LOGD("start event server");
    if (OpenDevs() < 0) {
        return;
    }

    int pollFd = epoll_create1(EPOLL_CLOEXEC);
    if (pollFd < 0) {
        HIVIEW_LOGE("create poll failed, error=%{public}d, msg=%{public}s", errno, strerror(errno));
        return;
    }

    struct epoll_event pollEvents[devs_.size()];
    if (AddToMonitor(pollFd, pollEvents) < 0) {
        return;
    }

    HIVIEW_LOGI("go into event loop");
    isStart_ = true;
    while (isStart_) {
        struct epoll_event chkPollEvents[devs_.size()];
        int eventCount = epoll_wait(pollFd, chkPollEvents, devs_.size(), -1); // -1: Wait indefinitely
        if (eventCount <= 0) {
            HIVIEW_LOGD("read event timeout");
            continue;
        }
        for (int ii = 0; ii < eventCount; ii++) {
            auto it = devs_.find(chkPollEvents[ii].data.fd);
            it->second->ReceiveMsg(receivers_);
        }
    }
    CloseDevs();
}

void EventServer::CloseDevs()
{
    for (auto devItem : devs_) {
        devItem.second->Close();
    }
}

void EventServer::Stop()
{
    isStart_ = false;
}

void EventServer::AddReceiver(std::shared_ptr<EventReceiver> receiver)
{
    receivers_.emplace_back(receiver);
}
} // namespace HiviewDFX
} // namespace OHOS
