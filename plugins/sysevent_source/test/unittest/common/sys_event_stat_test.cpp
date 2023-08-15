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
#include "sys_event_stat_test.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>

#include "sys_event_stat.h"

namespace OHOS {
namespace HiviewDFX {
void SysEventStatTest::SetUpTestCase() {}

void SysEventStatTest::TearDownTestCase() {}

void SysEventStatTest::SetUp() {}

void SysEventStatTest::TearDown() {}

static int OpenTestFile(const char *filename)
{
    int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd <= 0) {
        printf("[ NET ]:open file error\n");
        return -1;
    }
    return fd;
}

static std::string GetFileContent(const std::string& filename)
{
    std::ifstream inputfile(filename);
    std::string contents;
    std::string temp;
    while (inputfile >> temp) {
        contents.append(temp);
    }
    return contents;
}

/**
 * @tc.name: SysEventStatTest001
 * @tc.desc: check the sys event statistics function.
 * @tc.type: FUNC
 * @tc.require: issueI62WJT
 */
HWTEST_F(SysEventStatTest, SysEventStatTest001, testing::ext::TestSize.Level0)
{
    SysEventStat sysEventStat_;
    sysEventStat_.AccumulateEvent(true);
    sysEventStat_.AccumulateEvent(false);
    sysEventStat_.AccumulateEvent("domain1", "eventName_1");
    sysEventStat_.AccumulateEvent("domain1", "eventName_2");
    sysEventStat_.AccumulateEvent("domain2", "eventName_3");
    sysEventStat_.AccumulateEvent("domain4", "eventName_4", false);
    sysEventStat_.AccumulateEvent("domain4", "eventName_5", false);
    sysEventStat_.AccumulateEvent("domain5", "eventName_6", false);
    int fd1 = OpenTestFile("./fd1.txt");
    ASSERT_FALSE(fd1 == -1);
    sysEventStat_.StatSummary(fd1);
    close(fd1);
    std::string result1 = GetFileContent("./fd1.txt");
    ASSERT_TRUE(result1.size() > 0);

    int fd2 = OpenTestFile("./fd2.txt");
    sysEventStat_.StatDetail(fd2);
    close(fd2);
    std::string result2 = GetFileContent("./fd2.txt");
    ASSERT_TRUE(result2.size() > 0);

    int fd3 = OpenTestFile("./fd3.txt");
    sysEventStat_.StatInvalidDetail(fd3);
    close(fd3);
    std::string result3 = GetFileContent("./fd3.txt");
    ASSERT_TRUE(result3.size() > 0);

    int fd4 = OpenTestFile("./fd4.txt");
    sysEventStat_.Clear(fd4);
    close(fd4);
    std::string result4 = GetFileContent("./fd4.txt");
    std::cout << "result4:" << result4 << std::endl;
    ASSERT_TRUE(result4 == "cleanallstatinfo");
}
} // namespace HiviewDFX
} // namespace OHOS
