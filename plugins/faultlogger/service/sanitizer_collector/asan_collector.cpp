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

#include "asan_collector.h"

#include <fcntl.h>
#include <map>
#include <regex>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include "logger.h"

#include "reporter.h"
#include "zip_helper.h"
#include "faultlog_util.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("Faultlogger");
const char CLANGLIB[] = "libclang_rt";
const std::string SKIP_SPECIAL_PROCESS = "sa_main appspawn";
const std::string SKIP_SPECIAL_LIB = "libclang_rt libc libutils libcutils";

const std::string ASAN_RECORD_REGEX =
                std::string("==([0-9a-zA-Z_.]{1,})==(\\d+)==ERROR: (AddressSanitizer|LeakSanitizer): "
                            "(\\S+) (.|\\r|\\n)*?SUMMARY: AddressSanitizer:(.|\\r|\\n)*");

enum FieldOfAsanRecord {
    DESCRIPTION_FIELD,
    PROCNAME_FIELD,
    PID_FIELD,
    ORISANITIZERTYPE_FIELD,
    ERRTYPE_FIELD,
    MAX_FIELD
};

const char XDIGIT_REGEX[] = "0[xX][0-9a-fA-F]+";

static std::map<std::string, std::string> g_faultTypeInShort = {
    {"heap-use-after-free", "uaf"},  {"heap-buffer-overflow", "of"},
    {"stack-buffer-underflow", "uf"},  {"initialization-order-fiasco", "iof"},
    {"stack-buffer-overflow", "of"},  {"stack-use-after-return", "uar"},
    {"stack-use-after-scope", "uas"},  {"global-buffer-overflow", "of"},
    {"use-after-poison", "uap"},  {"dynamic-stack-buffer-overflow", "of"},
    {"SEGV", "SEGV"},
};

AsanCollector::AsanCollector(std::unordered_map<std::string, std::string> &stkmap) : SanitizerdCollector(stkmap)
{
    curr_.type = ASAN_LOG_RPT;
    curr_.pid = -1;
    curr_.uid = -1;
    curr_.procName = "0";
    curr_.appVersion = "0";
    curr_.happenTime = 0;
}

AsanCollector::~AsanCollector()
{
}

void AsanCollector::ProcessStackTrace(
    const std::string& asanDump,
    bool printDiagnostics,
    unsigned *hash)
{
    // std::string delimiter = "[\\r\\n]+";
    // include blankline
    std::string delimiter = "(?:\\r\\n|\\r|\\n)";
    auto str_lines = OHOS::HiviewDFX::SplitString(asanDump, delimiter);

    // Match lines such as the following and grab out "function_name".
    // The ? may or may not be present.
    //
    // : #0 0x7215208f97  (/vendor/lib64/hwcam/hwcam.hi3660.m.DUKE.so+0x289f97)

    std::string stackEntry =
        "    #(\\d+) " + std::string(XDIGIT_REGEX) +       // Matches "0x7215208f97"
        "([\\s\\?(]+)" +                                     // Matches " ? ("
        "([^\\+ )]+\\+" + std::string(XDIGIT_REGEX) + ")";   // Matches until delimiter reached
    static const std::regex stackEntryRe(stackEntry);
    std::match_results<std::string::iterator> stack_entry_captured;

    std::string hashable;
    std::string previous_hashable;

    *hash = 0;
    // Stacktrace end until "is located" is reached.
    std::string stackEnd = " is located";
    static const std::regex stackEndRe(stackEnd);
    std::match_results<std::string::iterator> stack_end_captured;

    for (auto str_line: str_lines) {
        std::string frm_no;
        std::string xdigit;
        std::string function_name;
        if (std::regex_search(str_line.begin(), str_line.end(), stack_entry_captured, stackEntryRe)) {
            frm_no = stack_entry_captured[1].str();
            xdigit = stack_entry_captured[2].str();
            function_name = stack_entry_captured[3].str();

            if (frm_no == "0") {
                if (printDiagnostics) {
                    HIVIEW_LOGI("Stack trace starting.%{public}s",
                                hashable.empty() ? "" : "  Saving prior trace.");
                }
                previous_hashable = hashable;
                hashable.clear();
                curr_.func.clear();
            }

            if (!hashable.empty())
                hashable.append("|");
            hashable.append(function_name);

            if (curr_.func.empty()) {
                if (!function_name.empty()) {
                    // skip special libclang_rt lib
                    if (function_name.find(CLANGLIB) == std::string::npos) {
                        curr_.func = function_name;
                    }
                }
            }
        } else if (std::regex_search(str_line.begin(), str_line.end(), stack_end_captured, stackEndRe)) {
            if (printDiagnostics) {
                SANITIZERD_LOGI("end of stack matched reline:(%{public}s)\n", str_line.c_str());
            }
            break;
        }
    }

    // If the hashable is empty (meaning all frames are uncertain,
    // for whatever reason) also use the previous frame, as it cannot be any
    // worse.
    if (hashable.empty()) {
        hashable = previous_hashable;
    }

    *hash = OHOS::HiviewDFX::HashString(hashable);
}

// Compute the stacktrace signature
bool AsanCollector::ComputeStackSignature(const std::string& asanDump, std::string& asanSignature,
                                          bool printDiagnostics)
{
    unsigned stackHash = 0;
    std::string human_string;

    ProcessStackTrace(asanDump,
                      printDiagnostics,
                      &stackHash);

    if (stackHash == 0) {
        if (printDiagnostics) {
            HIVIEW_LOGI("Maple Found not a stack, failing.");
        }
        return false;
    }

    // Format to the hex string
    asanSignature = "%08X" + std::to_string(stackHash);
    return true;
}

