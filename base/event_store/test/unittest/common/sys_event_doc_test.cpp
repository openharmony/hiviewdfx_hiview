/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "sys_event_doc_test.h"

#include "base_def.h"
#include "hiview_global.h"
#include "sys_event_doc.h"

namespace OHOS {
namespace HiviewDFX {
using namespace  OHOS::HiviewDFX::EventStore;
namespace {
const std::string TEST_DB_VERSION1_FILE = "/data/test/TEST_DOMAIN/TEST_VERSION1-1-CRITICAL-1.db";
constexpr size_t MAX_COUNT = 100;
constexpr int BUFFER_SIZE = 1024;
constexpr int ERR_RAW_LEN = 21;
constexpr char TEST_DB_PATH[] = "/data/text/";
class DocTestContext : public HiviewContext {
public:
    std::string GetHiViewDirectory(HiviewContext::DirectoryType type)
    {
        return TEST_DB_PATH;
    }
};
}

void SysEventDocTest::SetUpTestCase()
{
}

void SysEventDocTest::TearDownTestCase()
{
}

void SysEventDocTest::SetUp()
{
}

void SysEventDocTest::TearDown()
{
}

/**
 * @tc.name: SysEventDocTest001
 * @tc.desc: Insert test
 * @tc.type: FUNC
 */
HWTEST_F(SysEventDocTest, SysEventDocTest001, testing::ext::TestSize.Level3)
{
    DocTestContext context;
    HiviewGlobal::CreateInstance(context);
    SysEventDoc sysEventDoc(TEST_DB_VERSION1_FILE);
    ASSERT_EQ(sysEventDoc.Insert(nullptr), DOC_STORE_ERROR_NULL);

    SysEventCreator sysEventCreator("WINDOWMANAGER", "NO_FOCUS_WINDOW", SysEventCreator::STATISTIC);
    std::vector<int> values = {1, 2, 3};
    sysEventCreator.SetKeyValue("test1", values);
    time_t now = time(nullptr);
    sysEventCreator.SetKeyValue("test2", now);
    std::shared_ptr<SysEvent> sysEvent = std::make_shared<SysEvent>("test", nullptr, sysEventCreator);
    sysEvent->SetLevel("CRITICAL");
    int64_t testSeq = 0;
    sysEvent->SetEventSeq(testSeq);
    sysEventCreator.SetKeyValue("test_str", "abcdeghijklmnopqrstuvwxyz1234567890");
    sysEventCreator.SetKeyValue("test_str1", "ABCDFGHIJKLMNOPQRSTUVWXYZ");
    ASSERT_EQ(sysEventDoc.Insert(sysEvent), DOC_STORE_SUCCESS);
    for (int i = 0; i < MAX_COUNT; ++i) {
        testSeq++;
        sysEvent->SetEventSeq(testSeq);
        ASSERT_EQ(sysEventDoc.Insert(sysEvent), DOC_STORE_SUCCESS);
    }
}

/**
 * @tc.name: SysEventDocTest002
 * @tc.desc: Insert test
 * @tc.type: FUNC
 */
HWTEST_F(SysEventDocTest, SysEventDocTest002, testing::ext::TestSize.Level3)
{
    DocTestContext context;
    HiviewGlobal::CreateInstance(context);
    char* buffer = new char[BUFFER_SIZE + 1]();
    *(reinterpret_cast<uint32_t*>(buffer)) = ERR_RAW_LEN;
    *(buffer + sizeof(uint32_t)) = 0;
    auto rawData = std::make_shared<EventRaw::RawData>(reinterpret_cast<uint8_t*>(buffer), BUFFER_SIZE + 1);
    auto event = std::make_shared<SysEvent>("SysEventSource", nullptr, rawData);
    SysEventDoc sysEventDoc(TEST_DB_VERSION1_FILE);
    ASSERT_EQ(sysEventDoc.Insert(event), DOC_STORE_ERROR_INVALID);

    *(reinterpret_cast<uint32_t*>(buffer)) = MAX_NEW_SIZE + 1;
    auto rawDataMax = std::make_shared<EventRaw::RawData>(reinterpret_cast<uint8_t*>(buffer), BUFFER_SIZE + 1);
    auto eventMax = std::make_shared<SysEvent>("SysEventSource", nullptr, rawDataMax);
    ASSERT_EQ(sysEventDoc.Insert(eventMax), DOC_STORE_ERROR_INVALID);
    delete[] buffer;
}

/**
 * @tc.name: SysEventDocTest003
 * @tc.desc: Query test
 * @tc.type: FUNC
 */
HWTEST_F(SysEventDocTest, SysEventDocTest003, testing::ext::TestSize.Level3)
{
    DocTestContext context;
    HiviewGlobal::CreateInstance(context);
    SysEventDoc sysEventDoc("");
    DocQuery query;
    EntryQueue entries;
    int num;
    ASSERT_EQ(sysEventDoc.Query(query, entries, num), DOC_STORE_ERROR_IO);
}
} // namespace HiviewDFX
} // namespace OHOS