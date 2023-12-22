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
#ifndef FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_FILE_UTILS_H
#define FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_FILE_UTILS_H

#include <map>
#include <string>

#include "hitrace_dump.h"
#include "trace_collector.h"

using OHOS::HiviewDFX::UCollectUtil::TraceCollector;
using OHOS::HiviewDFX::Hitrace::TraceErrorCode;
using OHOS::HiviewDFX::Hitrace::TraceRetInfo;
using OHOS::HiviewDFX::UCollect::UcError;

namespace OHOS {
namespace HiviewDFX {
namespace {
const std::map<TraceErrorCode, UcError> CODE_MAP = {
    {TraceErrorCode::SUCCESS, UcError::SUCCESS},
    {TraceErrorCode::TRACE_NOT_SUPPORTED, UcError::UNSUPPORT},
    {TraceErrorCode::TRACE_IS_OCCUPIED, UcError::TRACE_IS_OCCUPIED},
    {TraceErrorCode::TAG_ERROR, UcError::TRACE_TAG_ERROR},
    {TraceErrorCode::FILE_ERROR, UcError::TRACE_FILE_ERROR},
    {TraceErrorCode::WRITE_TRACE_INFO_ERROR, UcError::TRACE_WRITE_ERROR},
    {TraceErrorCode::CALL_ERROR, UcError::TRACE_CALL_ERROR},
};
}

UcError TransCodeToUcError(TraceErrorCode ret);
void FileRemove(UCollectUtil::TraceCollector::Caller &caller);
void CheckAndCreateDirectory(const std::string &tmpDirPath);
bool CreateMultiDirectory(const std::string &dirPath);
const std::string EnumToString(UCollectUtil::TraceCollector::Caller &caller);
std::vector<std::string> GetUnifiedFiles(Hitrace::TraceRetInfo ret, UCollectUtil::TraceCollector::Caller &caller);
void CopyFile(const std::string &src, const std::string &dst);
void CopyToSpecialPath(const std::string &trace, const std::string &revRightStr, const std::string &traceCaller);
std::vector<std::string> GetUnifiedShareFiles(Hitrace::TraceRetInfo ret, UCollectUtil::TraceCollector::Caller &caller);
std::vector<std::string> GetUnifiedSpecialFiles(Hitrace::TraceRetInfo ret,
    UCollectUtil::TraceCollector::Caller &caller);
} // HiViewDFX
} // OHOS
#endif // FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_FILE_UTILS_H
