/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "tbox.h"

#include <unistd.h>

#include <regex>
#include "calc_fingerprint.h"
#include "file_util.h"
#include "log_parse.h"
#include "string_util.h"
#include "time_util.h"

using namespace std;
namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("Tbox");

const string Tbox::ARRAY_STR = "ARRAY :";
const string Tbox::CAUSEDBY_HEADER = "Caused by:";
const string Tbox::SUPPRESSED_HEADER = "Suppressed:";

Tbox::Tbox()
{
}

Tbox::~Tbox()
{
}

string Tbox::CalcFingerPrint(const string& val, size_t mask, int mode)
{
    char hash[HAS_LEN] = {'0'};
    int err = -1;
    switch (mode) {
        case FP_FILE:
            err = CalcFingerprint::CalcFileSha(val, hash, sizeof(hash));
            break;
        case FP_BUFFER:
            err = CalcFingerprint::CalcBufferSha(val, val.size(), hash, sizeof(hash));
            break;
        default:
            break;
    }

    if (err != 0) {
        // if file not exist, API will return ENOENT
        HIVIEW_LOGE("ProcessedEvent: calc fingerprint failed, err is %{public}d.", err);
    }
    return string(hash);
}

bool Tbox::GetPartial(const string& src, const string& res, string& des)
{
    des = "";
    regex reNew(res);
    smatch result;
    if (regex_search(src, result, reNew)) {
        for (size_t i = 1; i < result.size(); ++i) {
            if (!result.str(i).empty()) {
                des = string(result.str(i));
                break;
            }
        }
        return true;
    }
    return false;
}

bool Tbox::IsCallStack(string& line)
{
    if (regex_search(line, regex("^\\s+at (.*)\\(.*")) ||
        regex_search(line, regex("^\\s*at .*")) ||
        regex_search(line, regex("^\\s+- (.*)\\(.*")) ||
        regex_search(line, regex("\\s+#\\d+")) ||
        regex_search(line, regex("[0-9a-zA-Z_]+\\+0x[0-9a-f]+/0x[0-9a-f]+")) ||
        regex_search(line, regex("#\\d+")) ||
        regex_search(line, regex("\\.*"))) {
        return true;
    }
    return false;
}

bool Tbox::HasCausedBy(const string& line)
{
    if ((line.find(CAUSEDBY_HEADER) != string::npos) ||
        (line.find(SUPPRESSED_HEADER) != string::npos)) {
        return true;
    }
    return false;
}

/*
 * format1:  com.ohos.launcher:extension
 * format2:  #06 pc 00000000000bb328  /system/lib/libart.so (__epoll_pwait+8)
 */
string Tbox::GetStackName(string line)
{
    string stackname = UNKNOWN_STR;
    if (IsCallStack(line)) {
        stackname = line;
        string str;
        if (GetPartial(line, "^\\s+at (.*)\\).*", str) ||
            GetPartial(line, "^\\s*at (.*)", str) || // for jsCrash
            GetPartial(line, "#\\d+ pc [0-9a-f]+ (.*\\+\\d+)\\)", str) ||
            GetPartial(line, "#\\d+ pc [0-9a-f]+ (.*)", str) ||
            GetPartial(line, "([0-9a-zA-Z_]+\\+0x[0-9a-f]+/0x[0-9a-f]+)", str)) {
            stackname = str;
        } else if (GetPartial(line, "^\\s+- (.*)\\(.*", str)) {
            size_t ret = str.find_last_of("+");
            if (ret != string::npos) {
                str.replace(ret, string::npos, ")\0");
                stackname = str;
            } else {
                stackname = UNKNOWN_STR;
            }
        }
        regex re("(.+?)-(.+)==(.+)");
        stackname = regex_replace(stackname, re, "$1$3");
    }
    return stackname;
}

void Tbox::FilterTrace(std::map<std::string, std::string>& eventInfo)
{
    auto iterEndStack = eventInfo.find(PARAMETER_ENDSTACK);
    if (eventInfo.empty() || iterEndStack == eventInfo.end() || iterEndStack->second.empty()) {
        return;
    }
    std::vector<std::string> trace;
    LogParse logparse;
    std::string block = logparse.GetFilterTrace(iterEndStack->second, trace);
    eventInfo[PARAMETER_ENDSTACK] = block;
    eventInfo["FINGERPRINT"] = Tbox::CalcFingerPrint(block, 0, FP_BUFFER);
    std::stack<std::string> stackTop = logparse.GetStackTop(trace, 3);  // 3 : first/second/last frame
    logparse.SetFrame(stackTop, eventInfo);
}

bool Tbox::WaitForDoneFile(const std::string& file, unsigned int timeout)
{
    uint64_t remainedTime = timeout * NS_PER_SECOND;
    while (remainedTime > 0) {
        if (FileUtil::FileExists(file)) {
            HIVIEW_LOGD("Done file exist: %{public}s", file.c_str());
            return true;
        }
        uint64_t startTime = TimeUtil::GetNanoTime();
        sleep(1);
        uint64_t duration = TimeUtil::GetNanoTime() - startTime;
        remainedTime = (remainedTime > duration) ? (remainedTime - duration) : 0;
    }
    return false;
}
}
}
