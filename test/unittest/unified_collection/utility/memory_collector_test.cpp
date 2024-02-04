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
#include <dlfcn.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

#include "memory_collector.h"

#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::UCollectUtil;
using namespace OHOS::HiviewDFX::UCollect;

class MemoryCollectorTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
};

namespace {
bool HasValidAILibrary()
{
    const std::string libName = "libai_infra.so";
    void* handle = dlopen(libName.c_str(), RTLD_LAZY);
    return handle != nullptr;
}

bool CheckFormat(const std::string &fileName, std::regex &reg1, std::regex &reg2, std::regex &reg3, int cnt)
{
    std::ifstream file;
    file.open(fileName.c_str());
    if (!file.is_open()) {
        return false;
    }
    std::string line;
    while (cnt--) {
        getline(file, line);
    }
    while (getline(file, line)) {
        if (!((regex_match(line, reg1) && regex_match(line, reg2)) || regex_match(line, reg3))) {
            file.close();
            return false;
        }
    }
    file.close();
    return true;
}
}

/**
 * @tc.name: MemoryCollectorTest001
 * @tc.desc: used to test MemoryCollector.CollectProcessMemory
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest001, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<ProcessMemory> data = collector->CollectProcessMemory(1000);
    std::cout << "collect process memory result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: MemoryCollectorTest002
 * @tc.desc: used to test MemoryCollector.CollectSysMemory
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest002, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<SysMemory> data = collector->CollectSysMemory();
    std::cout << "collect system memory result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: MemoryCollectorTest003
 * @tc.desc: used to test MemoryCollector.CollectRawMemInfo
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest003, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::string> data = collector->CollectRawMemInfo();
    std::cout << "collect raw memory info result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);

    std::regex reg1("^[\\w()]{1,}:\\s{1,}\\d{1,}$");
    std::regex reg2("^[\\w\\s:()]{24,}$");
    std::regex reg3("^[\\w()]{1,}:\\s{1,}\\d{1,} kB$");
    bool flag = CheckFormat(data.data, reg1, reg2, reg3, 0);    //0: don't skip the first line
    ASSERT_TRUE(flag);
}

/**
 * @tc.name: MemoryCollectorTest004
 * @tc.desc: used to test MemoryCollector.CollectAllProcessMemory
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest004, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::vector<ProcessMemory>> data = collector->CollectAllProcessMemory();
    std::cout << "collect all process memory result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: MemoryCollectorTest005
 * @tc.desc: used to test MemoryCollector.ExportAllProcessMemory
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest005, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::string> data = collector->ExportAllProcessMemory();
    std::cout << "export all process memory result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);

    std::regex reg1("^\\d{1,}\\s{1,}[\\w\\.:-]{1,}(\\s{1,}\\d{1,}){3}\\s{1,}[\\d-]{1,}$");
    std::regex reg2("^[\\w\\s\\.:-]{1,}$");
    std::regex reg3("^used to take a place$");
    bool flag = CheckFormat(data.data, reg1, reg2, reg3, 1);    //1: skip the first line
    ASSERT_TRUE(flag);
}

/**
 * @tc.name: MemoryCollectorTest006
 * @tc.desc: used to test MemoryCollector.CollectRawSlabInfo
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest006, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::string> data = collector->CollectRawSlabInfo();
    std::cout << "collect raw slab info result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);

    std::string first = "^[\\w\\[\\]-]{1,}(\\s{1,}\\d{1,}){5}( : tunables)(\\s{1,}\\d{1,}){3}";
    std::string second = "( : slabdata)(\\s{1,}\\d{1,}){3}$";
    std::string regStr = first + second;
    std::regex reg1(regStr);
    std::regex reg2("^[\\w\\s:\\[\\]-]{1,}$");
    std::regex reg3("^(\\w{1,} - version: )[\\d\\.]{1,}|(#name)[\\s\\w<>:]{1,}$");
    bool flag = CheckFormat(data.data, reg1, reg2, reg3, 0);    //0: don't skip the first line
    ASSERT_TRUE(flag);
}

/**
 * @tc.name: MemoryCollectorTest007
 * @tc.desc: used to test MemoryCollector.CollectRawPageTypeInfo
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest007, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::string> data = collector->CollectRawPageTypeInfo();
    std::cout << "collect raw pagetype info result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);

    std::regex reg1("^(Node)\\s{1,}\\d{1,}(, zone)\\s{1,}\\w{1,}(, type)\\s{1,}\\w{1,}(\\s{1,}\\d{1,}){21} $");
    std::regex reg2("^(Node)[\\s\\d]{5,}(, zone)[\\s\\w]{9,}(, type)[\\s\\w]{13}([\\s\\d]{7}){21} $");
    std::regex reg3("^used to take a place$");
    bool flag = CheckFormat(data.data, reg1, reg2, reg3, 4);    //4: skip the first four lines
    ASSERT_TRUE(flag);
}

/**
 * @tc.name: MemoryCollectorTest008
 * @tc.desc: used to test MemoryCollector.CollectRawDMA
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest008, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::string> data = collector->CollectRawDMA();
    std::cout << "collect raw DMA result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);

    std::regex reg1("^[\\w:\\.]{1,}(\\s{1,}\\d{1,}){5}(\\s{1,}\\w{1,}){3}$");
    std::regex reg2("^[\\w:\\.\\s]{17}([\\d\\s]{17}){5}[\\s\\w]{1,}$");
    std::regex reg3("^(Total dmabuf size of )[\\w:]{1,}(: )\\d{1,}( bytes)$");
    bool flag = CheckFormat(data.data, reg1, reg2, reg3, 2);    //2: skip the first two lines
    ASSERT_TRUE(flag);
}

/**
 * @tc.name: MemoryCollectorTest009
 * @tc.desc: used to test MemoryCollector.CollectAllAIProcess
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest009, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::vector<AIProcessMem>> data = collector->CollectAllAIProcess();
    std::cout << "collect all AI process result" << data.retCode << std::endl;
    if (HasValidAILibrary()) {
        ASSERT_TRUE(data.retCode == UcError::SUCCESS);
    } else {
        ASSERT_TRUE(data.retCode == UcError::READ_FAILED);
    }
}

/**
 * @tc.name: MemoryCollectorTest010
 * @tc.desc: used to test MemoryCollector.ExportAllAIProcess
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest010, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::string> data = collector->ExportAllAIProcess();
    std::cout << "export all AI process result" << data.retCode << std::endl;
    if (HasValidAILibrary()) {
        ASSERT_TRUE(data.retCode == UcError::SUCCESS);
    } else {
        ASSERT_TRUE(data.retCode == UcError::READ_FAILED);
    }
}

/**
 * @tc.name: MemoryCollectorTest011
 * @tc.desc: used to test MemoryCollector.CollectRawSmaps
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest011, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::string> data = collector->CollectRawSmaps(1);
    std::cout << "collect raw smaps info result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: MemoryCollectorTest012
 * @tc.desc: used to test MemoryCollector.CollectHprof
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest012, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<std::string> data = collector->CollectHprof(1);
    std::cout << "collect heap snapshot result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}

/**
 * @tc.name: MemoryCollectorTest013
 * @tc.desc: used to test MemoryCollector.CollectProcessVss
 * @tc.type: FUNC
*/
HWTEST_F(MemoryCollectorTest, MemoryCollectorTest013, TestSize.Level1)
{
    std::shared_ptr<MemoryCollector> collector = MemoryCollector::Create();
    CollectResult<uint64_t> data = collector->CollectProcessVss(1000);
    std::cout << "collect processvss result" << data.retCode << std::endl;
    ASSERT_TRUE(data.retCode == UcError::SUCCESS);
}