bool AsanCollector::IsDuplicate(const std::string& hash)
{
    auto back_iter = stacks_.find(hash);
    return (back_iter != stacks_.end());
}

int AsanCollector::UpdateCollectedData(const std::string& hash, const std::string& rfile)
{
    auto jstack = std::pair<std::string, std::string> {hash, rfile};
    std::lock_guard<std::mutex> lockGuard(mutex_);
    stacks_.insert(jstack);

    HIVIEW_LOGI("Updating collected data ...");
    // Do upload when data ready
    OHOS::HiviewDFX::Upload(&curr_);
    return 0;
}

std::string AsanCollector::GetTopStackWithoutCommonLib(const std::string& description)
{
    std::string topstack;
    std::string record = description;
    std::smatch stackCaptured;
    std::string stackRecord =
    "  #[\\d+] " + std::string(XDIGIT_REGEX) +
    "[\\s\\?(]+" +
    "[^\\+ ]+/(\\w+)(.z)?.so\\+" + std::string(XDIGIT_REGEX);
    static const std::regex stackRe(stackRecord);

    while (std::regex_search(record, stackCaptured, stackRe)) {
        if (topstack.size() == 0) {
            topstack = stackCaptured[1].str();
        }
        if (SKIP_SPECIAL_LIB.find(stackCaptured[1].str().c_str()) == std::string::npos) {
            return stackCaptured[1].str();
        }
        record = stackCaptured.suffix().str();
    }

    return topstack;
}

void AsanCollector::CalibrateErrTypeProcName()
{
    char procName[MAX_PROCESS_PATH];
    std::map<std::string, std::string>::iterator fault_type_iter;

    fault_type_iter = g_faultTypeInShort.find(curr_.errType);
    if (fault_type_iter != g_faultTypeInShort.end()) {
        curr_.errTypeInShort = fault_type_iter->second;
    } else {
        curr_.errTypeInShort = curr_.errType;
    }

    if (curr_.uid >= MIN_APP_USERID) {
        curr_.procName = GetApplicationNameById(curr_.uid);
    }

    if  (curr_.uid >= MIN_APP_USERID && !curr_.procName.empty() && IsModuleNameValid(curr_.procName)) {
        curr_.procName = RegulateModuleNameIfNeed(curr_.procName);
        HIVIEW_LOGI("Get procName %{public}s from uid %{public}d.", curr_.procName.c_str(), curr_.uid);
        curr_.appVersion = GetApplicationVersion(curr_.uid, curr_.procName);
        HIVIEW_LOGI("Version is %{public}s.", curr_.appVersion.c_str());
    } else if (OHOS::HiviewDFX::GetNameByPid(static_cast<pid_t>(curr_.pid), procName) == true) {
        curr_.procName = std::string(procName);
    } else if (SKIP_SPECIAL_PROCESS.find(curr_.procName.c_str()) != std::string::npos) {
        // get top stack
        curr_.procName = GetTopStackWithoutCommonLib(curr_.description);
    }
}

void AsanCollector::SetHappenTime()
{
    time_t timeNow = time(nullptr);
    uint64_t timeTmp = timeNow;
    std::string timeStr = GetFormatedTime(timeTmp);
    curr_.happenTime = std::stoll(timeStr);
}

bool AsanCollector::ReadRecordToString(std::string& fullFile, const std::string& fileName)
{
    // A record is a log dump. It has an associated size of "record_size".
    std::string record;
    std::smatch captured;

    record.clear();
    if (!OHOS::HiviewDFX::ReadFileToString(fullFile, record)) {
        HIVIEW_LOGI("Unable to open %{public}s", fullFile.c_str());
        return false;
    }

    int hitcount = 0;
    struct stat st;
    if (stat(fullFile.c_str(), &st) == -1) {
        HIVIEW_LOGI("stat %{public}s error", fullFile.c_str());
    } else {
        curr_.uid = static_cast<int32_t>(st.st_uid);
    }

    static const std::regex recordRe(ASAN_RECORD_REGEX);
    while (std::regex_search(record, captured, recordRe)) {
        std::string signature;

        curr_.description = captured[DESCRIPTION_FIELD].str();
        curr_.procName = captured[PROCNAME_FIELD].str();
        curr_.pid = stoi(captured[PID_FIELD].str());
        if (captured[ORISANITIZERTYPE_FIELD].str().compare(
            std::string(SANITIZERD_TYPE_STR[LSAN_LOG_RPT][ORISANITIZERTYPE])) == 0) {
            curr_.errType = captured[ORISANITIZERTYPE_FIELD].str();
            curr_.type = LSAN_LOG_RPT;
        } else {
            curr_.errType = captured[ERRTYPE_FIELD].str();
        }

        CalibrateErrTypeProcName();

        // if record_found
        if (ComputeStackSignature(captured[DESCRIPTION_FIELD].str(), signature, false)) {
            SetHappenTime();
            curr_.logName = fileName;
            curr_.hash = signature;
        }

        if (hitcount > 0) {
            sleep(1);
        }

        // Sync hash map in memory
        UpdateCollectedData(signature, fullFile);
        hitcount++;
        record = captured.suffix().str();
    }

    return true;
}

void AsanCollector::Collect(const std::string& filepath)
{
    // Compose the full path string
    std::string strAsanLogPath(ASAN_LOG_PATH);
    std::string fullPath = strAsanLogPath + "/" + filepath;
    ReadRecordToString(fullPath, filepath);
}
} // namespace HiviewDFX
} // namespace OHOS
