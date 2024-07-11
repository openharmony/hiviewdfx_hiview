/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#ifndef HIVIEW_FAULTLOG_INFO_H
#define HIVIEW_FAULTLOG_INFO_H
#include <cstdint>
#include <string>
namespace OHOS {
namespace HiviewDFX {
class FaultLogInfo {
public:
    ~FaultLogInfo();

    /**
     * @brief get user id
     *
     * @return user id
    */
    uint32_t GetId() const;

    /**
     * @brief get process id
     *
     * @return process id
    */
    int32_t GetProcessId() const;

    /**
     * @brief get file descriptor
     * the fd is managed by fault log info obj it will be closed in destructor
     *
     * @return file descriptor
    */
    int32_t GetRawFileDescriptor() const;

    /**
     * @brief get type of fault
     *
     * @return type of fault(int32_t)
    */
    int32_t GetFaultType() const;

    /**
     * @brief get timestamp when fault happened
     *
     * @return timestamp
    */
    int64_t GetTimeStamp() const;

    /**
     * @brief get type of fault
     *
     * @return fault type(string)
    */
    std::string GetStringFaultType() const;

    /**
     * @brief get the reason for fault
     *
     * @return the reason for fault
    */
    std::string GetFaultReason() const;

    /**
     * @brief get the name of module which occurred fault
     *
     * @return the name of module
    */
    std::string GetModuleName() const;

    /**
     * @brief get the summary of fault information
     *
     * @return summary of fault information
    */
    std::string GetFaultSummary() const;

    /**
     * @brief set user id
     *
     * @param id user id
    */
    void SetId(uint32_t id);

    /**
     * @brief set process id
     *
     * @param pid process id
    */
    void SetProcessId(int32_t pid);

    /**
     * @brief set timestamp when fault happened
     *
     * @param ts timestamp
    */
    void SetTimeStamp(int64_t ts);

    /**
     * @brief set type of fault
     *
     * @param faultType type of fault
    */
    void SetFaultType(int32_t faultType);

    /**
     * @brief set the reason for fault
     *
     * @param reason the reason for fault
    */
    void SetFaultReason(const std::string &reason);

    /**
     * @brief set the name of module which occurred fault
     *
     * @param module the name of module
    */
    void SetModuleName(const std::string &module);

    /**
     * @brief set the summary of fault information
     *
     * @param summary summary of fault information
    */
    void SetFaultSummary(const std::string &summary);

    /**
     * @brief set file descriptor
     * the fd is managed by fault log info obj it will be closed in destructor
     *
     * @param fd file descriptor
    */
    void SetRawFileDescriptor(int32_t fd);

private:
    int32_t pid_ {-1};
    uint32_t uid_ {-1};
    int32_t type_ {0};
    int32_t fd_ {-1};
    int64_t ts_ {0};
    std::string reason_;
    std::string module_;
    std::string summary_;
};
}  // namespace HiviewDFX
}  // namespace OHOS
#endif  // HIVIEW_FAULTLOG_INFO_H